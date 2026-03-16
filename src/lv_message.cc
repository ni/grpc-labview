//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_server.h>
#include <lv_message.h>
#include <climits>
#include <sstream>
#include <feature_toggles.h>
#include <string_utils.h>
#include <google/protobuf/io/coded_stream.h>
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

    inline bool SkipField(google::protobuf::io::CodedInputStream* input, uint32_t tag)
    {
        return HandleUnknownField(input, tag, nullptr);
    }

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

        // Extract bytes from ByteBuffer
        std::vector<grpc::Slice> slices;
        buffer.Dump(&slices);
        std::string buf;
        buf.reserve(buffer.Length());
        for (auto s = slices.begin(); s != slices.end(); s++)
        {
            buf.append(reinterpret_cast<const char *>(s->begin()), s->size());
        }

        return ParseFromString(buf);
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
    // Helper methods to parse specific field types from CodedInputStream
    //---------------------------------------------------------------------
    bool LVMessage::ParseInt32Field(google::protobuf::io::CodedInputStream* input, 
                                     uint32_t fieldNumber, 
                                     const MessageElementMetadata& fieldInfo,
                                     uint32_t wireType)
    {
        if (fieldInfo.isRepeated)
        {
            if (wireType == WFL::WIRETYPE_LENGTH_DELIMITED) // packed
            {
                auto v = std::make_shared<LVRepeatedMessageValue<int>>(fieldNumber);
                uint32_t length;
                if (!input->ReadVarint32(&length)) return false;
                auto limit = input->PushLimit(length);
                while (input->BytesUntilLimit() > 0)
                {
                    uint32_t value;
                    if (!input->ReadVarint32(&value)) return false;
                    v->_value.Add(static_cast<int32_t>(value));
                }
                input->PopLimit(limit);
                _values.emplace(fieldNumber, v);
            }
            else // unpacked: single element per tag occurrence
            {
                uint32_t value;
                if (!input->ReadVarint32(&value)) return false;
                auto it = _values.find(fieldNumber);
                if (it == _values.end())
                {
                    auto v = std::make_shared<LVRepeatedMessageValue<int>>(fieldNumber);
                    v->_value.Add(static_cast<int32_t>(value));
                    _values.emplace(fieldNumber, v);
                }
                else
                {
                    auto v = std::static_pointer_cast<LVRepeatedMessageValue<int>>(it->second);
                    v->_value.Add(static_cast<int32_t>(value));
                }
            }
        }
        else
        {
            uint32_t value;
            if (!input->ReadVarint32(&value)) return false;
            auto v = std::make_shared<LVVariableMessageValue<int>>(fieldNumber, static_cast<int32_t>(value));
            _values.emplace(fieldNumber, v);
        }
        return true;
    }
    
    bool LVMessage::ParseInt64Field(google::protobuf::io::CodedInputStream* input,
                                     uint32_t fieldNumber,
                                     const MessageElementMetadata& fieldInfo,
                                     uint32_t wireType)
    {
        if (fieldInfo.isRepeated)
        {
            if (wireType == WFL::WIRETYPE_LENGTH_DELIMITED) // packed
            {
                auto v = std::make_shared<LVRepeatedMessageValue<int64_t>>(fieldNumber);
                uint32_t length;
                if (!input->ReadVarint32(&length)) return false;
                auto limit = input->PushLimit(length);
                while (input->BytesUntilLimit() > 0)
                {
                    uint64_t value;
                    if (!input->ReadVarint64(&value)) return false;
                    v->_value.Add(static_cast<int64_t>(value));
                }
                input->PopLimit(limit);
                _values.emplace(fieldNumber, v);
            }
            else // unpacked: single element per tag occurrence
            {
                uint64_t value;
                if (!input->ReadVarint64(&value)) return false;
                auto it = _values.find(fieldNumber);
                if (it == _values.end())
                {
                    auto v = std::make_shared<LVRepeatedMessageValue<int64_t>>(fieldNumber);
                    v->_value.Add(static_cast<int64_t>(value));
                    _values.emplace(fieldNumber, v);
                }
                else
                {
                    auto v = std::static_pointer_cast<LVRepeatedMessageValue<int64_t>>(it->second);
                    v->_value.Add(static_cast<int64_t>(value));
                }
            }
        }
        else
        {
            uint64_t value;
            if (!input->ReadVarint64(&value)) return false;
            auto v = std::make_shared<LVVariableMessageValue<int64_t>>(fieldNumber, static_cast<int64_t>(value));
            _values.emplace(fieldNumber, v);
        }
        return true;
    }
    
    bool LVMessage::ParseUInt32Field(google::protobuf::io::CodedInputStream* input,
                                      uint32_t fieldNumber,
                                      const MessageElementMetadata& fieldInfo,
                                      uint32_t wireType)
    {
        if (fieldInfo.isRepeated)
        {
            if (wireType == WFL::WIRETYPE_LENGTH_DELIMITED) // packed
            {
                auto v = std::make_shared<LVRepeatedMessageValue<uint32_t>>(fieldNumber);
                uint32_t length;
                if (!input->ReadVarint32(&length)) return false;
                auto limit = input->PushLimit(length);
                while (input->BytesUntilLimit() > 0)
                {
                    uint32_t value;
                    if (!input->ReadVarint32(&value)) return false;
                    v->_value.Add(value);
                }
                input->PopLimit(limit);
                _values.emplace(fieldNumber, v);
            }
            else // unpacked: single element per tag occurrence
            {
                uint32_t value;
                if (!input->ReadVarint32(&value)) return false;
                auto it = _values.find(fieldNumber);
                if (it == _values.end())
                {
                    auto v = std::make_shared<LVRepeatedMessageValue<uint32_t>>(fieldNumber);
                    v->_value.Add(value);
                    _values.emplace(fieldNumber, v);
                }
                else
                {
                    auto v = std::static_pointer_cast<LVRepeatedMessageValue<uint32_t>>(it->second);
                    v->_value.Add(value);
                }
            }
        }
        else
        {
            uint32_t value;
            if (!input->ReadVarint32(&value)) return false;
            auto v = std::make_shared<LVVariableMessageValue<uint32_t>>(fieldNumber, value);
            _values.emplace(fieldNumber, v);
        }
        return true;
    }
    
    bool LVMessage::ParseUInt64Field(google::protobuf::io::CodedInputStream* input,
                                      uint32_t fieldNumber,
                                      const MessageElementMetadata& fieldInfo,
                                      uint32_t wireType)
    {
        if (fieldInfo.isRepeated)
        {
            if (wireType == WFL::WIRETYPE_LENGTH_DELIMITED) // packed
            {
                auto v = std::make_shared<LVRepeatedMessageValue<uint64_t>>(fieldNumber);
                uint32_t length;
                if (!input->ReadVarint32(&length)) return false;
                auto limit = input->PushLimit(length);
                while (input->BytesUntilLimit() > 0)
                {
                    uint64_t value;
                    if (!input->ReadVarint64(&value)) return false;
                    v->_value.Add(value);
                }
                input->PopLimit(limit);
                _values.emplace(fieldNumber, v);
            }
            else // unpacked: single element per tag occurrence
            {
                uint64_t value;
                if (!input->ReadVarint64(&value)) return false;
                auto it = _values.find(fieldNumber);
                if (it == _values.end())
                {
                    auto v = std::make_shared<LVRepeatedMessageValue<uint64_t>>(fieldNumber);
                    v->_value.Add(value);
                    _values.emplace(fieldNumber, v);
                }
                else
                {
                    auto v = std::static_pointer_cast<LVRepeatedMessageValue<uint64_t>>(it->second);
                    v->_value.Add(value);
                }
            }
        }
        else
        {
            uint64_t value;
            if (!input->ReadVarint64(&value)) return false;
            auto v = std::make_shared<LVVariableMessageValue<uint64_t>>(fieldNumber, value);
            _values.emplace(fieldNumber, v);
        }
        return true;
    }
    
    bool LVMessage::ParseFloatField(google::protobuf::io::CodedInputStream* input,
                                     uint32_t fieldNumber,
                                     const MessageElementMetadata& fieldInfo,
                                     uint32_t wireType)
    {
        if (fieldInfo.isRepeated)
        {
            if (wireType == WFL::WIRETYPE_LENGTH_DELIMITED) // packed
            {
                auto v = std::make_shared<LVRepeatedMessageValue<float>>(fieldNumber);
                uint32_t length;
                if (!input->ReadVarint32(&length)) return false;
                auto limit = input->PushLimit(length);
                while (input->BytesUntilLimit() > 0)
                {
                    uint32_t value;
                    if (!input->ReadLittleEndian32(&value)) return false;
                    float f;
                    memcpy(&f, &value, sizeof(float));
                    v->_value.Add(f);
                }
                input->PopLimit(limit);
                _values.emplace(fieldNumber, v);
            }
            else // unpacked: single element per tag occurrence
            {
                uint32_t value;
                if (!input->ReadLittleEndian32(&value)) return false;
                float f;
                memcpy(&f, &value, sizeof(float));
                auto it = _values.find(fieldNumber);
                if (it == _values.end())
                {
                    auto v = std::make_shared<LVRepeatedMessageValue<float>>(fieldNumber);
                    v->_value.Add(f);
                    _values.emplace(fieldNumber, v);
                }
                else
                {
                    auto v = std::static_pointer_cast<LVRepeatedMessageValue<float>>(it->second);
                    v->_value.Add(f);
                }
            }
        }
        else
        {
            uint32_t value;
            if (!input->ReadLittleEndian32(&value)) return false;
            float f;
            memcpy(&f, &value, sizeof(float));
            auto v = std::make_shared<LVVariableMessageValue<float>>(fieldNumber, f);
            _values.emplace(fieldNumber, v);
        }
        return true;
    }
    
    bool LVMessage::ParseDoubleField(google::protobuf::io::CodedInputStream* input,
                                      uint32_t fieldNumber,
                                      const MessageElementMetadata& fieldInfo,
                                      uint32_t wireType)
    {
        if (fieldInfo.isRepeated)
        {
            if (wireType == WFL::WIRETYPE_LENGTH_DELIMITED) // packed
            {
                auto v = std::make_shared<LVRepeatedMessageValue<double>>(fieldNumber);
                uint32_t length;
                if (!input->ReadVarint32(&length)) return false;
                auto limit = input->PushLimit(length);
                while (input->BytesUntilLimit() > 0)
                {
                    uint64_t value;
                    if (!input->ReadLittleEndian64(&value)) return false;
                    double d;
                    memcpy(&d, &value, sizeof(double));
                    v->_value.Add(d);
                }
                input->PopLimit(limit);
                _values.emplace(fieldNumber, v);
            }
            else // unpacked: single element per tag occurrence
            {
                uint64_t value;
                if (!input->ReadLittleEndian64(&value)) return false;
                double d;
                memcpy(&d, &value, sizeof(double));
                auto it = _values.find(fieldNumber);
                if (it == _values.end())
                {
                    auto v = std::make_shared<LVRepeatedMessageValue<double>>(fieldNumber);
                    v->_value.Add(d);
                    _values.emplace(fieldNumber, v);
                }
                else
                {
                    auto v = std::static_pointer_cast<LVRepeatedMessageValue<double>>(it->second);
                    v->_value.Add(d);
                }
            }
        }
        else
        {
            uint64_t value;
            if (!input->ReadLittleEndian64(&value)) return false;
            double d;
            memcpy(&d, &value, sizeof(double));
            auto v = std::make_shared<LVVariableMessageValue<double>>(fieldNumber, d);
            _values.emplace(fieldNumber, v);
        }
        return true;
    }
    
    bool LVMessage::ParseBoolField(google::protobuf::io::CodedInputStream* input,
                                    uint32_t fieldNumber,
                                    const MessageElementMetadata& fieldInfo,
                                    uint32_t wireType)
    {
        if (fieldInfo.isRepeated)
        {
            if (wireType == WFL::WIRETYPE_LENGTH_DELIMITED) // packed
            {
                auto v = std::make_shared<LVRepeatedMessageValue<bool>>(fieldNumber);
                uint32_t length;
                if (!input->ReadVarint32(&length)) return false;
                auto limit = input->PushLimit(length);
                while (input->BytesUntilLimit() > 0)
                {
                    uint64_t value;
                    if (!input->ReadVarint64(&value)) return false;
                    v->_value.Add(value != 0);
                }
                input->PopLimit(limit);
                _values.emplace(fieldNumber, v);
            }
            else // unpacked: single element per tag occurrence
            {
                uint64_t value;
                if (!input->ReadVarint64(&value)) return false;
                auto it = _values.find(fieldNumber);
                if (it == _values.end())
                {
                    auto v = std::make_shared<LVRepeatedMessageValue<bool>>(fieldNumber);
                    v->_value.Add(value != 0);
                    _values.emplace(fieldNumber, v);
                }
                else
                {
                    auto v = std::static_pointer_cast<LVRepeatedMessageValue<bool>>(it->second);
                    v->_value.Add(value != 0);
                }
            }
        }
        else
        {
            uint64_t value;
            if (!input->ReadVarint64(&value)) return false;
            auto v = std::make_shared<LVVariableMessageValue<bool>>(fieldNumber, value != 0);
            _values.emplace(fieldNumber, v);
        }
        return true;
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
        if (fieldInfo.isRepeated)
        {
            if (wireType == WFL::WIRETYPE_LENGTH_DELIMITED) // packed
            {
                auto v = std::make_shared<LVRepeatedEnumMessageValue>(fieldNumber);
                uint32_t length;
                if (!input->ReadVarint32(&length)) return false;
                auto limit = input->PushLimit(length);
                while (input->BytesUntilLimit() > 0)
                {
                    uint32_t value;
                    if (!input->ReadVarint32(&value)) return false;
                    v->_value.Add(static_cast<int32_t>(value));
                }
                input->PopLimit(limit);
                _values.emplace(fieldNumber, v);
            }
            else // unpacked: single element per tag occurrence
            {
                uint32_t value;
                if (!input->ReadVarint32(&value)) return false;
                auto it = _values.find(fieldNumber);
                if (it == _values.end())
                {
                    auto v = std::make_shared<LVRepeatedEnumMessageValue>(fieldNumber);
                    v->_value.Add(static_cast<int32_t>(value));
                    _values.emplace(fieldNumber, v);
                }
                else
                {
                    auto v = std::static_pointer_cast<LVRepeatedEnumMessageValue>(it->second);
                    v->_value.Add(static_cast<int32_t>(value));
                }
            }
        }
        else
        {
            uint32_t value;
            if (!input->ReadVarint32(&value)) return false;
            auto v = std::make_shared<LVEnumMessageValue>(fieldNumber, static_cast<int32_t>(value));
            _values.emplace(fieldNumber, v);
        }
        return true;
    }
    
    bool LVMessage::ParseSInt32Field(google::protobuf::io::CodedInputStream* input,
                                      uint32_t fieldNumber,
                                      const MessageElementMetadata& fieldInfo,
                                      uint32_t wireType)
    {
        if (fieldInfo.isRepeated)
        {
            if (wireType == WFL::WIRETYPE_LENGTH_DELIMITED) // packed
            {
                auto v = std::make_shared<LVRepeatedSInt32MessageValue>(fieldNumber);
                uint32_t length;
                if (!input->ReadVarint32(&length)) return false;
                auto limit = input->PushLimit(length);
                while (input->BytesUntilLimit() > 0)
                {
                    uint32_t value;
                    if (!input->ReadVarint32(&value)) return false;
                    v->_value.Add(WFL::ZigZagDecode32(value));
                }
                input->PopLimit(limit);
                _values.emplace(fieldNumber, v);
            }
            else // unpacked: single element per tag occurrence
            {
                uint32_t value;
                if (!input->ReadVarint32(&value)) return false;
                auto it = _values.find(fieldNumber);
                if (it == _values.end())
                {
                    auto v = std::make_shared<LVRepeatedSInt32MessageValue>(fieldNumber);
                    v->_value.Add(WFL::ZigZagDecode32(value));
                    _values.emplace(fieldNumber, v);
                }
                else
                {
                    auto v = std::static_pointer_cast<LVRepeatedSInt32MessageValue>(it->second);
                    v->_value.Add(WFL::ZigZagDecode32(value));
                }
            }
        }
        else
        {
            uint32_t value;
            if (!input->ReadVarint32(&value)) return false;
            auto v = std::make_shared<LVSInt32MessageValue>(fieldNumber, WFL::ZigZagDecode32(value));
            _values.emplace(fieldNumber, v);
        }
        return true;
    }
    
    bool LVMessage::ParseSInt64Field(google::protobuf::io::CodedInputStream* input,
                                      uint32_t fieldNumber,
                                      const MessageElementMetadata& fieldInfo,
                                      uint32_t wireType)
    {
        if (fieldInfo.isRepeated)
        {
            if (wireType == WFL::WIRETYPE_LENGTH_DELIMITED) // packed
            {
                auto v = std::make_shared<LVRepeatedSInt64MessageValue>(fieldNumber);
                uint32_t length;
                if (!input->ReadVarint32(&length)) return false;
                auto limit = input->PushLimit(length);
                while (input->BytesUntilLimit() > 0)
                {
                    uint64_t value;
                    if (!input->ReadVarint64(&value)) return false;
                    v->_value.Add(WFL::ZigZagDecode64(value));
                }
                input->PopLimit(limit);
                _values.emplace(fieldNumber, v);
            }
            else // unpacked: single element per tag occurrence
            {
                uint64_t value;
                if (!input->ReadVarint64(&value)) return false;
                auto it = _values.find(fieldNumber);
                if (it == _values.end())
                {
                    auto v = std::make_shared<LVRepeatedSInt64MessageValue>(fieldNumber);
                    v->_value.Add(WFL::ZigZagDecode64(value));
                    _values.emplace(fieldNumber, v);
                }
                else
                {
                    auto v = std::static_pointer_cast<LVRepeatedSInt64MessageValue>(it->second);
                    v->_value.Add(WFL::ZigZagDecode64(value));
                }
            }
        }
        else
        {
            uint64_t value;
            if (!input->ReadVarint64(&value)) return false;
            auto v = std::make_shared<LVSInt64MessageValue>(fieldNumber, WFL::ZigZagDecode64(value));
            _values.emplace(fieldNumber, v);
        }
        return true;
    }
    
    bool LVMessage::ParseFixed32Field(google::protobuf::io::CodedInputStream* input,
                                       uint32_t fieldNumber,
                                       const MessageElementMetadata& fieldInfo,
                                       uint32_t wireType)
    {
        if (fieldInfo.isRepeated)
        {
            if (wireType == WFL::WIRETYPE_LENGTH_DELIMITED) // packed
            {
                auto v = std::make_shared<LVRepeatedFixed32MessageValue>(fieldNumber);
                uint32_t length;
                if (!input->ReadVarint32(&length)) return false;
                auto limit = input->PushLimit(length);
                while (input->BytesUntilLimit() > 0)
                {
                    uint32_t value;
                    if (!input->ReadLittleEndian32(&value)) return false;
                    v->_value.Add(value);
                }
                input->PopLimit(limit);
                _values.emplace(fieldNumber, v);
            }
            else // unpacked: single element per tag occurrence
            {
                uint32_t value;
                if (!input->ReadLittleEndian32(&value)) return false;
                auto it = _values.find(fieldNumber);
                if (it == _values.end())
                {
                    auto v = std::make_shared<LVRepeatedFixed32MessageValue>(fieldNumber);
                    v->_value.Add(value);
                    _values.emplace(fieldNumber, v);
                }
                else
                {
                    auto v = std::static_pointer_cast<LVRepeatedFixed32MessageValue>(it->second);
                    v->_value.Add(value);
                }
            }
        }
        else
        {
            uint32_t value;
            if (!input->ReadLittleEndian32(&value)) return false;
            auto v = std::make_shared<LVFixed32MessageValue>(fieldNumber, value);
            _values.emplace(fieldNumber, v);
        }
        return true;
    }
    
    bool LVMessage::ParseFixed64Field(google::protobuf::io::CodedInputStream* input,
                                       uint32_t fieldNumber,
                                       const MessageElementMetadata& fieldInfo,
                                       uint32_t wireType)
    {
        if (fieldInfo.isRepeated)
        {
            if (wireType == WFL::WIRETYPE_LENGTH_DELIMITED) // packed
            {
                auto v = std::make_shared<LVRepeatedFixed64MessageValue>(fieldNumber);
                uint32_t length;
                if (!input->ReadVarint32(&length)) return false;
                auto limit = input->PushLimit(length);
                while (input->BytesUntilLimit() > 0)
                {
                    uint64_t value;
                    if (!input->ReadLittleEndian64(&value)) return false;
                    v->_value.Add(value);
                }
                input->PopLimit(limit);
                _values.emplace(fieldNumber, v);
            }
            else // unpacked: single element per tag occurrence
            {
                uint64_t value;
                if (!input->ReadLittleEndian64(&value)) return false;
                auto it = _values.find(fieldNumber);
                if (it == _values.end())
                {
                    auto v = std::make_shared<LVRepeatedFixed64MessageValue>(fieldNumber);
                    v->_value.Add(value);
                    _values.emplace(fieldNumber, v);
                }
                else
                {
                    auto v = std::static_pointer_cast<LVRepeatedFixed64MessageValue>(it->second);
                    v->_value.Add(value);
                }
            }
        }
        else
        {
            uint64_t value;
            if (!input->ReadLittleEndian64(&value)) return false;
            auto v = std::make_shared<LVFixed64MessageValue>(fieldNumber, value);
            _values.emplace(fieldNumber, v);
        }
        return true;
    }
    
    bool LVMessage::ParseSFixed32Field(google::protobuf::io::CodedInputStream* input,
                                        uint32_t fieldNumber,
                                        const MessageElementMetadata& fieldInfo,
                                        uint32_t wireType)
    {
        if (fieldInfo.isRepeated)
        {
            if (wireType == WFL::WIRETYPE_LENGTH_DELIMITED) // packed
            {
                auto v = std::make_shared<LVRepeatedSFixed32MessageValue>(fieldNumber);
                uint32_t length;
                if (!input->ReadVarint32(&length)) return false;
                auto limit = input->PushLimit(length);
                while (input->BytesUntilLimit() > 0)
                {
                    uint32_t value;
                    if (!input->ReadLittleEndian32(&value)) return false;
                    v->_value.Add(static_cast<int32_t>(value));
                }
                input->PopLimit(limit);
                _values.emplace(fieldNumber, v);
            }
            else // unpacked: single element per tag occurrence
            {
                uint32_t value;
                if (!input->ReadLittleEndian32(&value)) return false;
                auto it = _values.find(fieldNumber);
                if (it == _values.end())
                {
                    auto v = std::make_shared<LVRepeatedSFixed32MessageValue>(fieldNumber);
                    v->_value.Add(static_cast<int32_t>(value));
                    _values.emplace(fieldNumber, v);
                }
                else
                {
                    auto v = std::static_pointer_cast<LVRepeatedSFixed32MessageValue>(it->second);
                    v->_value.Add(static_cast<int32_t>(value));
                }
            }
        }
        else
        {
            uint32_t value;
            if (!input->ReadLittleEndian32(&value)) return false;
            auto v = std::make_shared<LVSFixed32MessageValue>(fieldNumber, static_cast<int32_t>(value));
            _values.emplace(fieldNumber, v);
        }
        return true;
    }
    
    bool LVMessage::ParseSFixed64Field(google::protobuf::io::CodedInputStream* input,
                                        uint32_t fieldNumber,
                                        const MessageElementMetadata& fieldInfo,
                                        uint32_t wireType)
    {
        if (fieldInfo.isRepeated)
        {
            if (wireType == WFL::WIRETYPE_LENGTH_DELIMITED) // packed
            {
                auto v = std::make_shared<LVRepeatedSFixed64MessageValue>(fieldNumber);
                uint32_t length;
                if (!input->ReadVarint32(&length)) return false;
                auto limit = input->PushLimit(length);
                while (input->BytesUntilLimit() > 0)
                {
                    uint64_t value;
                    if (!input->ReadLittleEndian64(&value)) return false;
                    v->_value.Add(static_cast<int64_t>(value));
                }
                input->PopLimit(limit);
                _values.emplace(fieldNumber, v);
            }
            else // unpacked: single element per tag occurrence
            {
                uint64_t value;
                if (!input->ReadLittleEndian64(&value)) return false;
                auto it = _values.find(fieldNumber);
                if (it == _values.end())
                {
                    auto v = std::make_shared<LVRepeatedSFixed64MessageValue>(fieldNumber);
                    v->_value.Add(static_cast<int64_t>(value));
                    _values.emplace(fieldNumber, v);
                }
                else
                {
                    auto v = std::static_pointer_cast<LVRepeatedSFixed64MessageValue>(it->second);
                    v->_value.Add(static_cast<int64_t>(value));
                }
            }
        }
        else
        {
            uint64_t value;
            if (!input->ReadLittleEndian64(&value)) return false;
            auto v = std::make_shared<LVSFixed64MessageValue>(fieldNumber, static_cast<int64_t>(value));
            _values.emplace(fieldNumber, v);
        }
        return true;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    std::unique_ptr<grpc::ByteBuffer> LVMessage::SerializeToByteBuffer() const
    {
        std::string buf;
        if (!SerializeToString(&buf) || buf.empty())
        {
            return std::make_unique<grpc::ByteBuffer>();
        }
        grpc::Slice slice(buf);
        return std::make_unique<grpc::ByteBuffer>(&slice, 1);
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
        for (auto& e : _values)
            e.second->Serialize(&cos);
        return !cos.HadError();
    }

    //---------------------------------------------------------------------
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
