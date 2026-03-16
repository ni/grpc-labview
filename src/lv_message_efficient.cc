//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_server.h>
#include <lv_message_efficient.h>
#include <well_known_messages.h>
#include <sstream>
#include <feature_toggles.h>
#include <string_utils.h>
#include <google/protobuf/wire_format_lite.h>

namespace {
    inline int32_t ZigZagDecode32(uint32_t n) { return google::protobuf::internal::WireFormatLite::ZigZagDecode32(n); }
    inline int64_t ZigZagDecode64(uint64_t n) { return google::protobuf::internal::WireFormatLite::ZigZagDecode64(n); }
    constexpr uint32_t WIRETYPE_LENGTH_DELIMITED = 2;
}

namespace grpc_labview
{
    template<typename T>
    static void AppendToLVArray(T val, int8_t* lv_ptr)
    {
        auto arr = *(LV1DArrayHandle*)lv_ptr;
        int32_t cnt = (arr != nullptr) ? (*arr)->cnt : 0;
        NumericArrayResize(GetTypeCodeForSize(sizeof(T)), 1, reinterpret_cast<void*>(lv_ptr), static_cast<size_t>(cnt) + 1);
        arr = *(LV1DArrayHandle*)lv_ptr;
        (*arr)->cnt = cnt + 1;
        (*arr)->bytes<T>()[cnt] = val;
    }

    template<typename T>
    static void AppendVectorToLVArray(const std::vector<T>& vals, int8_t* lv_ptr)
    {
        if (vals.empty()) return;
        auto arr = *(LV1DArrayHandle*)lv_ptr;
        int32_t existing = (arr != nullptr) ? (*arr)->cnt : 0;
        size_t newCount = static_cast<size_t>(existing) + vals.size();
        NumericArrayResize(GetTypeCodeForSize(sizeof(T)), 1, reinterpret_cast<void*>(lv_ptr), newCount);
        arr = *(LV1DArrayHandle*)lv_ptr;
        (*arr)->cnt = static_cast<int32_t>(newCount);
        std::memcpy((*arr)->bytes<T>() + existing, vals.data(), vals.size() * sizeof(T));
    }
    //---------------------------------------------------------------------
    // Traits-based generic numeric field parser.
    //
    // Each traits struct defines:
    //   RawType    – type read from the wire by Read()
    //   StoredType – type written into the LV cluster (may differ from RawType)
    //   Read()     – reads one RawType from a CodedInputStream
    //   Transform()– converts RawType to the StoredType stored in the cluster
    //
    // ParseNumericField<Traits> captures the shared packed/unpacked/scalar
    // branching logic. Each template instantiation compiles to the same
    // machine code as the equivalent hand-written method would produce.
    // BoolTraits uses uint8_t as StoredType to avoid std::vector<bool> issues.
    //---------------------------------------------------------------------
    namespace
    {
        struct Int32Traits {
            using RawType = uint32_t;
            using StoredType = int32_t;
            static bool Read(google::protobuf::io::CodedInputStream* s, RawType& out) { return s->ReadVarint32(&out); }
            static StoredType Transform(RawType v) { return static_cast<int32_t>(v); }
        };
        struct UInt32Traits {
            using RawType = uint32_t;
            using StoredType = uint32_t;
            static bool Read(google::protobuf::io::CodedInputStream* s, RawType& out) { return s->ReadVarint32(&out); }
            static StoredType Transform(RawType v) { return v; }
        };
        struct Int64Traits {
            using RawType = uint64_t;
            using StoredType = int64_t;
            static bool Read(google::protobuf::io::CodedInputStream* s, RawType& out) { return s->ReadVarint64(&out); }
            static StoredType Transform(RawType v) { return static_cast<int64_t>(v); }
        };
        struct UInt64Traits {
            using RawType = uint64_t;
            using StoredType = uint64_t;
            static bool Read(google::protobuf::io::CodedInputStream* s, RawType& out) { return s->ReadVarint64(&out); }
            static StoredType Transform(RawType v) { return v; }
        };
        struct BoolTraits {
            using RawType = uint64_t;
            using StoredType = uint8_t;  // uint8_t avoids std::vector<bool> specialisation
            static bool Read(google::protobuf::io::CodedInputStream* s, RawType& out) { return s->ReadVarint64(&out); }
            static StoredType Transform(RawType v) { return static_cast<uint8_t>(v != 0); }
        };
        struct FloatTraits {
            using RawType = uint32_t;
            using StoredType = float;
            static bool Read(google::protobuf::io::CodedInputStream* s, RawType& out) { return s->ReadLittleEndian32(&out); }
            static StoredType Transform(RawType v) { float f; memcpy(&f, &v, sizeof(f)); return f; }
        };
        struct DoubleTraits {
            using RawType = uint64_t;
            using StoredType = double;
            static bool Read(google::protobuf::io::CodedInputStream* s, RawType& out) { return s->ReadLittleEndian64(&out); }
            static StoredType Transform(RawType v) { double d; memcpy(&d, &v, sizeof(d)); return d; }
        };
        struct SInt32Traits {
            using RawType = uint32_t;
            using StoredType = int32_t;
            static bool Read(google::protobuf::io::CodedInputStream* s, RawType& out) { return s->ReadVarint32(&out); }
            static StoredType Transform(RawType v) { return ZigZagDecode32(v); }
        };
        struct SInt64Traits {
            using RawType = uint64_t;
            using StoredType = int64_t;
            static bool Read(google::protobuf::io::CodedInputStream* s, RawType& out) { return s->ReadVarint64(&out); }
            static StoredType Transform(RawType v) { return ZigZagDecode64(v); }
        };
        struct Fixed32Traits {
            using RawType = uint32_t;
            using StoredType = uint32_t;
            static bool Read(google::protobuf::io::CodedInputStream* s, RawType& out) { return s->ReadLittleEndian32(&out); }
            static StoredType Transform(RawType v) { return v; }
        };
        struct Fixed64Traits {
            using RawType = uint64_t;
            using StoredType = uint64_t;
            static bool Read(google::protobuf::io::CodedInputStream* s, RawType& out) { return s->ReadLittleEndian64(&out); }
            static StoredType Transform(RawType v) { return v; }
        };
        struct SFixed32Traits {
            using RawType = uint32_t;
            using StoredType = int32_t;
            static bool Read(google::protobuf::io::CodedInputStream* s, RawType& out) { return s->ReadLittleEndian32(&out); }
            static StoredType Transform(RawType v) { return static_cast<int32_t>(v); }
        };
        struct SFixed64Traits {
            using RawType = uint64_t;
            using StoredType = int64_t;
            static bool Read(google::protobuf::io::CodedInputStream* s, RawType& out) { return s->ReadLittleEndian64(&out); }
            static StoredType Transform(RawType v) { return static_cast<int64_t>(v); }
        };

        template<typename Traits>
        bool ParseNumericField(
            google::protobuf::io::CodedInputStream* input,
            const MessageElementMetadata& fieldInfo,
            uint32_t wireType,
            int8_t* lv_ptr)
        {
            using RawType = typename Traits::RawType;
            using StoredType = typename Traits::StoredType;
            if (fieldInfo.isRepeated)
            {
                if (wireType == WIRETYPE_LENGTH_DELIMITED) // packed
                {
                    uint32_t length;
                    if (!input->ReadVarint32(&length)) return false;
                    auto limit = input->PushLimit(static_cast<int>(length));
                    std::vector<StoredType> vals;
                    while (input->BytesUntilLimit() > 0)
                    {
                        RawType raw;
                        if (!Traits::Read(input, raw)) { input->PopLimit(limit); return false; }
                        vals.push_back(Traits::Transform(raw));
                    }
                    input->PopLimit(limit);
                    AppendVectorToLVArray(vals, lv_ptr);
                }
                else // unpacked: single element per tag occurrence
                {
                    RawType raw;
                    if (!Traits::Read(input, raw)) return false;
                    AppendToLVArray(Traits::Transform(raw), lv_ptr);
                }
            }
            else
            {
                RawType raw;
                if (!Traits::Read(input, raw)) return false;
                *reinterpret_cast<StoredType*>(lv_ptr) = Traits::Transform(raw);
            }
            return true;
        }
    } // anonymous namespace

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseInt32Field(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo, uint32_t wireType)
    {
        return ParseNumericField<Int32Traits>(input, fieldInfo, wireType, _LVClusterHandle + fieldInfo.clusterOffset);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseUInt32Field(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo, uint32_t wireType)
    {
        return ParseNumericField<UInt32Traits>(input, fieldInfo, wireType, _LVClusterHandle + fieldInfo.clusterOffset);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseInt64Field(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo, uint32_t wireType)
    {
        return ParseNumericField<Int64Traits>(input, fieldInfo, wireType, _LVClusterHandle + fieldInfo.clusterOffset);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseUInt64Field(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo, uint32_t wireType)
    {
        return ParseNumericField<UInt64Traits>(input, fieldInfo, wireType, _LVClusterHandle + fieldInfo.clusterOffset);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseBoolField(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo, uint32_t wireType)
    {
        return ParseNumericField<BoolTraits>(input, fieldInfo, wireType, _LVClusterHandle + fieldInfo.clusterOffset);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseFloatField(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo, uint32_t wireType)
    {
        return ParseNumericField<FloatTraits>(input, fieldInfo, wireType, _LVClusterHandle + fieldInfo.clusterOffset);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseDoubleField(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo, uint32_t wireType)
    {
        return ParseNumericField<DoubleTraits>(input, fieldInfo, wireType, _LVClusterHandle + fieldInfo.clusterOffset);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseSInt32Field(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo, uint32_t wireType)
    {
        return ParseNumericField<SInt32Traits>(input, fieldInfo, wireType, _LVClusterHandle + fieldInfo.clusterOffset);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseSInt64Field(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo, uint32_t wireType)
    {
        return ParseNumericField<SInt64Traits>(input, fieldInfo, wireType, _LVClusterHandle + fieldInfo.clusterOffset);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseFixed32Field(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo, uint32_t wireType)
    {
        return ParseNumericField<Fixed32Traits>(input, fieldInfo, wireType, _LVClusterHandle + fieldInfo.clusterOffset);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseFixed64Field(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo, uint32_t wireType)
    {
        return ParseNumericField<Fixed64Traits>(input, fieldInfo, wireType, _LVClusterHandle + fieldInfo.clusterOffset);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseSFixed32Field(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo, uint32_t wireType)
    {
        return ParseNumericField<SFixed32Traits>(input, fieldInfo, wireType, _LVClusterHandle + fieldInfo.clusterOffset);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseSFixed64Field(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo, uint32_t wireType)
    {
        return ParseNumericField<SFixed64Traits>(input, fieldInfo, wireType, _LVClusterHandle + fieldInfo.clusterOffset);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseEnumField(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo, uint32_t wireType)
    {
        std::shared_ptr<EnumMetadata> enumMetadata = fieldInfo._owner->FindEnumMetadata(fieldInfo.embeddedMessageName);
        auto lv_ptr = _LVClusterHandle + fieldInfo.clusterOffset;

        if (fieldInfo.isRepeated)
        {
            if (wireType == WIRETYPE_LENGTH_DELIMITED) // packed
            {
                uint32_t length; if (!input->ReadVarint32(&length)) return false;
                auto limit = input->PushLimit(static_cast<int>(length));
                std::vector<int32_t> vals;
                while (input->BytesUntilLimit() > 0)
                {
                    uint32_t raw; if (!input->ReadVarint32(&raw)) { input->PopLimit(limit); return false; }
                    vals.push_back(enumMetadata->GetLVEnumValueFromProtoValue(static_cast<int32_t>(raw)));
                }
                input->PopLimit(limit);
                auto cnt = vals.size();
                if (cnt > 0)
                {
                    NumericArrayResize(GetTypeCodeForSize(sizeof(int32_t)), 1, reinterpret_cast<void*>(lv_ptr), cnt);
                    auto arr = *(LV1DArrayHandle*)lv_ptr;
                    (*arr)->cnt = static_cast<int32_t>(cnt);
                    memcpy((*arr)->bytes<int32_t>(), vals.data(), cnt * sizeof(int32_t));
                }
            }
            else // unpacked: single element per tag occurrence
            {
                uint32_t raw; if (!input->ReadVarint32(&raw)) return false;
                int32_t mapped = enumMetadata->GetLVEnumValueFromProtoValue(static_cast<int32_t>(raw));
                auto arr = *(LV1DArrayHandle*)lv_ptr;
                int32_t cnt = (arr != nullptr) ? (*arr)->cnt : 0;
                NumericArrayResize(GetTypeCodeForSize(sizeof(int32_t)), 1, reinterpret_cast<void*>(lv_ptr), static_cast<size_t>(cnt) + 1);
                arr = *(LV1DArrayHandle*)lv_ptr;
                (*arr)->cnt = cnt + 1;
                (*arr)->bytes<int32_t>()[cnt] = mapped;
            }
        }
        else
        {
            uint32_t raw; if (!input->ReadVarint32(&raw)) return false;
            *reinterpret_cast<int32_t*>(lv_ptr) = enumMetadata->GetLVEnumValueFromProtoValue(static_cast<int32_t>(raw));
        }
        return true;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseStringField(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo)
    {
        uint32_t length;
        if (!input->ReadVarint32(&length)) return false;
        std::string str;
        if (!input->ReadString(&str, static_cast<int>(length))) return false;

        if (!VerifyUtf8String(str, fieldInfo.fieldName.c_str())) {
            throw std::runtime_error("String contains invalid UTF-8 data.");
        }

        if (fieldInfo.isRepeated)
        {
            auto it = _repeatedStringValuesMap.find(fieldInfo.fieldName);
            if (it == _repeatedStringValuesMap.end())
            {
                auto m_val = std::make_shared<RepeatedStringValue>(fieldInfo);
                it = _repeatedStringValuesMap.emplace(fieldInfo.fieldName, m_val).first;
            }
            *it->second->_repeatedString.Add() = str;
        }
        else
        {
            auto lv_ptr = _LVClusterHandle + fieldInfo.clusterOffset;
            SetLVString((LStrHandle*)lv_ptr, str);
        }
        return true;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseBytesField(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo)
    {
        uint32_t length;
        if (!input->ReadVarint32(&length)) return false;
        std::string bytes;
        if (!input->ReadString(&bytes, static_cast<int>(length))) return false;

        if (!FeatureConfig::getInstance().AreUtf8StringsEnabled())
        {
            // Treat bytes as string
            if (fieldInfo.isRepeated)
            {
                auto it = _repeatedStringValuesMap.find(fieldInfo.fieldName);
                if (it == _repeatedStringValuesMap.end())
                {
                    auto m_val = std::make_shared<RepeatedStringValue>(fieldInfo);
                    it = _repeatedStringValuesMap.emplace(fieldInfo.fieldName, m_val).first;
                }
                *it->second->_repeatedString.Add() = bytes;
            }
            else
            {
                auto lv_ptr = _LVClusterHandle + fieldInfo.clusterOffset;
                SetLVString((LStrHandle*)lv_ptr, bytes);
            }
        }
        else
        {
            if (fieldInfo.isRepeated)
            {
                auto it = _repeatedBytesValuesMap.find(fieldInfo.fieldName);
                if (it == _repeatedBytesValuesMap.end())
                {
                    auto m_val = std::make_shared<RepeatedBytesValue>(fieldInfo);
                    it = _repeatedBytesValuesMap.emplace(fieldInfo.fieldName, m_val).first;
                }
                *it->second->_repeatedBytes.Add() = bytes;
            }
            else
            {
                auto lv_ptr = _LVClusterHandle + fieldInfo.clusterOffset;
                SetLVBytes((LStrHandle*)lv_ptr, bytes);
            }
        }
        return true;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseMessageField(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo)
    {
        switch (fieldInfo.wellKnownType)
        {
        case wellknown::Types::Double2DArray:
            return ParseDouble2DArrayField(input, fieldNumber, fieldInfo);
        case wellknown::Types::String2DArray:
            return ParseString2DArrayField(input, fieldNumber, fieldInfo);
        }

        uint32_t length;
        if (!input->ReadVarint32(&length)) return false;
        auto limit = input->PushLimit(static_cast<int>(length));

        auto metadata = fieldInfo._owner->FindMetadata(fieldInfo.embeddedMessageName);
        if (fieldInfo.isRepeated)
        {
            auto it = _repeatedMessageValuesMap.find(fieldInfo.fieldName);
            if (it == _repeatedMessageValuesMap.end())
            {
                auto m_val = std::make_shared<RepeatedMessageValue>(fieldInfo, google::protobuf::RepeatedPtrField<google::protobuf::Message>());
                it = _repeatedMessageValuesMap.emplace(fieldInfo.fieldName, m_val).first;
                it->second->_buffer.Reserve(128);
            }

            auto& repeatedValue = *it->second;
            auto elementIndex = repeatedValue._numElements;
            auto clusterSize = metadata->clusterSize;
            auto numElements = static_cast<uint64_t>(repeatedValue._buffer.Capacity()) / clusterSize;
            if (numElements == 0)
            {
                numElements = 128;
                repeatedValue._buffer.Reserve(static_cast<int>(numElements));
            }

            if (elementIndex >= numElements - 1)
            {
                numElements = std::max((uint64_t)128, numElements * 2);
                repeatedValue._buffer.Reserve(static_cast<int>(numElements));
            }

            auto nestedCluster = const_cast<int8_t*>(reinterpret_cast<const int8_t*>(repeatedValue._buffer.data()));
            nestedCluster += elementIndex * clusterSize;
            LVMessageEfficient nestedMessage(metadata, nestedCluster);
            if (!nestedMessage.ParseFromCodedStream(input))
            {
                input->PopLimit(limit);
                return false;
            }
            repeatedValue._numElements = elementIndex + 1;
        }
        else
        {
            auto nestedClusterPtr = _LVClusterHandle + fieldInfo.clusterOffset;
            LVMessageEfficient nestedMessage(metadata, nestedClusterPtr);
            if (!nestedMessage.ParseFromCodedStream(input))
            {
                input->PopLimit(limit);
                return false;
            }
        }

        input->PopLimit(limit);
        return true;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::Parse2DArrayField(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo, wellknown::I2DArray& array)
    {
        uint32_t length;
        if (!input->ReadVarint32(&length)) return false;
        auto limit = input->PushLimit(static_cast<int>(length));

        auto metadata = fieldInfo._owner->FindMetadata(fieldInfo.embeddedMessageName);
        auto nestedMessage = std::make_shared<LVMessage>(metadata);
        if (!nestedMessage->ParseFromCodedStream(input))
        {
            input->PopLimit(limit);
            return false;
        }
        input->PopLimit(limit);

        auto nestedClusterPtr = _LVClusterHandle + fieldInfo.clusterOffset;
        auto nestedMessageValue = std::make_shared<LVNestedMessageMessageValue>(fieldNumber, nestedMessage);
        array.CopyFromMessageToCluster(fieldInfo, nestedMessageValue, nestedClusterPtr);
        return true;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseDouble2DArrayField(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo)
    {
        return Parse2DArrayField(input, fieldNumber, fieldInfo, wellknown::Double2DArray::GetInstance());
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseString2DArrayField(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo)
    {
        return Parse2DArrayField(input, fieldNumber, fieldInfo, wellknown::String2DArray::GetInstance());
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVMessageEfficient::PostInteralParseAction()
    {
        CopyOneofIndicesToCluster(_LVClusterHandle);

        for (auto nestedMessage : _repeatedMessageValuesMap)
        {
            auto& fieldInfo = nestedMessage.second.get()->_fieldInfo;
            auto& buffer = nestedMessage.second.get()->_buffer;
            auto numClusters = nestedMessage.second.get()->_numElements;

            auto metadata = fieldInfo._owner->FindMetadata(fieldInfo.embeddedMessageName);
            auto lv_ptr = _LVClusterHandle + fieldInfo.clusterOffset;
            auto clusterSize = metadata->clusterSize;
            auto alignment = metadata->alignmentRequirement;


            // Allocate an array with the correct size and alignment for the cluster.
            auto byteSize = numClusters * clusterSize;
            auto alignedElementSize = byteSize / alignment;
            if (byteSize % alignment != 0)
            {
                alignedElementSize++;
            }
            NumericArrayResize(GetTypeCodeForSize(alignment), 1, reinterpret_cast<void*>(lv_ptr), alignedElementSize);
            auto arrayHandle = *(LV1DArrayHandle*)lv_ptr;
            (*arrayHandle)->cnt = numClusters;

            auto vectorDataPtr = buffer.data();
            auto lvArrayDataPtr = (*arrayHandle)->bytes(0, alignment);
            memcpy(lvArrayDataPtr, vectorDataPtr, byteSize);
        }

        for (auto repeatedStringValue : _repeatedStringValuesMap)
        {
            auto& fieldInfo = repeatedStringValue.second.get()->_fieldInfo;
            auto& repeatedString = repeatedStringValue.second.get()->_repeatedString;
            auto lv_ptr = _LVClusterHandle + fieldInfo.clusterOffset;

            NumericArrayResize(GetTypeCodeForSize(sizeof(char*)), 1, reinterpret_cast<void*>(lv_ptr), repeatedString.size());
            auto arrayHandle = *(LV1DArrayHandle*)lv_ptr;
            (*arrayHandle)->cnt = repeatedString.size();

            // Copy the repeated string values into the LabVIEW array
            auto lvStringPtr = (*arrayHandle)->bytes<LStrHandle>();
            for (auto& str : repeatedString)
            {
                *lvStringPtr = nullptr;
                SetLVString(lvStringPtr, str);
                lvStringPtr++;
            }
        }

        if (FeatureConfig::getInstance().AreUtf8StringsEnabled()) {
            for (auto repeatedBytesValue : _repeatedBytesValuesMap)
            {
                auto& fieldInfo = repeatedBytesValue.second.get()->_fieldInfo;
                auto& repeatedBytes = repeatedBytesValue.second.get()->_repeatedBytes;
                auto lv_ptr = _LVClusterHandle + fieldInfo.clusterOffset;

                NumericArrayResize(GetTypeCodeForSize(sizeof(char*)), 1, reinterpret_cast<void*>(lv_ptr), repeatedBytes.size());
                auto arrayHandle = *(LV1DArrayHandle*)lv_ptr;
                (*arrayHandle)->cnt = repeatedBytes.size();

                // Copy the repeated bytes values into the LabVIEW array
                auto lvBytesPtr = (*arrayHandle)->bytes<LStrHandle>();
                for (auto& bytes : repeatedBytes)
                {
                    *lvBytesPtr = nullptr;
                    SetLVBytes(lvBytesPtr, bytes);
                    lvBytesPtr++;
                }
            }
        }
    }
}

