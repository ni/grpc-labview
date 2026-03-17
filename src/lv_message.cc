//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_server.h>
#include <lv_message.h>
#include <climits>
#include <sstream>
#include <feature_toggles.h>
#include <string_utils.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/unknown_field_set.h>
#include <google/protobuf/wire_format_lite.h>

namespace
{
    using WFL = google::protobuf::internal::WireFormatLite;

    // Read a field and optionally store it in an UnknownFieldSet.
    // Pass nullptr for unknownFields to just skip (used for nested groups).
    bool HandleUnknownField(google::protobuf::io::CodedInputStream* input, uint32_t tag,
                     google::protobuf::UnknownFieldSet* unknownFields)
    {
        uint32_t fieldNumber = WFL::GetTagFieldNumber(tag);
        WFL::WireType wireType = WFL::GetTagWireType(tag);

        switch (wireType)
        {
            case WFL::WIRETYPE_VARINT:
            {
                uint64_t value;
                if (!input->ReadVarint64(&value)) return false;
                if (unknownFields) unknownFields->AddVarint(fieldNumber, value);
                return true;
            }
            case WFL::WIRETYPE_FIXED64:
            {
                uint64_t value;
                if (!input->ReadLittleEndian64(&value)) return false;
                if (unknownFields) unknownFields->AddFixed64(fieldNumber, value);
                return true;
            }
            case WFL::WIRETYPE_LENGTH_DELIMITED:
            {
                uint32_t length;
                if (!input->ReadVarint32(&length)) return false;
                if (length > static_cast<uint32_t>(INT_MAX)) return false;
                std::string value;
                if (!input->ReadString(&value, static_cast<int>(length))) return false;
                if (unknownFields) unknownFields->AddLengthDelimited(fieldNumber, value);
                return true;
            }
            case WFL::WIRETYPE_START_GROUP:
            {
                // Groups are deprecated but must be skipped correctly.
                // Inner fields are forwarded to unknownFields so nested group
                // content is preserved when the caller wants unknown-field storage.
                uint32_t end_tag = WFL::MakeTag(fieldNumber, WFL::WIRETYPE_END_GROUP);
                while (true)
                {
                    uint32_t inner_tag = input->ReadTag();
                    if (inner_tag == 0) return false;
                    if (inner_tag == end_tag) return true;
                    if (!HandleUnknownField(input, inner_tag, unknownFields)) return false;
                }
                return false; // unreachable; guards against future refactoring
            }
            case WFL::WIRETYPE_END_GROUP:
                return false;
            case WFL::WIRETYPE_FIXED32:
            {
                uint32_t value;
                if (!input->ReadLittleEndian32(&value)) return false;
                if (unknownFields) unknownFields->AddFixed32(fieldNumber, value);
                return true;
            }
            default:
                return false;
        }
    }

    // ZeroCopyInputStream over a sequence of grpc::Slices.
    // Allows parsing directly from a multi-slice ByteBuffer with no data copy:
    // grpc::ByteBuffer::Dump() only increments slice refcounts, so the parser
    // reads straight from the original gRPC transport buffers.
    class MultiSliceInputStream final : public google::protobuf::io::ZeroCopyInputStream
    {
    public:
        explicit MultiSliceInputStream(std::vector<grpc::Slice> slices)
            : _slices(std::move(slices)), _sliceIndex(0), _offsetInSlice(0), _byteCount(0)
        {
        }

        bool Next(const void** data, int* size) override
        {
            while (_sliceIndex < _slices.size())
            {
                const grpc::Slice& s = _slices[_sliceIndex];
                size_t remaining = s.size() - _offsetInSlice;
                if (remaining > 0)
                {
                    *data = s.begin() + _offsetInSlice;
                    *size = static_cast<int>(remaining);
                    _byteCount += static_cast<int64_t>(remaining);
                    _sliceIndex++;
                    _offsetInSlice = 0;
                    return true;
                }
                // Empty slice - skip it.
                _sliceIndex++;
                _offsetInSlice = 0;
            }
            return false;
        }

        // count <= size returned by the preceding Next() call (ZeroCopyInputStream contract).
        void BackUp(int count) override
        {
            _byteCount -= count;
            _sliceIndex--;
            _offsetInSlice = _slices[_sliceIndex].size() - static_cast<size_t>(count);
        }

        bool Skip(int count) override
        {
            while (count > 0 && _sliceIndex < _slices.size())
            {
                const grpc::Slice& s = _slices[_sliceIndex];
                size_t remaining = s.size() - _offsetInSlice;
                if (static_cast<size_t>(count) < remaining)
                {
                    _offsetInSlice += static_cast<size_t>(count);
                    _byteCount += count;
                    count = 0;
                }
                else
                {
                    _byteCount += static_cast<int64_t>(remaining);
                    count -= static_cast<int>(remaining);
                    _sliceIndex++;
                    _offsetInSlice = 0;
                }
            }
            return count == 0;
        }

        int64_t ByteCount() const override
        {
            return _byteCount;
        }

    private:
        std::vector<grpc::Slice> _slices;
        size_t _sliceIndex;
        size_t _offsetInSlice;
        int64_t _byteCount;
    };

}

namespace grpc_labview
{
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    LVMessage::LVMessage(std::shared_ptr<MessageMetadata> metadata) : _metadata(metadata)
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    LVMessage::~LVMessage()
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessage::ParseFromByteBuffer(const grpc::ByteBuffer &buffer)
    {
        Clear();

        using namespace google::protobuf::io;

        // Fast path: if the ByteBuffer is already backed by a single contiguous
        // slice we can parse directly from its memory with no copy or allocation.
        grpc::Slice singleSlice;
        if (buffer.TrySingleSlice(&singleSlice).ok())
        {
            ArrayInputStream ais(singleSlice.begin(), static_cast<int>(singleSlice.size()));
            CodedInputStream cis(&ais);
            cis.SetRecursionLimit(100);
            return ParseFromCodedStream(&cis) && cis.ConsumedEntireMessage();
        }

        // Slow path: multiple slices - iterate over them without coalescing.
        // Dump() only increments slice refcounts; no byte data is copied.
        std::vector<grpc::Slice> slices;
        if (!buffer.Dump(&slices).ok())
        {
            return false;
        }
        MultiSliceInputStream msis(std::move(slices));
        CodedInputStream cis(&msis);
        cis.SetRecursionLimit(100);
        return ParseFromCodedStream(&cis) && cis.ConsumedEntireMessage();
    }
    
    //---------------------------------------------------------------------
    // Parse from string using only public protobuf APIs
    //---------------------------------------------------------------------
    bool LVMessage::ParseFromString(const std::string& data)
    {
        using namespace google::protobuf::io;
        
        ArrayInputStream ais(data.data(), static_cast<int>(data.size()));
        // Use public CodedInputStream API (handles empty and non-empty data uniformly)
        CodedInputStream cis(&ais);
        cis.SetRecursionLimit(100);
        
        if (!ParseFromCodedStream(&cis)) {
            return false;
        }
        
        return cis.ConsumedEntireMessage();
    }
    
    //---------------------------------------------------------------------
    // Parse from CodedInputStream - uses only public APIs
    //---------------------------------------------------------------------
    bool LVMessage::ParseFromCodedStream(google::protobuf::io::CodedInputStream* input)
    {
        uint32_t tag;
        
        while ((tag = input->ReadTag()) != 0)
        {
            uint32_t fieldNumber = WFL::GetTagFieldNumber(tag);
            
            if (_metadata == nullptr)
            {
                // No schema - store everything as unknown for UnknownFields use
                if (!HandleUnknownField(input, tag, &_unknownFields)) {
                    return false;
                }
            }
            else
            {
                auto fieldIt = _metadata->_mappedElements.find(fieldNumber);
                if (fieldIt != _metadata->_mappedElements.end())
                {
                    auto& fieldInfo = (*fieldIt).second;

                    if (fieldInfo->isInOneof)
                    {
                        // Protobuf "last value wins" semantics for oneof: if a different member of
                        // this oneof was already parsed, evict its stale entry from _values so it
                        // is not re-serialized, then update the selected-index to this field.
                        auto existing = _oneofContainerToSelectedIndexMap.find(fieldInfo->oneofContainerName);
                        if (existing != _oneofContainerToSelectedIndexMap.end())
                        {
                            _values.erase(existing->second);
                            existing->second = fieldInfo->protobufIndex;
                        }
                        else
                        {
                            _oneofContainerToSelectedIndexMap.emplace(fieldInfo->oneofContainerName, fieldInfo->protobufIndex);
                        }
                    }

                    // Parse field based on type using CodedInputStream
                    if (!ParseFieldFromCodedStream(input, tag, fieldNumber, *fieldInfo)) {
                        return false;
                    }
                }
                else
                {
                    // Unknown field - store for potential later inspection
                    if (!HandleUnknownField(input, tag, &_unknownFields)) {
                        return false;
                    }
                }
            }
        }
        
        PostInteralParseAction();
        return true;
    }
    
    //---------------------------------------------------------------------
    // Parse a single field from CodedInputStream
    //---------------------------------------------------------------------
    bool LVMessage::ParseFieldFromCodedStream(
        google::protobuf::io::CodedInputStream* input,
        uint32_t tag,
        uint32_t fieldNumber,
        const MessageElementMetadata& fieldInfo)
    {
        uint32_t wireType = static_cast<uint32_t>(WFL::GetTagWireType(tag));
        switch (fieldInfo.type)
        {
        case LVMessageMetadataType::Int32Value:
            return ParseInt32Field(input, fieldNumber, fieldInfo, wireType);
        case LVMessageMetadataType::Int64Value:
            return ParseInt64Field(input, fieldNumber, fieldInfo, wireType);
        case LVMessageMetadataType::UInt32Value:
            return ParseUInt32Field(input, fieldNumber, fieldInfo, wireType);
        case LVMessageMetadataType::UInt64Value:
            return ParseUInt64Field(input, fieldNumber, fieldInfo, wireType);
        case LVMessageMetadataType::SInt32Value:
            return ParseSInt32Field(input, fieldNumber, fieldInfo, wireType);
        case LVMessageMetadataType::SInt64Value:
            return ParseSInt64Field(input, fieldNumber, fieldInfo, wireType);
        case LVMessageMetadataType::Fixed32Value:
            return ParseFixed32Field(input, fieldNumber, fieldInfo, wireType);
        case LVMessageMetadataType::Fixed64Value:
            return ParseFixed64Field(input, fieldNumber, fieldInfo, wireType);
        case LVMessageMetadataType::SFixed32Value:
            return ParseSFixed32Field(input, fieldNumber, fieldInfo, wireType);
        case LVMessageMetadataType::SFixed64Value:
            return ParseSFixed64Field(input, fieldNumber, fieldInfo, wireType);
        case LVMessageMetadataType::FloatValue:
            return ParseFloatField(input, fieldNumber, fieldInfo, wireType);
        case LVMessageMetadataType::DoubleValue:
            return ParseDoubleField(input, fieldNumber, fieldInfo, wireType);
        case LVMessageMetadataType::BoolValue:
            return ParseBoolField(input, fieldNumber, fieldInfo, wireType);
        case LVMessageMetadataType::EnumValue:
            return ParseEnumField(input, fieldNumber, fieldInfo, wireType);
        case LVMessageMetadataType::StringValue:
            return ParseStringField(input, fieldNumber, fieldInfo);
        case LVMessageMetadataType::BytesValue:
            return ParseBytesField(input, fieldNumber, fieldInfo);
        case LVMessageMetadataType::MessageValue:
            return ParseMessageField(input, fieldNumber, fieldInfo);
        default:
            return false;
        }
    }
    
    //---------------------------------------------------------------------
    // Traits-based generic numeric field parser.
    //
    // Each traits struct defines:
    //   RawType      – type read from the wire by Read()
    //   ScalarType   – LVMessageValue subclass for a scalar field
    //   RepeatedType – LVMessageValue subclass for a repeated field
    //   Read()       – reads one RawType from a CodedInputStream
    //   Transform()  – converts RawType to the value stored in the LV container
    //
    // ParseNumericField<Traits> captures the shared packed/unpacked/scalar
    // branching logic. Each template instantiation compiles to the same
    // machine code as the equivalent hand-written method would produce.
    //---------------------------------------------------------------------
    namespace
    {
        struct Int32Traits {
            using RawType = uint32_t;
            using ScalarType = LVVariableMessageValue<int>;
            using RepeatedType = LVRepeatedMessageValue<int>;
            static bool Read(google::protobuf::io::CodedInputStream* s, RawType& out) { return s->ReadVarint32(&out); }
            static int Transform(RawType v) { return static_cast<int>(v); }
        };
        struct Int64Traits {
            using RawType = uint64_t;
            using ScalarType = LVVariableMessageValue<int64_t>;
            using RepeatedType = LVRepeatedMessageValue<int64_t>;
            static bool Read(google::protobuf::io::CodedInputStream* s, RawType& out) { return s->ReadVarint64(&out); }
            static int64_t Transform(RawType v) { return static_cast<int64_t>(v); }
        };
        struct UInt32Traits {
            using RawType = uint32_t;
            using ScalarType = LVVariableMessageValue<uint32_t>;
            using RepeatedType = LVRepeatedMessageValue<uint32_t>;
            static bool Read(google::protobuf::io::CodedInputStream* s, RawType& out) { return s->ReadVarint32(&out); }
            static uint32_t Transform(RawType v) { return v; }
        };
        struct UInt64Traits {
            using RawType = uint64_t;
            using ScalarType = LVVariableMessageValue<uint64_t>;
            using RepeatedType = LVRepeatedMessageValue<uint64_t>;
            static bool Read(google::protobuf::io::CodedInputStream* s, RawType& out) { return s->ReadVarint64(&out); }
            static uint64_t Transform(RawType v) { return v; }
        };
        struct FloatTraits {
            using RawType = uint32_t;
            using ScalarType = LVVariableMessageValue<float>;
            using RepeatedType = LVRepeatedMessageValue<float>;
            static bool Read(google::protobuf::io::CodedInputStream* s, RawType& out) { return s->ReadLittleEndian32(&out); }
            static float Transform(RawType v) { float f; memcpy(&f, &v, sizeof(f)); return f; }
        };
        struct DoubleTraits {
            using RawType = uint64_t;
            using ScalarType = LVVariableMessageValue<double>;
            using RepeatedType = LVRepeatedMessageValue<double>;
            static bool Read(google::protobuf::io::CodedInputStream* s, RawType& out) { return s->ReadLittleEndian64(&out); }
            static double Transform(RawType v) { double d; memcpy(&d, &v, sizeof(d)); return d; }
        };
        struct BoolTraits {
            using RawType = uint32_t;
            using ScalarType = LVVariableMessageValue<bool>;
            using RepeatedType = LVRepeatedMessageValue<bool>;
            static bool Read(google::protobuf::io::CodedInputStream* s, RawType& out) { return s->ReadVarint32(&out); }
            static bool Transform(RawType v) { return v != 0; }
        };
        struct EnumTraits {
            using RawType = uint32_t;
            using ScalarType = LVEnumMessageValue;
            using RepeatedType = LVRepeatedEnumMessageValue;
            static bool Read(google::protobuf::io::CodedInputStream* s, RawType& out) { return s->ReadVarint32(&out); }
            static int Transform(RawType v) { return static_cast<int>(v); }
        };
        struct SInt32Traits {
            using RawType = uint32_t;
            using ScalarType = LVSInt32MessageValue;
            using RepeatedType = LVRepeatedSInt32MessageValue;
            static bool Read(google::protobuf::io::CodedInputStream* s, RawType& out) { return s->ReadVarint32(&out); }
            static int32_t Transform(RawType v) { return WFL::ZigZagDecode32(v); }
        };
        struct SInt64Traits {
            using RawType = uint64_t;
            using ScalarType = LVSInt64MessageValue;
            using RepeatedType = LVRepeatedSInt64MessageValue;
            static bool Read(google::protobuf::io::CodedInputStream* s, RawType& out) { return s->ReadVarint64(&out); }
            static int64_t Transform(RawType v) { return WFL::ZigZagDecode64(v); }
        };
        struct Fixed32Traits {
            using RawType = uint32_t;
            using ScalarType = LVFixed32MessageValue;
            using RepeatedType = LVRepeatedFixed32MessageValue;
            static bool Read(google::protobuf::io::CodedInputStream* s, RawType& out) { return s->ReadLittleEndian32(&out); }
            static uint32_t Transform(RawType v) { return v; }
        };
        struct Fixed64Traits {
            using RawType = uint64_t;
            using ScalarType = LVFixed64MessageValue;
            using RepeatedType = LVRepeatedFixed64MessageValue;
            static bool Read(google::protobuf::io::CodedInputStream* s, RawType& out) { return s->ReadLittleEndian64(&out); }
            static uint64_t Transform(RawType v) { return v; }
        };
        struct SFixed32Traits {
            using RawType = uint32_t;
            using ScalarType = LVSFixed32MessageValue;
            using RepeatedType = LVRepeatedSFixed32MessageValue;
            static bool Read(google::protobuf::io::CodedInputStream* s, RawType& out) { return s->ReadLittleEndian32(&out); }
            static int32_t Transform(RawType v) { return static_cast<int32_t>(v); }
        };
        struct SFixed64Traits {
            using RawType = uint64_t;
            using ScalarType = LVSFixed64MessageValue;
            using RepeatedType = LVRepeatedSFixed64MessageValue;
            static bool Read(google::protobuf::io::CodedInputStream* s, RawType& out) { return s->ReadLittleEndian64(&out); }
            static int64_t Transform(RawType v) { return static_cast<int64_t>(v); }
        };

        template<typename Traits>
        bool ParseNumericField(
            google::protobuf::io::CodedInputStream* input,
            uint32_t fieldNumber,
            const MessageElementMetadata& fieldInfo,
            uint32_t wireType,
            std::map<int, std::shared_ptr<LVMessageValue>>& values)
        {
            if (fieldInfo.isRepeated)
            {
                if (wireType == WFL::WIRETYPE_LENGTH_DELIMITED) // packed
                {
                    uint32_t length;
                    if (!input->ReadVarint32(&length)) return false;
                    auto limit = input->PushLimit(length);
                    auto it = values.find(fieldNumber);
                    std::shared_ptr<typename Traits::RepeatedType> v;
                    if (it == values.end())
                    {
                        v = std::make_shared<typename Traits::RepeatedType>(fieldNumber);
                        values.emplace(fieldNumber, v);
                    }
                    else
                    {
                        v = std::static_pointer_cast<typename Traits::RepeatedType>(it->second);
                    }
                    while (input->BytesUntilLimit() > 0)
                    {
                        typename Traits::RawType raw;
                        if (!Traits::Read(input, raw)) return false;
                        v->_value.Add(Traits::Transform(raw));
                    }
                    input->PopLimit(limit);
                }
                else // unpacked: single element per tag occurrence
                {
                    typename Traits::RawType raw;
                    if (!Traits::Read(input, raw)) return false;
                    auto it = values.find(fieldNumber);
                    if (it == values.end())
                    {
                        auto v = std::make_shared<typename Traits::RepeatedType>(fieldNumber);
                        v->_value.Add(Traits::Transform(raw));
                        values.emplace(fieldNumber, v);
                    }
                    else
                    {
                        auto v = std::static_pointer_cast<typename Traits::RepeatedType>(it->second);
                        v->_value.Add(Traits::Transform(raw));
                    }
                }
            }
            else
            {
                typename Traits::RawType raw;
                if (!Traits::Read(input, raw)) return false;
                values.emplace(fieldNumber, std::make_shared<typename Traits::ScalarType>(fieldNumber, Traits::Transform(raw)));
            }
            return true;
        }
    } // anonymous namespace

    //---------------------------------------------------------------------
    // Helper methods to parse specific field types from CodedInputStream
    //---------------------------------------------------------------------
    bool LVMessage::ParseInt32Field(google::protobuf::io::CodedInputStream* input,
                                     uint32_t fieldNumber,
                                     const MessageElementMetadata& fieldInfo,
                                     uint32_t wireType)
    {
        return ParseNumericField<Int32Traits>(input, fieldNumber, fieldInfo, wireType, _values);
    }
    
    bool LVMessage::ParseInt64Field(google::protobuf::io::CodedInputStream* input,
                                     uint32_t fieldNumber,
                                     const MessageElementMetadata& fieldInfo,
                                     uint32_t wireType)
    {
        return ParseNumericField<Int64Traits>(input, fieldNumber, fieldInfo, wireType, _values);
    }
    
    bool LVMessage::ParseUInt32Field(google::protobuf::io::CodedInputStream* input,
                                      uint32_t fieldNumber,
                                      const MessageElementMetadata& fieldInfo,
                                      uint32_t wireType)
    {
        return ParseNumericField<UInt32Traits>(input, fieldNumber, fieldInfo, wireType, _values);
    }
    
    bool LVMessage::ParseUInt64Field(google::protobuf::io::CodedInputStream* input,
                                      uint32_t fieldNumber,
                                      const MessageElementMetadata& fieldInfo,
                                      uint32_t wireType)
    {
        return ParseNumericField<UInt64Traits>(input, fieldNumber, fieldInfo, wireType, _values);
    }
    
    bool LVMessage::ParseFloatField(google::protobuf::io::CodedInputStream* input,
                                     uint32_t fieldNumber,
                                     const MessageElementMetadata& fieldInfo,
                                     uint32_t wireType)
    {
        return ParseNumericField<FloatTraits>(input, fieldNumber, fieldInfo, wireType, _values);
    }
    
    bool LVMessage::ParseDoubleField(google::protobuf::io::CodedInputStream* input,
                                      uint32_t fieldNumber,
                                      const MessageElementMetadata& fieldInfo,
                                      uint32_t wireType)
    {
        return ParseNumericField<DoubleTraits>(input, fieldNumber, fieldInfo, wireType, _values);
    }
    
    bool LVMessage::ParseBoolField(google::protobuf::io::CodedInputStream* input,
                                    uint32_t fieldNumber,
                                    const MessageElementMetadata& fieldInfo,
                                    uint32_t wireType)
    {
        return ParseNumericField<BoolTraits>(input, fieldNumber, fieldInfo, wireType, _values);
    }
    
    bool LVMessage::ParseStringField(google::protobuf::io::CodedInputStream* input,
                                      uint32_t fieldNumber,
                                      const MessageElementMetadata& fieldInfo)
    {
        uint32_t length;
        if (!input->ReadVarint32(&length)) return false;
        if (length > static_cast<uint32_t>(INT_MAX)) return false;
        std::string value;
        if (!input->ReadString(&value, static_cast<int>(length))) return false;

        if (!VerifyUtf8String(value, fieldInfo.fieldName.c_str())) {
            throw std::runtime_error("String contains invalid UTF-8 data.");
        }

        if (fieldInfo.isRepeated)
        {
            // For repeated strings, use RepeatedPtrField
            auto it = _values.find(fieldNumber);
            if (it == _values.end())
            {
                auto v = std::make_shared<LVRepeatedStringMessageValue>(fieldNumber);
                *v->_value.Add() = value;
                _values.emplace(fieldNumber, v);
            }
            else
            {
                auto v = std::static_pointer_cast<LVRepeatedStringMessageValue>(it->second);
                *v->_value.Add() = value;
            }
        }
        else
        {
            auto v = std::make_shared<LVStringMessageValue>(fieldNumber, value);
            _values.emplace(fieldNumber, v);
        }
        return true;
    }
    
    bool LVMessage::ParseBytesField(google::protobuf::io::CodedInputStream* input,
                                     uint32_t fieldNumber,
                                     const MessageElementMetadata& fieldInfo)
    {
        uint32_t length;
        if (!input->ReadVarint32(&length)) return false;
        if (length > static_cast<uint32_t>(INT_MAX)) return false;
        std::string value;
        if (!input->ReadString(&value, static_cast<int>(length))) return false;

        if (!FeatureConfig::getInstance().AreUtf8StringsEnabled())
        {
            // Legacy mode: bytes fields are stored as strings so CopyBytesToCluster
            // can delegate to CopyStringToCluster (matching LVMessageEfficient behaviour).
            if (fieldInfo.isRepeated)
            {
                auto it = _values.find(fieldNumber);
                if (it == _values.end())
                {
                    auto v = std::make_shared<LVRepeatedStringMessageValue>(fieldNumber);
                    *v->_value.Add() = value;
                    _values.emplace(fieldNumber, v);
                }
                else
                {
                    auto v = std::static_pointer_cast<LVRepeatedStringMessageValue>(it->second);
                    *v->_value.Add() = value;
                }
            }
            else
            {
                auto v = std::make_shared<LVStringMessageValue>(fieldNumber, value);
                _values.emplace(fieldNumber, v);
            }
        }
        else
        {
            if (fieldInfo.isRepeated)
            {
                auto it = _values.find(fieldNumber);
                if (it == _values.end())
                {
                    auto v = std::make_shared<LVRepeatedBytesMessageValue>(fieldNumber);
                    *v->_value.Add() = value;
                    _values.emplace(fieldNumber, v);
                }
                else
                {
                    auto v = std::static_pointer_cast<LVRepeatedBytesMessageValue>(it->second);
                    *v->_value.Add() = value;
                }
            }
            else
            {
                auto v = std::make_shared<LVBytesMessageValue>(fieldNumber, value);
                _values.emplace(fieldNumber, v);
            }
        }
        return true;
    }
    
    bool LVMessage::ParseMessageField(google::protobuf::io::CodedInputStream* input,
                                       uint32_t fieldNumber,
                                       const MessageElementMetadata& fieldInfo)
    {
        uint32_t length;
        if (!input->ReadVarint32(&length)) return false;
        
        auto limit = input->PushLimit(length);
        
        // Get metadata for nested message
        auto nestedMetadata = fieldInfo._owner->FindMetadata(fieldInfo.embeddedMessageName);
        auto nestedMessage = std::make_shared<LVMessage>(nestedMetadata);
        
        if (!nestedMessage->ParseFromCodedStream(input)) {
            input->PopLimit(limit);
            return false;
        }
        
        input->PopLimit(limit);
        
        if (fieldInfo.isRepeated)
        {
            auto it = _values.find(fieldNumber);
            if (it == _values.end())
            {
                auto v = std::make_shared<LVRepeatedNestedMessageMessageValue>(fieldNumber);
                v->_value.push_back(nestedMessage);
                _values.emplace(fieldNumber, v);
            }
            else
            {
                auto v = std::static_pointer_cast<LVRepeatedNestedMessageMessageValue>(it->second);
                v->_value.push_back(nestedMessage);
            }
        }
        else
        {
            auto v = std::make_shared<LVNestedMessageMessageValue>(fieldNumber, nestedMessage);
            _values.emplace(fieldNumber, v);
        }
        return true;
    }
    
    bool LVMessage::ParseEnumField(google::protobuf::io::CodedInputStream* input,
                                    uint32_t fieldNumber,
                                    const MessageElementMetadata& fieldInfo,
                                    uint32_t wireType)
    {
        return ParseNumericField<EnumTraits>(input, fieldNumber, fieldInfo, wireType, _values);
    }
    
    bool LVMessage::ParseSInt32Field(google::protobuf::io::CodedInputStream* input,
                                      uint32_t fieldNumber,
                                      const MessageElementMetadata& fieldInfo,
                                      uint32_t wireType)
    {
        return ParseNumericField<SInt32Traits>(input, fieldNumber, fieldInfo, wireType, _values);
    }
    
    bool LVMessage::ParseSInt64Field(google::protobuf::io::CodedInputStream* input,
                                      uint32_t fieldNumber,
                                      const MessageElementMetadata& fieldInfo,
                                      uint32_t wireType)
    {
        return ParseNumericField<SInt64Traits>(input, fieldNumber, fieldInfo, wireType, _values);
    }
    
    bool LVMessage::ParseFixed32Field(google::protobuf::io::CodedInputStream* input,
                                       uint32_t fieldNumber,
                                       const MessageElementMetadata& fieldInfo,
                                       uint32_t wireType)
    {
        return ParseNumericField<Fixed32Traits>(input, fieldNumber, fieldInfo, wireType, _values);
    }
    
    bool LVMessage::ParseFixed64Field(google::protobuf::io::CodedInputStream* input,
                                       uint32_t fieldNumber,
                                       const MessageElementMetadata& fieldInfo,
                                       uint32_t wireType)
    {
        return ParseNumericField<Fixed64Traits>(input, fieldNumber, fieldInfo, wireType, _values);
    }
    
    bool LVMessage::ParseSFixed32Field(google::protobuf::io::CodedInputStream* input,
                                        uint32_t fieldNumber,
                                        const MessageElementMetadata& fieldInfo,
                                        uint32_t wireType)
    {
        return ParseNumericField<SFixed32Traits>(input, fieldNumber, fieldInfo, wireType, _values);
    }
    
    bool LVMessage::ParseSFixed64Field(google::protobuf::io::CodedInputStream* input,
                                        uint32_t fieldNumber,
                                        const MessageElementMetadata& fieldInfo,
                                        uint32_t wireType)
    {
        return ParseNumericField<SFixed64Traits>(input, fieldNumber, fieldInfo, wireType, _values);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    std::unique_ptr<grpc::ByteBuffer> LVMessage::SerializeToByteBuffer() const
    {
        auto size = ByteSizeLong();
        if (size == 0)
        {
            // gRPC does not accept a default-constructed ByteBuffer (null internal
            // buffer); send a valid empty ByteBuffer instead.
            grpc::Slice buf(0);
            return std::make_unique<grpc::ByteBuffer>(&buf, 1);
        }

        if (size > static_cast<size_t>(INT_MAX))
        {
            return nullptr;
        }

        grpc::Slice buf(size);
        {
            google::protobuf::io::ArrayOutputStream aos(const_cast<uint8_t*>(buf.begin()), static_cast<int>(size));
            google::protobuf::io::CodedOutputStream cos(&aos);
            for (auto& e : _values)
            {
                e.second->Serialize(&cos);
            }
        } // CodedOutputStream flushes on destruction

        return std::make_unique<grpc::ByteBuffer>(&buf, 1);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    google::protobuf::UnknownFieldSet& LVMessage::UnknownFields()
    {
        return _unknownFields;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessage::SerializeToString(std::string* output) const
    {
        output->clear();
        google::protobuf::io::StringOutputStream sos(output);
        google::protobuf::io::CodedOutputStream cos(&sos);
        SerializeToCodedStream(&cos);
        return !cos.HadError();
    }

    //---------------------------------------------------------------------
    // Clear() destroys every LVMessageValue object in _values, which is
    // sufficient to reset all per-value byte-size caches (_cachedDataSize,
    // _cachedNestedByteSize, etc.).  New LVMessageValue objects created during
    // the next parse start with the sentinel value (-1), guaranteeing that
    // ByteSizeLong() recomputes before Serialize() is called for the new write.
    // There is therefore no need to iterate over _values to invalidate caches
    // individually.
    //---------------------------------------------------------------------
    void LVMessage::Clear()
    {
        _values.clear();
        _oneofContainerToSelectedIndexMap.clear();
        _unknownFields.Clear();
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVMessage::SerializeToCodedStream(google::protobuf::io::CodedOutputStream* output) const
    {
        for (auto& e : _values)
        {
            e.second->Serialize(output);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    size_t LVMessage::ByteSizeLong() const
    {
        size_t totalSize = 0;
        for (auto& e : _values)
        {
            totalSize += e.second->ByteSizeLong();
        }
        return totalSize;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessage::IsInitialized() const
    {
        return true;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVMessage::CopyOneofIndicesToCluster(int8_t* cluster) const
    {
        if (_oneofContainerToSelectedIndexMap.size() > 0)
        {
            // Must iterate over _elements and not _mappedElements since all oneof selected_index fields use -1 for the field number
            // and there can be multiple oneof fields in a message.
            for (auto& fieldMetadata : _metadata->_elements)
            {
                if (fieldMetadata->isInOneof && fieldMetadata->protobufIndex < 0)
                {
                    // This field is the selected_index field of a oneof. This field only exists in the cluster
                    // and is not a field in the message.
                    auto it = _oneofContainerToSelectedIndexMap.find(fieldMetadata->oneofContainerName);
                    if (it != _oneofContainerToSelectedIndexMap.end())
                    {
                        auto selectedIndexPtr = reinterpret_cast<int*>(cluster + fieldMetadata->clusterOffset);
                        *selectedIndexPtr = it->second;
                    }
                }
            }
        }
    }
}
