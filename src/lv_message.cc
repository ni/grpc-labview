//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_server.h>
#include <lv_message.h>
#include <Windows.h>
#include <sstream>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
using namespace google::protobuf::internal;

namespace grpc_labview
{
    bool g_use_hardcoded_parse = true;

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    LVMessage::LVMessage(std::shared_ptr<MessageMetadata> metadata) : 
        _metadata(metadata), _use_hardcoded_parse(g_use_hardcoded_parse), _skipCopyOnFirstParse(false)
    {
    }

    LVMessage::LVMessage(std::shared_ptr<MessageMetadata> metadata, bool use_hardcoded_parse, bool skipCopyOnFirstParse) :
        _metadata(metadata), _use_hardcoded_parse(use_hardcoded_parse), _skipCopyOnFirstParse(skipCopyOnFirstParse)
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    LVMessage::~LVMessage()
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    google::protobuf::UnknownFieldSet& LVMessage::UnknownFields()
    {
        return _unknownFields;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessage::ParseFromByteBuffer(const grpc::ByteBuffer& buffer)
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
    google::protobuf::Message* LVMessage::New(google::protobuf::Arena* arena) const
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
                    if (_LVClusterHandle.get() == nullptr) {
                        _use_hardcoded_parse = false;
                    }
                    else {
                        _use_hardcoded_parse = true;
                    }
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
                    if (tag == 0 || WireFormatLite::GetTagWireType(tag) == WireFormatLite::WIRETYPE_END_GROUP) {
                        ctx->SetLastTag(tag);
                        return ptr;
                    }
                    ptr = UnknownFieldParse(tag, &_unknownFields, ptr, ctx);
                    assert(ptr != nullptr);
                }
            }
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseBoolean(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
    {
        if(_use_hardcoded_parse){
            grpc_labview::SinglePassMessageParser<bool, ReadBOOL, PackedBoolParser> parser(*this);
            auto v = std::make_shared<LVRepeatedMessageValue<bool>>(index);
            ptr = parser.ParseAndCopyMessage(fieldInfo, index, ptr, ctx, v);
        }
        else{       
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
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseInt32(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
    {    
        if (_use_hardcoded_parse)
        {
            grpc_labview::SinglePassMessageParser<int32_t, ReadINT32, PackedInt32Parser> parser(*this);
            auto v = std::make_shared<LVRepeatedMessageValue<int>>(index);
            ptr = parser.ParseAndCopyMessage(fieldInfo, index, ptr, ctx, v);
        }
        else {
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
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseUInt32(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
    {    
        if (_use_hardcoded_parse)
        {
            SinglePassMessageParser<uint32_t, ReadUINT32, PackedUInt32Parser> parser(*this);
            auto v = std::make_shared<LVRepeatedMessageValue<uint32_t>>(index);
            ptr = parser.ParseAndCopyMessage(fieldInfo, index, ptr, ctx, v);
        }
        else {
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
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseEnum(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
    {    
        if(_use_hardcoded_parse){
            grpc_labview::SinglePassMessageParser<int32_t, ReadENUM, PackedEnumParser> parser(*this);
            auto v = std::make_shared<LVRepeatedEnumMessageValue>(index);
            ptr = parser.ParseAndCopyMessage(fieldInfo, index, ptr, ctx, v);
        }
        else{
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
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseInt64(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
    {    
        if (_use_hardcoded_parse)
        {
            SinglePassMessageParser<int64_t, ReadINT64, PackedInt64Parser> parser(*this);
            auto v = std::make_shared<LVRepeatedMessageValue<int64_t>>(index);
            ptr = parser.ParseAndCopyMessage(fieldInfo, index, ptr, ctx, v);
        }
        else {
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
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseUInt64(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
    {   
        if (_use_hardcoded_parse)
        {
            grpc_labview::SinglePassMessageParser<uint64_t, ReadUINT64, PackedUInt64Parser> parser(*this);
            auto v = std::make_shared<LVRepeatedMessageValue<uint64_t>>(index);
            ptr = parser.ParseAndCopyMessage(fieldInfo, index, ptr, ctx, v);
        } 
        else {
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
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseFloat(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
    {    
        if(_use_hardcoded_parse){
            grpc_labview::SinglePassMessageParser<float, ReadFLOAT, PackedFloatParser> parser(*this);
            auto v = std::make_shared<LVRepeatedMessageValue<float>>(index);
            ptr = parser.ParseAndCopyMessage(fieldInfo, index, ptr, ctx, v);
        }
        else{
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
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseDouble(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
    {    
        if(_use_hardcoded_parse){
            grpc_labview::SinglePassMessageParser<double, ReadDOUBLE, PackedDoubleParser> parser(*this);
            auto v = std::make_shared<LVRepeatedMessageValue<double>>(index);
            ptr = parser.ParseAndCopyMessage(fieldInfo, index, ptr, ctx, v);
        }
        else{
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
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseString(google::protobuf::uint32 tag, const MessageElementMetadata& fieldInfo, uint32_t index, const char *protobuf_ptr, ParseContext *ctx)
    {    
        const char* lv_ptr = (this->getLVClusterHandleSharedPtr()) + fieldInfo.clusterOffset;

        if (_use_hardcoded_parse)
        {
            if (fieldInfo.isRepeated)
            {
                // Get the _repeatedMessageValues vector from the map
                auto _repeatedStringValuesIt = _repeatedStringValuesMap.find(fieldInfo.fieldName);
                if (_repeatedStringValuesIt == _repeatedStringValuesMap.end())
                {
                    _repeatedStringValuesIt = _repeatedStringValuesMap.emplace(fieldInfo.fieldName, google::protobuf::RepeatedField<std::string>()).first;
                }

                protobuf_ptr -= 1;
                do {
                    protobuf_ptr += 1;
                    auto str = _repeatedStringValuesIt->second.Add();
                    protobuf_ptr = InlineGreedyStringParser(str, protobuf_ptr, ctx);
                    if (!ctx->DataAvailable(protobuf_ptr))
                    {
                        break;
                    }
                } while (ExpectTag(tag, protobuf_ptr));

                auto arraySize = sizeof(void*) * _repeatedStringValuesIt->second.size();
                auto _lvProvidedArrayHandle = *(void**)lv_ptr; 
                *(void**)lv_ptr = DSNewHandle(arraySize);
                auto arrayHandle = *(LV1DArrayHandle*)lv_ptr;
                (*arrayHandle)->cnt = _repeatedStringValuesIt->second.size();

                // Copy the repeated string values into the LabVIEW array
                auto lvStringPtr = (*arrayHandle)->bytes<LStrHandle>();
                for (auto str:_repeatedStringValuesIt->second)
                {
                    *lvStringPtr = nullptr;
                    SetLVString(lvStringPtr, str);
                    lvStringPtr++;
                }
            }
            else {
                auto str = std::string();
                protobuf_ptr = InlineGreedyStringParser(&str, protobuf_ptr, ctx);
                SetLVString((LStrHandle*)lv_ptr, str);
            }
        }
        else {
            if (fieldInfo.isRepeated)
            {
                std::shared_ptr<LVRepeatedMessageValue<std::string>> v;
                auto it = _values.find(index);
                if (it == _values.end())
                {
                    v = std::make_shared<LVRepeatedMessageValue<std::string>>(index);
                    _values.emplace(index, v);
                }
                else
                {
                    v = std::static_pointer_cast<LVRepeatedMessageValue<std::string>>((*it).second);
                }
                protobuf_ptr -= 1;
                do {
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
        }
        return protobuf_ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseBytes(google::protobuf::uint32 tag, const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
    {
        return ParseString(tag, fieldInfo, index, ptr, ctx);    
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessage::ExpectTag(google::protobuf::uint32 tag, const char* ptr)
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
    const char* LVMessage::ParseSInt32(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, ParseContext* ctx)
    {
        if(_use_hardcoded_parse){
            grpc_labview::SinglePassMessageParser<int32_t, ReadSINT32, PackedSInt32Parser> parser(*this);
            auto v = std::make_shared<LVRepeatedSInt32MessageValue>(index);
            ptr = parser.ParseAndCopyMessage(fieldInfo, index, ptr, ctx, v);
        }
        else{
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
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char* LVMessage::ParseSInt64(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, ParseContext* ctx)
    {
        if(_use_hardcoded_parse){
            grpc_labview::SinglePassMessageParser<int64_t, ReadSINT64, PackedSInt64Parser> parser(*this);
            auto v = std::make_shared<LVRepeatedSInt64MessageValue>(index);
            ptr = parser.ParseAndCopyMessage(fieldInfo, index, ptr, ctx, v);
        }
        else{
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
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char* LVMessage::ParseFixed32(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, ParseContext* ctx)
    {
        if(_use_hardcoded_parse){
            grpc_labview::SinglePassMessageParser<uint32_t, ReadFIXED32, PackedFixed32Parser> parser(*this);
            auto v = std::make_shared<LVRepeatedFixed32MessageValue>(index);
            ptr = parser.ParseAndCopyMessage(fieldInfo, index, ptr, ctx, v);
        }
        else{
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
        }
        return ptr;
    }


    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char* LVMessage::ParseFixed64(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, ParseContext* ctx)
    {
        if(_use_hardcoded_parse){
            grpc_labview::SinglePassMessageParser<uint64_t, ReadFIXED64, PackedFixed64Parser> parser(*this);
            auto v = std::make_shared<LVRepeatedFixed64MessageValue>(index);
            ptr = parser.ParseAndCopyMessage(fieldInfo, index, ptr, ctx, v);
        }
        else{
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
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char* LVMessage::ParseSFixed32(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, ParseContext* ctx)
    {
        if(_use_hardcoded_parse){
            grpc_labview::SinglePassMessageParser<int32_t, ReadSFIXED32, PackedSFixed32Parser> parser(*this);
            auto v = std::make_shared<LVRepeatedSFixed32MessageValue>(index);
            ptr = parser.ParseAndCopyMessage(fieldInfo, index, ptr, ctx, v);
        }
        else{
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
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char* LVMessage::ParseSFixed64(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, ParseContext* ctx)
    {
        if(_use_hardcoded_parse){
            grpc_labview::SinglePassMessageParser<int64_t, ReadSFIXED64, PackedSFixed64Parser> parser(*this);
            auto v = std::make_shared<LVRepeatedSFixed64MessageValue>(index);
            ptr = parser.ParseAndCopyMessage(fieldInfo, index, ptr, ctx, v);
        }
        else{
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
        }
        return ptr;
    }


    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    struct LVCluster
    {
    }; // TODO: Do not check this in.


    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessage::ParseNestedMessage(google::protobuf::uint32 tag, const MessageElementMetadata& fieldInfo, uint32_t index, const char *protobuf_ptr, ParseContext *ctx)
    {    
        auto metadata = fieldInfo._owner->FindMetadata(fieldInfo.embeddedMessageName);
        if (_use_hardcoded_parse)
        {
            const char* lv_ptr = (this->getLVClusterHandleSharedPtr()) + fieldInfo.clusterOffset;
            LVMessage nestedMessage(metadata);

            if (fieldInfo.isRepeated) {
                // if the array is not big enough, resize it to 2x the size
                auto numElements = 128;
                auto elementIndex = 0;
                auto clusterSize = metadata->clusterSize;
                auto alignment = metadata->alignmentRequirement;
                auto arraySize = numElements * clusterSize;
                LV1DArrayHandle arrayHandle;
                char _fillData = '\0';

                // Get the _repeatedMessageValues vector from the map
                auto _repeatedMessageValuesIt = _repeatedMessageValuesMap.find(metadata->messageName);
                if (_repeatedMessageValuesIt == _repeatedMessageValuesMap.end())
                {
                    _repeatedMessageValuesIt = _repeatedMessageValuesMap.emplace(metadata->messageName, google::protobuf::RepeatedField<char>()).first;
                }

                // There are situations where the protobuf message is not complete, and we need to continue from the last index.
                // This function returns to _internalParse, and then gets back to this function.
                // If we are continuing from a previous parse, then we need to continue from the last index
                auto _continueFromIndex = _repeatedField_continueIndex.find(metadata->messageName);
                if (_continueFromIndex != _repeatedField_continueIndex.end()) {
                    elementIndex = _continueFromIndex->second;
                    _repeatedField_continueIndex.erase(_continueFromIndex);
                    // find next largest power of 2, as we assume that we previously resized it to a power of 2
                    auto _size = (int)ceil(log2(elementIndex));
                    numElements = ((1 << _size) > 128) ? (1 << _size) : 128;
                }
                else {
                    // occurs on the first time this function is called
                    _repeatedMessageValuesIt->second.Resize(arraySize, _fillData);
                }

                protobuf_ptr -= 1;
                do
                {
                    protobuf_ptr += 1;

                    // Resize the vector if we need more memory
                    if (elementIndex >= numElements - 1) {
                        numElements *= 2;
                        arraySize = numElements * clusterSize;
                        auto s = _repeatedMessageValuesIt->second.size();
                        _repeatedMessageValuesIt->second.Resize(arraySize, _fillData);
                    }

                    auto _vectorPtr = _repeatedMessageValuesIt->second.data();
                    _vectorPtr = _vectorPtr + (elementIndex * clusterSize);
                    nestedMessage.setLVClusterHandle(_vectorPtr);
                    protobuf_ptr = ctx->ParseMessage(&nestedMessage, protobuf_ptr);

                    elementIndex++;

                    if (!ctx->DataAvailable(protobuf_ptr)) {
                        break;
                    }
                } while (ExpectTag(tag, protobuf_ptr));

                // shrink the array to the correct size
                arraySize = elementIndex * clusterSize;
                auto old_arrayHandle = *(void**)lv_ptr;
                DSDisposeHandle(old_arrayHandle);
                *(void**)lv_ptr = DSNewHandle(arraySize);
                arrayHandle = *(LV1DArrayHandle*)lv_ptr;
                (*arrayHandle)->cnt = elementIndex;

                auto _vectorDataPtr = _repeatedMessageValuesIt->second.data();
                auto _lvArrayDataPtr = (*arrayHandle)->bytes(0, alignment);
                memcpy(_lvArrayDataPtr, _vectorDataPtr, arraySize);

                _repeatedField_continueIndex.emplace(metadata->messageName, elementIndex);
            }
            else {
                nestedMessage.setLVClusterHandle(lv_ptr);
                protobuf_ptr = ctx->ParseMessage(&nestedMessage, protobuf_ptr);
            }
        }
        else {
            if (fieldInfo.isRepeated)
            {
                protobuf_ptr -= 1;
                do {
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

