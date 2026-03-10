//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_server.h>
#include <lv_message.h>
#include <sstream>
#include <feature_toggles.h>
#include <string_utils.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/unknown_field_set.h>
#include <google/protobuf/wire_format_lite.h>

namespace {
    // Wire type constants
    enum WireType {
        WIRETYPE_VARINT = 0,
        WIRETYPE_FIXED64 = 1,
        WIRETYPE_LENGTH_DELIMITED = 2,
        WIRETYPE_START_GROUP = 3,
        WIRETYPE_END_GROUP = 4,
        WIRETYPE_FIXED32 = 5
    };
    
    // Tag parsing helpers (public API alternative to WireFormatLite)
    inline uint32_t GetTagFieldNumber(uint32_t tag) {
        return tag >> 3;
    }
    
    inline uint32_t GetTagWireType(uint32_t tag) {
        return tag & 7;
    }
    
    // Read a field and optionally store it in an UnknownFieldSet.
    // Pass nullptr for unknownFields to just skip (used for nested groups).
    bool HandleField(google::protobuf::io::CodedInputStream* input, uint32_t tag,
                     google::protobuf::UnknownFieldSet* unknownFields) {
        uint32_t field_number = GetTagFieldNumber(tag);
        uint32_t wire_type = GetTagWireType(tag);

        switch (wire_type) {
            case WIRETYPE_VARINT: {
                uint64_t value;
                if (!input->ReadVarint64(&value)) return false;
                if (unknownFields) unknownFields->AddVarint(field_number, value);
                return true;
            }
            case WIRETYPE_FIXED64: {
                uint64_t value;
                if (!input->ReadLittleEndian64(&value)) return false;
                if (unknownFields) unknownFields->AddFixed64(field_number, value);
                return true;
            }
            case WIRETYPE_LENGTH_DELIMITED: {
                uint32_t length;
                if (!input->ReadVarint32(&length)) return false;
                std::string value;
                if (!input->ReadString(&value, length)) return false;
                if (unknownFields) unknownFields->AddLengthDelimited(field_number, value);
                return true;
            }
            case WIRETYPE_START_GROUP: {
                // Groups are deprecated; skip recursively without storing
                uint32_t end_tag = (field_number << 3) | WIRETYPE_END_GROUP;
                while (true) {
                    uint32_t inner_tag = input->ReadTag();
                    if (inner_tag == 0) return false;
                    if (inner_tag == end_tag) return true;
                    if (!HandleField(input, inner_tag, nullptr)) return false;
                }
            }
            case WIRETYPE_END_GROUP:
                return false;
            case WIRETYPE_FIXED32: {
                uint32_t value;
                if (!input->ReadLittleEndian32(&value)) return false;
                if (unknownFields) unknownFields->AddFixed32(field_number, value);
                return true;
            }
            default:
                return false;
        }
    }

    inline bool SkipField(google::protobuf::io::CodedInputStream* input, uint32_t tag) {
        return HandleField(input, tag, nullptr);
    }

    inline int32_t ZigZagDecode32(uint32_t n) {
        return google::protobuf::internal::WireFormatLite::ZigZagDecode32(n);
    }
    inline int64_t ZigZagDecode64(uint64_t n) {
        return google::protobuf::internal::WireFormatLite::ZigZagDecode64(n);
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
        
        if (buf.empty()) {
            return true;  // Empty message is valid
        }
        
        // Use public CodedInputStream API
        return ParseFromString(buf);
    }
    
    //---------------------------------------------------------------------
    // Parse from string using only public protobuf APIs
    //---------------------------------------------------------------------
    bool LVMessage::ParseFromString(const std::string& data)
    {
        using namespace google::protobuf::io;
        
        ArrayInputStream ais(data.data(), static_cast<int>(data.size()));
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
            uint32_t field_number = GetTagFieldNumber(tag);
            
            if (_metadata == nullptr)
            {
                // No schema - store everything as unknown for UnpackedFields use
                if (!HandleField(input, tag, &_unknownFields)) {
                    return false;
                }
            }
            else
            {
                auto fieldIt = _metadata->_mappedElements.find(field_number);
                if (fieldIt != _metadata->_mappedElements.end())
                {
                    auto& fieldInfo = (*fieldIt).second;

                    if (fieldInfo->isInOneof)
                    {
                        _oneofContainerToSelectedIndexMap.insert({ fieldInfo->oneofContainerName, fieldInfo->protobufIndex });
                    }

                    // Parse field based on type using CodedInputStream
                    if (!ParseFieldFromCodedStream(input, tag, field_number, *fieldInfo)) {
                        return false;
                    }
                }
                else
                {
                    // Unknown field - store for potential later inspection
                    if (!HandleField(input, tag, &_unknownFields)) {
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
        uint32_t field_number,
        const MessageElementMetadata& fieldInfo)
    {
        switch (fieldInfo.type)
        {
        case LVMessageMetadataType::Int32Value:
            return ParseInt32Field(input, field_number, fieldInfo);
        case LVMessageMetadataType::Int64Value:
            return ParseInt64Field(input, field_number, fieldInfo);
        case LVMessageMetadataType::UInt32Value:
            return ParseUInt32Field(input, field_number, fieldInfo);
        case LVMessageMetadataType::UInt64Value:
            return ParseUInt64Field(input, field_number, fieldInfo);
        case LVMessageMetadataType::SInt32Value:
            return ParseSInt32Field(input, field_number, fieldInfo);
        case LVMessageMetadataType::SInt64Value:
            return ParseSInt64Field(input, field_number, fieldInfo);
        case LVMessageMetadataType::Fixed32Value:
            return ParseFixed32Field(input, field_number, fieldInfo);
        case LVMessageMetadataType::Fixed64Value:
            return ParseFixed64Field(input, field_number, fieldInfo);
        case LVMessageMetadataType::SFixed32Value:
            return ParseSFixed32Field(input, field_number, fieldInfo);
        case LVMessageMetadataType::SFixed64Value:
            return ParseSFixed64Field(input, field_number, fieldInfo);
        case LVMessageMetadataType::FloatValue:
            return ParseFloatField(input, field_number, fieldInfo);
        case LVMessageMetadataType::DoubleValue:
            return ParseDoubleField(input, field_number, fieldInfo);
        case LVMessageMetadataType::BoolValue:
            return ParseBoolField(input, field_number, fieldInfo);
        case LVMessageMetadataType::EnumValue:
            return ParseEnumField(input, field_number, fieldInfo);
        case LVMessageMetadataType::StringValue:
            return ParseStringField(input, field_number, fieldInfo);
        case LVMessageMetadataType::BytesValue:
            return ParseBytesField(input, field_number, fieldInfo);
        case LVMessageMetadataType::MessageValue:
            return ParseMessageField(input, field_number, fieldInfo);
        default:
            return false;
        }
    }
    
    //---------------------------------------------------------------------
    // Helper methods to parse specific field types from CodedInputStream
    //---------------------------------------------------------------------
    bool LVMessage::ParseInt32Field(google::protobuf::io::CodedInputStream* input, 
                                     uint32_t field_number, 
                                     const MessageElementMetadata& fieldInfo)
    {
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedMessageValue<int>>(field_number);
            
            // Check if it's packed
            uint32_t length;
            if (input->ReadVarint32(&length))
            {
                auto limit = input->PushLimit(length);
                while (input->BytesUntilLimit() > 0)
                {
                    uint32_t value;
                    if (!input->ReadVarint32(&value)) return false;
                    v->_value.Add(static_cast<int32_t>(value));
                }
                input->PopLimit(limit);
            }
            _values.emplace(field_number, v);
        }
        else
        {
            uint32_t value;
            if (!input->ReadVarint32(&value)) return false;
            auto v = std::make_shared<LVVariableMessageValue<int>>(field_number, static_cast<int32_t>(value));
            _values.emplace(field_number, v);
        }
        return true;
    }
    
    bool LVMessage::ParseInt64Field(google::protobuf::io::CodedInputStream* input,
                                     uint32_t field_number,
                                     const MessageElementMetadata& fieldInfo)
    {
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedMessageValue<int64_t>>(field_number);
            uint32_t length;
            if (input->ReadVarint32(&length))
            {
                auto limit = input->PushLimit(length);
                while (input->BytesUntilLimit() > 0)
                {
                    uint64_t value;
                    if (!input->ReadVarint64(&value)) return false;
                    v->_value.Add(static_cast<int64_t>(value));
                }
                input->PopLimit(limit);
            }
            _values.emplace(field_number, v);
        }
        else
        {
            uint64_t value;
            if (!input->ReadVarint64(&value)) return false;
            auto v = std::make_shared<LVVariableMessageValue<int64_t>>(field_number, static_cast<int64_t>(value));
            _values.emplace(field_number, v);
        }
        return true;
    }
    
    bool LVMessage::ParseUInt32Field(google::protobuf::io::CodedInputStream* input,
                                      uint32_t field_number,
                                      const MessageElementMetadata& fieldInfo)
    {
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedMessageValue<uint32_t>>(field_number);
            uint32_t length;
            if (input->ReadVarint32(&length))
            {
                auto limit = input->PushLimit(length);
                while (input->BytesUntilLimit() > 0)
                {
                    uint32_t value;
                    if (!input->ReadVarint32(&value)) return false;
                    v->_value.Add(value);
                }
                input->PopLimit(limit);
            }
            _values.emplace(field_number, v);
        }
        else
        {
            uint32_t value;
            if (!input->ReadVarint32(&value)) return false;
            auto v = std::make_shared<LVVariableMessageValue<uint32_t>>(field_number, value);
            _values.emplace(field_number, v);
        }
        return true;
    }
    
    bool LVMessage::ParseUInt64Field(google::protobuf::io::CodedInputStream* input,
                                      uint32_t field_number,
                                      const MessageElementMetadata& fieldInfo)
    {
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedMessageValue<uint64_t>>(field_number);
            uint32_t length;
            if (input->ReadVarint32(&length))
            {
                auto limit = input->PushLimit(length);
                while (input->BytesUntilLimit() > 0)
                {
                    uint64_t value;
                    if (!input->ReadVarint64(&value)) return false;
                    v->_value.Add(value);
                }
                input->PopLimit(limit);
            }
            _values.emplace(field_number, v);
        }
        else
        {
            uint64_t value;
            if (!input->ReadVarint64(&value)) return false;
            auto v = std::make_shared<LVVariableMessageValue<uint64_t>>(field_number, value);
            _values.emplace(field_number, v);
        }
        return true;
    }
    
    bool LVMessage::ParseFloatField(google::protobuf::io::CodedInputStream* input,
                                     uint32_t field_number,
                                     const MessageElementMetadata& fieldInfo)
    {
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedMessageValue<float>>(field_number);
            uint32_t length;
            if (input->ReadVarint32(&length))
            {
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
            }
            _values.emplace(field_number, v);
        }
        else
        {
            uint32_t value;
            if (!input->ReadLittleEndian32(&value)) return false;
            float f;
            memcpy(&f, &value, sizeof(float));
            auto v = std::make_shared<LVVariableMessageValue<float>>(field_number, f);
            _values.emplace(field_number, v);
        }
        return true;
    }
    
    bool LVMessage::ParseDoubleField(google::protobuf::io::CodedInputStream* input,
                                      uint32_t field_number,
                                      const MessageElementMetadata& fieldInfo)
    {
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedMessageValue<double>>(field_number);
            uint32_t length;
            if (input->ReadVarint32(&length))
            {
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
            }
            _values.emplace(field_number, v);
        }
        else
        {
            uint64_t value;
            if (!input->ReadLittleEndian64(&value)) return false;
            double d;
            memcpy(&d, &value, sizeof(double));
            auto v = std::make_shared<LVVariableMessageValue<double>>(field_number, d);
            _values.emplace(field_number, v);
        }
        return true;
    }
    
    bool LVMessage::ParseBoolField(google::protobuf::io::CodedInputStream* input,
                                    uint32_t field_number,
                                    const MessageElementMetadata& fieldInfo)
    {
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedMessageValue<bool>>(field_number);
            uint32_t length;
            if (input->ReadVarint32(&length))
            {
                auto limit = input->PushLimit(length);
                while (input->BytesUntilLimit() > 0)
                {
                    uint64_t value;
                    if (!input->ReadVarint64(&value)) return false;
                    v->_value.Add(value != 0);
                }
                input->PopLimit(limit);
            }
            _values.emplace(field_number, v);
        }
        else
        {
            uint64_t value;
            if (!input->ReadVarint64(&value)) return false;
            auto v = std::make_shared<LVVariableMessageValue<bool>>(field_number, value != 0);
            _values.emplace(field_number, v);
        }
        return true;
    }
    
    bool LVMessage::ParseStringField(google::protobuf::io::CodedInputStream* input,
                                      uint32_t field_number,
                                      const MessageElementMetadata& fieldInfo)
    {
        uint32_t length;
        if (!input->ReadVarint32(&length)) return false;
        std::string value;
        if (!input->ReadString(&value, length)) return false;
        
        if (fieldInfo.isRepeated)
        {
            // For repeated strings, use RepeatedPtrField
            auto it = _values.find(field_number);
            if (it == _values.end())
            {
                auto v = std::make_shared<LVRepeatedStringMessageValue>(field_number);
                *v->_value.Add() = value;
                _values.emplace(field_number, v);
            }
            else
            {
                auto v = std::static_pointer_cast<LVRepeatedStringMessageValue>(it->second);
                *v->_value.Add() = value;
            }
        }
        else
        {
            auto v = std::make_shared<LVStringMessageValue>(field_number, value);
            _values.emplace(field_number, v);
        }
        return true;
    }
    
    bool LVMessage::ParseBytesField(google::protobuf::io::CodedInputStream* input,
                                     uint32_t field_number,
                                     const MessageElementMetadata& fieldInfo)
    {
        uint32_t length;
        if (!input->ReadVarint32(&length)) return false;
        std::string value;
        if (!input->ReadString(&value, length)) return false;
        
        if (fieldInfo.isRepeated)
        {
            auto it = _values.find(field_number);
            if (it == _values.end())
            {
                auto v = std::make_shared<LVRepeatedBytesMessageValue>(field_number);
                *v->_value.Add() = value;
                _values.emplace(field_number, v);
            }
            else
            {
                auto v = std::static_pointer_cast<LVRepeatedBytesMessageValue>(it->second);
                *v->_value.Add() = value;
            }
        }
        else
        {
            auto v = std::make_shared<LVBytesMessageValue>(field_number, value);
            _values.emplace(field_number, v);
        }
        return true;
    }
    
    bool LVMessage::ParseMessageField(google::protobuf::io::CodedInputStream* input,
                                       uint32_t field_number,
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
        
        auto v = std::make_shared<LVNestedMessageMessageValue>(field_number, nestedMessage);
        _values.emplace(field_number, v);
        return true;
    }
    
    bool LVMessage::ParseEnumField(google::protobuf::io::CodedInputStream* input,
                                    uint32_t field_number,
                                    const MessageElementMetadata& fieldInfo)
    {
        // Enums are encoded as int32
        return ParseInt32Field(input, field_number, fieldInfo);
    }
    
    bool LVMessage::ParseSInt32Field(google::protobuf::io::CodedInputStream* input,
                                      uint32_t field_number,
                                      const MessageElementMetadata& fieldInfo)
    {
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedSInt32MessageValue>(field_number);
            uint32_t length;
            if (input->ReadVarint32(&length))
            {
                auto limit = input->PushLimit(length);
                while (input->BytesUntilLimit() > 0)
                {
                    uint32_t value;
                    if (!input->ReadVarint32(&value)) return false;
                    v->_value.Add(ZigZagDecode32(value));
                }
                input->PopLimit(limit);
            }
            _values.emplace(field_number, v);
        }
        else
        {
            uint32_t value;
            if (!input->ReadVarint32(&value)) return false;
            auto v = std::make_shared<LVVariableMessageValue<int32_t>>(field_number, ZigZagDecode32(value));
            _values.emplace(field_number, v);
        }
        return true;
    }
    
    bool LVMessage::ParseSInt64Field(google::protobuf::io::CodedInputStream* input,
                                      uint32_t field_number,
                                      const MessageElementMetadata& fieldInfo)
    {
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedSInt64MessageValue>(field_number);
            uint32_t length;
            if (input->ReadVarint32(&length))
            {
                auto limit = input->PushLimit(length);
                while (input->BytesUntilLimit() > 0)
                {
                    uint64_t value;
                    if (!input->ReadVarint64(&value)) return false;
                    v->_value.Add(ZigZagDecode64(value));
                }
                input->PopLimit(limit);
            }
            _values.emplace(field_number, v);
        }
        else
        {
            uint64_t value;
            if (!input->ReadVarint64(&value)) return false;
            auto v = std::make_shared<LVVariableMessageValue<int64_t>>(field_number, ZigZagDecode64(value));
            _values.emplace(field_number, v);
        }
        return true;
    }
    
    bool LVMessage::ParseFixed32Field(google::protobuf::io::CodedInputStream* input,
                                       uint32_t field_number,
                                       const MessageElementMetadata& fieldInfo)
    {
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedFixed32MessageValue>(field_number);
            uint32_t length;
            if (input->ReadVarint32(&length))
            {
                auto limit = input->PushLimit(length);
                while (input->BytesUntilLimit() > 0)
                {
                    uint32_t value;
                    if (!input->ReadLittleEndian32(&value)) return false;
                    v->_value.Add(value);
                }
                input->PopLimit(limit);
            }
            _values.emplace(field_number, v);
        }
        else
        {
            uint32_t value;
            if (!input->ReadLittleEndian32(&value)) return false;
            auto v = std::make_shared<LVVariableMessageValue<uint32_t>>(field_number, value);
            _values.emplace(field_number, v);
        }
        return true;
    }
    
    bool LVMessage::ParseFixed64Field(google::protobuf::io::CodedInputStream* input,
                                       uint32_t field_number,
                                       const MessageElementMetadata& fieldInfo)
    {
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedFixed64MessageValue>(field_number);
            uint32_t length;
            if (input->ReadVarint32(&length))
            {
                auto limit = input->PushLimit(length);
                while (input->BytesUntilLimit() > 0)
                {
                    uint64_t value;
                    if (!input->ReadLittleEndian64(&value)) return false;
                    v->_value.Add(value);
                }
                input->PopLimit(limit);
            }
            _values.emplace(field_number, v);
        }
        else
        {
            uint64_t value;
            if (!input->ReadLittleEndian64(&value)) return false;
            auto v = std::make_shared<LVVariableMessageValue<uint64_t>>(field_number, value);
            _values.emplace(field_number, v);
        }
        return true;
    }
    
    bool LVMessage::ParseSFixed32Field(google::protobuf::io::CodedInputStream* input,
                                        uint32_t field_number,
                                        const MessageElementMetadata& fieldInfo)
    {
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedSFixed32MessageValue>(field_number);
            uint32_t length;
            if (input->ReadVarint32(&length))
            {
                auto limit = input->PushLimit(length);
                while (input->BytesUntilLimit() > 0)
                {
                    uint32_t value;
                    if (!input->ReadLittleEndian32(&value)) return false;
                    v->_value.Add(static_cast<int32_t>(value));
                }
                input->PopLimit(limit);
            }
            _values.emplace(field_number, v);
        }
        else
        {
            uint32_t value;
            if (!input->ReadLittleEndian32(&value)) return false;
            auto v = std::make_shared<LVVariableMessageValue<int32_t>>(field_number, static_cast<int32_t>(value));
            _values.emplace(field_number, v);
        }
        return true;
    }
    
    bool LVMessage::ParseSFixed64Field(google::protobuf::io::CodedInputStream* input,
                                        uint32_t field_number,
                                        const MessageElementMetadata& fieldInfo)
    {
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedSFixed64MessageValue>(field_number);
            uint32_t length;
            if (input->ReadVarint32(&length))
            {
                auto limit = input->PushLimit(length);
                while (input->BytesUntilLimit() > 0)
                {
                    uint64_t value;
                    if (!input->ReadLittleEndian64(&value)) return false;
                    v->_value.Add(static_cast<int64_t>(value));
                }
                input->PopLimit(limit);
            }
            _values.emplace(field_number, v);
        }
        else
        {
            uint64_t value;
            if (!input->ReadLittleEndian64(&value)) return false;
            auto v = std::make_shared<LVVariableMessageValue<int64_t>>(field_number, static_cast<int64_t>(value));
            _values.emplace(field_number, v);
        }
        return true;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    std::unique_ptr<grpc::ByteBuffer> LVMessage::SerializeToByteBuffer() const
    {
        std::string buf;
        {
            google::protobuf::io::StringOutputStream sos(&buf);
            google::protobuf::io::CodedOutputStream cos(&sos);
            for (auto& e : _values)
                e.second->Serialize(&cos);
        }  // CodedOutputStream flushes on destruction

        if (buf.empty()) {
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
        return true;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVMessage::Clear()
    {
        _values.clear();
        _oneofContainerToSelectedIndexMap.clear();
        _unknownFields.Clear();
        _cachedByteSize = static_cast<size_t>(-1);
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
        if (_cachedByteSize != static_cast<size_t>(-1))
            return _cachedByteSize;

        size_t totalSize = 0;
        for (auto& e : _values)
        {
            totalSize += e.second->ByteSizeLong();
        }
        _cachedByteSize = totalSize;
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
