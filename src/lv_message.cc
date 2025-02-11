//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_server.h>
#include <lv_message.h>
#include <sstream>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
using namespace google::protobuf::internal;

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
    google::protobuf::UnknownFieldSet &LVMessage::UnknownFields()
    {
        return _unknownFields;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessage::ParseFromByteBuffer(const grpc::ByteBuffer &buffer)
    {
        Clear();

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
    //---------------------------------------------------------------------
    std::unique_ptr<grpc::ByteBuffer> LVMessage::SerializeToByteBuffer()
    {
        std::string buf;
        SerializeToString(&buf);
        grpc::Slice slice(buf);
        return std::unique_ptr<grpc::ByteBuffer>(new grpc::ByteBuffer(&slice, 1));
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    google::protobuf::Message *LVMessage::New(google::protobuf::Arena *arena) const
    {
        assert(false); // not expected to be called
        return nullptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVMessage::SetCachedSize(int size) const
    {
        _cached_size_.Set(size);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    int LVMessage::GetCachedSize(void) const
    {
        return _cached_size_.Get();
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVMessage::Clear()
    {
        _values.clear();
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::_InternalParse(const char *ptr, ParseContext *ctx)
    {
        assert(ptr != nullptr);
        while (!ctx->Done(&ptr))
        {
            google::protobuf::uint32 tag;
            ptr = ReadTag(ptr, &tag);
            auto index = (tag >> 3);
            if (_metadata == nullptr)
            {
                ptr = UnknownFieldParse(tag, &_unknownFields, ptr, ctx);
                assert(ptr != nullptr);
            }
            else
            {
                auto fieldIt = _metadata->_mappedElements.find(index);
                if (fieldIt != _metadata->_mappedElements.end())
                {
                    auto fieldInfo = (*fieldIt).second;
                    LVMessageMetadataType dataType = fieldInfo->type;
                    switch (dataType)
                    {
                    case LVMessageMetadataType::Int32Value:
                        ptr = ParseInt32(*fieldInfo, index, ptr, ctx);
                        break;
                    case LVMessageMetadataType::FloatValue:
                        ptr = ParseFloat(*fieldInfo, index, ptr, ctx);
                        break;
                    case LVMessageMetadataType::DoubleValue:
                        ptr = ParseDouble(*fieldInfo, index, ptr, ctx);
                        break;
                    case LVMessageMetadataType::BoolValue:
                        ptr = ParseBoolean(*fieldInfo, index, ptr, ctx);
                        break;
                    case LVMessageMetadataType::StringValue:
                        ptr = ParseString(tag, *fieldInfo, index, ptr, ctx);
                        break;
                    case LVMessageMetadataType::BytesValue:
                        ptr = ParseBytes(tag, *fieldInfo, index, ptr, ctx);
                        break;
                    case LVMessageMetadataType::MessageValue:
                        ptr = ParseNestedMessage(tag, *fieldInfo, index, ptr, ctx);
                        break;
                    case LVMessageMetadataType::Int64Value:
                        ptr = ParseInt64(*fieldInfo, index, ptr, ctx);
                        break;
                    case LVMessageMetadataType::UInt32Value:
                        ptr = ParseUInt32(*fieldInfo, index, ptr, ctx);
                        break;
                    case LVMessageMetadataType::UInt64Value:
                        ptr = ParseUInt64(*fieldInfo, index, ptr, ctx);
                        break;
                    case LVMessageMetadataType::EnumValue:
                        ptr = ParseEnum(*fieldInfo, index, ptr, ctx);
                        break;
                    case LVMessageMetadataType::SInt32Value:
                        ptr = ParseSInt32(*fieldInfo, index, ptr, ctx);
                        break;
                    case LVMessageMetadataType::SInt64Value:
                        ptr = ParseSInt64(*fieldInfo, index, ptr, ctx);
                        break;
                    case LVMessageMetadataType::Fixed32Value:
                        ptr = ParseFixed32(*fieldInfo, index, ptr, ctx);
                        break;
                    case LVMessageMetadataType::Fixed64Value:
                        ptr = ParseFixed64(*fieldInfo, index, ptr, ctx);
                        break;
                    case LVMessageMetadataType::SFixed32Value:
                        ptr = ParseSFixed32(*fieldInfo, index, ptr, ctx);
                        break;
                    case LVMessageMetadataType::SFixed64Value:
                        ptr = ParseSFixed64(*fieldInfo, index, ptr, ctx);
                        break;
                    }
                    assert(ptr != nullptr);
                }
                else
                {
                    if (tag == 0 || WireFormatLite::GetTagWireType(tag) == WireFormatLite::WIRETYPE_END_GROUP)
                    {
                        ctx->SetLastTag(tag);
                        return ptr;
                    }
                    ptr = UnknownFieldParse(tag, &_unknownFields, ptr, ctx);
                    assert(ptr != nullptr);
                }
            }
        }
        PostInteralParseAction();
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseBoolean(const MessageElementMetadata &fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
    {
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedMessageValue<bool>>(index);
            ptr = PackedBoolParser(&(v->_value), ptr, ctx);
            _values.emplace(index, v);
        }
        else
        {
            bool result;
            ptr = ReadBOOL(ptr, &result);
            auto v = std::make_shared<LVVariableMessageValue<bool>>(index, result);
            _values.emplace(index, v);
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseInt32(const MessageElementMetadata &fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
    {
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedMessageValue<int>>(index);
            ptr = PackedInt32Parser(&(v->_value), ptr, ctx);
            _values.emplace(index, v);
        }
        else
        {
            int32_t result;
            ptr = ReadINT32(ptr, &result);
            auto v = std::make_shared<LVVariableMessageValue<int>>(index, result);
            _values.emplace(index, v);
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseUInt32(const MessageElementMetadata &fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
    {
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedMessageValue<uint32_t>>(index);
            ptr = PackedUInt32Parser(&(v->_value), ptr, ctx);
            _values.emplace(index, v);
        }
        else
        {
            uint32_t result;
            ptr = ReadUINT32(ptr, &result);
            auto v = std::make_shared<LVVariableMessageValue<uint32_t>>(index, result);
            _values.emplace(index, v);
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseEnum(const MessageElementMetadata &fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
    {
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedEnumMessageValue>(index);
            ptr = PackedEnumParser(&(v->_value), ptr, ctx);
            _values.emplace(index, v);
        }
        else
        {
            int32_t result;
            ptr = ReadENUM(ptr, &result);
            auto v = std::make_shared<LVEnumMessageValue>(index, result);
            _values.emplace(index, v);
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseInt64(const MessageElementMetadata &fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
    {
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedMessageValue<int64_t>>(index);
            ptr = PackedInt64Parser(&(v->_value), ptr, ctx);
            _values.emplace(index, v);
        }
        else
        {
            int64_t result;
            ptr = ReadINT64(ptr, &result);
            auto v = std::make_shared<LVVariableMessageValue<int64_t>>(index, result);
            _values.emplace(index, v);
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseUInt64(const MessageElementMetadata &fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
    {
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedMessageValue<uint64_t>>(index);
            ptr = PackedUInt64Parser(&(v->_value), ptr, ctx);
            _values.emplace(index, v);
        }
        else
        {
            uint64_t result;
            ptr = ReadUINT64(ptr, &result);
            auto v = std::make_shared<LVVariableMessageValue<uint64_t>>(index, result);
            _values.emplace(index, v);
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseFloat(const MessageElementMetadata &fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
    {
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedMessageValue<float>>(index);
            ptr = PackedFloatParser(&(v->_value), ptr, ctx);
            _values.emplace(index, v);
        }
        else
        {
            float result;
            ptr = ReadFLOAT(ptr, &result);
            auto v = std::make_shared<LVVariableMessageValue<float>>(index, result);
            _values.emplace(index, v);
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseDouble(const MessageElementMetadata &fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
    {
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedMessageValue<double>>(index);
            ptr = PackedDoubleParser(&(v->_value), ptr, ctx);
            _values.emplace(index, v);
        }
        else
        {
            double result;
            ptr = ReadDOUBLE(ptr, &result);
            auto v = std::make_shared<LVVariableMessageValue<double>>(index, result);
            _values.emplace(index, v);
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseString(google::protobuf::uint32 tag, const MessageElementMetadata &fieldInfo, uint32_t index, const char *protobuf_ptr, ParseContext *ctx)
    {
        if (fieldInfo.isRepeated)
        {
            std::shared_ptr<LVRepeatedStringMessageValue> v;
            auto it = _values.find(index);
            if (it == _values.end())
            {
                v = std::make_shared<LVRepeatedStringMessageValue>(index);
                _values.emplace(index, v);
            }
            else
            {
                v = std::static_pointer_cast<LVRepeatedStringMessageValue>((*it).second);
            }
            protobuf_ptr -= 1;
            do
            {
                protobuf_ptr += 1;
                auto str = v->_value.Add();
                protobuf_ptr = InlineGreedyStringParser(str, protobuf_ptr, ctx);
                if (!ctx->DataAvailable(protobuf_ptr))
                {
                    break;
                }
            } while (ExpectTag(tag, protobuf_ptr));
        }
        else
        {
            auto str = std::string();
            protobuf_ptr = InlineGreedyStringParser(&str, protobuf_ptr, ctx);
            auto v = std::make_shared<LVStringMessageValue>(index, str);
            _values.emplace(index, v);
        }
        return protobuf_ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseBytes(google::protobuf::uint32 tag, const MessageElementMetadata &fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
    {
        return ParseString(tag, fieldInfo, index, ptr, ctx);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessage::ExpectTag(google::protobuf::uint32 tag, const char *ptr)
    {
        if (tag < 128)
        {
            return *ptr == tag;
        }
        else
        {
            char buf[2] = {static_cast<char>(tag | 0x80), static_cast<char>(tag >> 7)};
            return std::memcmp(ptr, buf, 2) == 0;
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseSInt32(const MessageElementMetadata &fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
    {
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedSInt32MessageValue>(index);
            ptr = PackedSInt32Parser(&(v->_value), ptr, ctx);
            _values.emplace(index, v);
        }
        else
        {
            int32_t result;
            ptr = ReadSINT32(ptr, &result);
            auto v = std::make_shared<LVSInt32MessageValue>(index, result);
            _values.emplace(index, v);
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseSInt64(const MessageElementMetadata &fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
    {
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedSInt64MessageValue>(index);
            ptr = PackedSInt64Parser(&(v->_value), ptr, ctx);
            _values.emplace(index, v);
        }
        else
        {
            int64_t result;
            ptr = ReadSINT64(ptr, &result);
            auto v = std::make_shared<LVSInt64MessageValue>(index, result);
            _values.emplace(index, v);
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseFixed32(const MessageElementMetadata &fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
    {
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedFixed32MessageValue>(index);
            ptr = PackedFixed32Parser(&(v->_value), ptr, ctx);
            _values.emplace(index, v);
        }
        else
        {
            uint32_t result;
            ptr = ReadFIXED32(ptr, &result);
            auto v = std::make_shared<LVFixed32MessageValue>(index, result);
            _values.emplace(index, v);
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseFixed64(const MessageElementMetadata &fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
    {
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedFixed64MessageValue>(index);
            ptr = PackedFixed64Parser(&(v->_value), ptr, ctx);
            _values.emplace(index, v);
        }
        else
        {
            uint64_t result;
            ptr = ReadFIXED64(ptr, &result);
            auto v = std::make_shared<LVFixed64MessageValue>(index, result);
            _values.emplace(index, v);
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseSFixed32(const MessageElementMetadata &fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
    {
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedSFixed32MessageValue>(index);
            ptr = PackedSFixed32Parser(&(v->_value), ptr, ctx);
            _values.emplace(index, v);
        }
        else
        {
            int32_t result;
            ptr = ReadSFIXED32(ptr, &result);
            auto v = std::make_shared<LVSFixed32MessageValue>(index, result);
            _values.emplace(index, v);
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseSFixed64(const MessageElementMetadata &fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
    {
        if (fieldInfo.isRepeated)
        {
            auto v = std::make_shared<LVRepeatedSFixed64MessageValue>(index);
            ptr = PackedSFixed64Parser(&(v->_value), ptr, ctx);
            _values.emplace(index, v);
        }
        else
        {
            int64_t result;
            ptr = ReadSFIXED64(ptr, &result);
            auto v = std::make_shared<LVSFixed64MessageValue>(index, result);
            _values.emplace(index, v);
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseNestedMessage(google::protobuf::uint32 tag, const MessageElementMetadata &fieldInfo, uint32_t index, const char *protobuf_ptr, ParseContext *ctx)
    {
        auto metadata = fieldInfo._owner->FindMetadata(fieldInfo.embeddedMessageName);
        if (fieldInfo.isRepeated)
        {
            protobuf_ptr -= 1;
            do
            {
                std::shared_ptr<LVRepeatedNestedMessageMessageValue> v;
                auto it = _values.find(index);
                if (it == _values.end())
                {
                    v = std::make_shared<LVRepeatedNestedMessageMessageValue>(index);
                    _values.emplace(index, v);
                }
                else
                {
                    v = std::static_pointer_cast<LVRepeatedNestedMessageMessageValue>((*it).second);
                }
                protobuf_ptr += 1;
                auto nestedMessage = std::make_shared<LVMessage>(metadata);
                protobuf_ptr = ctx->ParseMessage(nestedMessage.get(), protobuf_ptr);
                v->_value.push_back(nestedMessage);
                if (!ctx->DataAvailable(protobuf_ptr))
                {
                    break;
                }
            } while (ExpectTag(tag, protobuf_ptr));
        }
        else
        {
            auto nestedMessage = std::make_shared<LVMessage>(metadata);
            protobuf_ptr = ctx->ParseMessage(nestedMessage.get(), protobuf_ptr);
            auto v = std::make_shared<LVNestedMessageMessageValue>(index, nestedMessage);
            _values.emplace(index, v);
        }
        return protobuf_ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    google::protobuf::uint8 *LVMessage::_InternalSerialize(google::protobuf::uint8 *target, google::protobuf::io::EpsCopyOutputStream *stream) const
    {
        for (auto e : _values)
        {
            target = e.second->Serialize(target, stream);
        }
        return target;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    size_t LVMessage::ByteSizeLong() const
    {
        size_t totalSize = 0;

        for (auto e : _values)
        {
            totalSize += e.second->ByteSizeLong();
        }
        int cachedSize = ToCachedSize(totalSize);
        SetCachedSize(cachedSize);
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
    void LVMessage::SharedCtor()
    {
        assert(false); // not expected to be called
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVMessage::SharedDtor()
    {
        assert(false); // not expected to be called
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVMessage::ArenaDtor(void *object)
    {
        assert(false); // not expected to be called
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVMessage::RegisterArenaDtor(google::protobuf::Arena *)
    {
        assert(false); // not expected to be called
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVMessage::MergeFrom(const google::protobuf::Message &from)
    {
        assert(false); // not expected to be called
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVMessage::MergeFrom(const LVMessage &from)
    {
        assert(false); // not expected to be called
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVMessage::CopyFrom(const google::protobuf::Message &from)
    {
        assert(false); // not expected to be called
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVMessage::CopyFrom(const LVMessage &from)
    {
        assert(false); // not expected to be called
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVMessage::InternalSwap(LVMessage *other)
    {
        assert(false); // not expected to be called
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    google::protobuf::Metadata LVMessage::GetMetadata() const
    {
        assert(false); // not expected to be called
        return google::protobuf::Metadata();
    }
}
