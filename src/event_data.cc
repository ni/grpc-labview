//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_server.h>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
using namespace google::protobuf::internal;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
CallData::CallData(LabVIEWgRPCServer* server, grpc::AsyncGenericService *service, grpc::ServerCompletionQueue *cq) :
    _server(server), 
    _service(service),
    _cq(cq),
    _stream(&_ctx),
    _status(CallStatus::Create),
    _writeSemaphore(0),
    _cancelled(false),
    _requestDataReady(false)
{
    Proceed(true);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
bool CallData::ParseFromByteBuffer(const grpc::ByteBuffer& buffer, grpc::protobuf::Message& message)
{
    std::vector<grpc::Slice> slices;
    buffer.Dump(&slices);
    std::string buf;
    buf.reserve(buffer.Length());
    for (auto s = slices.begin(); s != slices.end(); s++)
    {
        buf.append(reinterpret_cast<const char *>(s->begin()), s->size());
    }
    return message.ParseFromString(buf);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
std::unique_ptr<grpc::ByteBuffer> CallData::SerializeToByteBuffer(
    const grpc::protobuf::Message& message)
{
    std::string buf;
    message.SerializeToString(&buf);
    grpc::Slice slice(buf);
    return std::unique_ptr<grpc::ByteBuffer>(new grpc::ByteBuffer(&slice, 1));
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
CallFinishedData::CallFinishedData(CallData* callData)
{
    _call = callData;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void CallFinishedData::Proceed(bool ok)
{
    _call->CallFinished();    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
bool CallData::Write()
{
    if (IsCancelled())
    {
        return false;
    }
    auto wb = SerializeToByteBuffer(*_response);
    grpc::WriteOptions options;
    _status = CallStatus::Writing;
    _stream.Write(*wb, this);
    _writeSemaphore.wait();
    if (IsCancelled())
    {
        return false;
    }
    return true;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void CallData::CallFinished()
{
    _cancelled = _ctx.IsCancelled();      
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void CallData::Finish()
{
    if (_status == CallStatus::PendingFinish)
    {
        _status = CallStatus::Finish;
        Proceed(false);
    }
    else
    {
        _status = CallStatus::Finish;
        _stream.Finish(grpc::Status::OK, this);
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
bool CallData::IsCancelled()
{
    return _cancelled;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
bool CallData::ReadNext()
{
    if (_requestDataReady)
    {
        return true;
    }
    if (IsCancelled())
    {
        return false;
    }
    auto tag = new ReadNextTag(this);
    _stream.Read(&_rb, tag);
    if (!tag->Wait())
    {
        return false;
    }
    _request->Clear();
    ParseFromByteBuffer(_rb, *_request);
    _requestDataReady = true;
    if (IsCancelled())
    {
        return false;
    }
    return true;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void CallData::ReadComplete()
{
    _requestDataReady = false;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void CallData::Proceed(bool ok)
{
    if (!ok)
    {
        if (_status != CallStatus::Finish)
        {
            _status = CallStatus::PendingFinish;
        }
    }
    if (_status == CallStatus::Create)
    {
        // As part of the initial CREATE state, we *request* that the system
        // start processing SayHello requests. In this request, "this" acts are
        // the tag uniquely identifying the request (so that different CallData
        // instances can serve different requests concurrently), in this case
        // the memory address of this CallData instance.
        _service->RequestCall(&_ctx, &_stream, _cq, _cq, this);
        _ctx.AsyncNotifyWhenDone(new CallFinishedData(this));
        _status = CallStatus::Read;
    }
    else if (_status == CallStatus::Read)
    {
        // Spawn a new CallData instance to serve new clients while we process
        // the one for this CallData. The instance will deallocate itself as
        // part of its FINISH state.
        new CallData(_server, _service, _cq);

        _stream.Read(&_rb, this);
        _status = CallStatus::Process;
    }
    else if (_status == CallStatus::Process)
    {
        auto name = _ctx.method();

        LVEventData eventData;
        if (_server->FindEventData(name, eventData) || _server->HasGenericMethodEvent())
        {
            auto requestMetadata = _server->FindMetadata(eventData.requestMetadataName);
            auto responseMetadata = _server->FindMetadata(eventData.responseMetadataName);
            _request = std::make_shared<LVMessage>(requestMetadata);
            _response = std::make_shared<LVMessage>(responseMetadata);
            ParseFromByteBuffer(_rb, *_request);
            _requestDataReady = true;

            _methodData = std::make_shared<GenericMethodData>(this, &_ctx, _request, _response);
            _server->SendEvent(name, _methodData.get());
        }
        else
        {
            _stream.Finish(grpc::Status::CANCELLED, this);
        }       
    }
    else if (_status == CallStatus::Writing)
    {
        _writeSemaphore.notify();
    }
    else if (_status == CallStatus::PendingFinish)
    {        
    }
    else
    {
        assert(_status == CallStatus::Finish);
        delete this;
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
ReadNextTag::ReadNextTag(CallData* callData) :
    _readCompleteSemaphore(0),
    _success(false)
{
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void ReadNextTag::Proceed(bool ok)
{
    _success = ok;
    _readCompleteSemaphore.notify();
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
bool ReadNextTag::Wait()
{
    _readCompleteSemaphore.wait();
    return _success;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVMessage::LVMessage(std::shared_ptr<MessageMetadata> metadata) : 
    _metadata(metadata)
{
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVMessage::~LVMessage()
{
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::Message* LVMessage::New() const
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
    while (!ctx->Done(&ptr))
    {
        google::protobuf::uint32 tag;
        ptr = ReadTag(ptr, &tag);
        auto index = (tag >> 3);
        auto fieldInfo = _metadata->_mappedElements[index];
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
        }
    }
    return ptr;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
const char *LVMessage::ParseBoolean(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
{
    if (fieldInfo.isRepeated)
    {
        auto v = std::make_shared<LVRepeatedBooleanMessageValue>(index);
        ptr = PackedBoolParser(&(v->_value), ptr, ctx);
        _values.emplace(index, v);
    }
    else
    {
        bool result;
        ptr = ReadBOOL(ptr, &result);
        auto v = std::make_shared<LVBooleanMessageValue>(index, result);
        _values.emplace(index, v);
    }
    return ptr;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
const char *LVMessage::ParseInt32(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
{    
    if (fieldInfo.isRepeated)
    {
        auto v = std::make_shared<LVRepeatedInt32MessageValue>(index);
        ptr = PackedInt32Parser(&(v->_value), ptr, ctx);
        _values.emplace(index, v);
    }
    else
    {
        int32_t result;
        ptr = ReadINT32(ptr, &result);
        auto v = std::make_shared<LVInt32MessageValue>(index, result);
        _values.emplace(index, v);
    }
    return ptr;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
const char *LVMessage::ParseUInt32(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
{    
    if (fieldInfo.isRepeated)
    {
        auto v = std::make_shared<LVRepeatedUInt32MessageValue>(index);
        ptr = PackedUInt32Parser(&(v->_value), ptr, ctx);
        _values.emplace(index, v);
    }
    else
    {
        uint32_t result;
        ptr = ReadUINT32(ptr, &result);
        auto v = std::make_shared<LVUInt32MessageValue>(index, result);
        _values.emplace(index, v);
    }
    return ptr;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
const char *LVMessage::ParseEnum(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
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
const char *LVMessage::ParseInt64(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
{    
    if (fieldInfo.isRepeated)
    {
        auto v = std::make_shared<LVRepeatedInt64MessageValue>(index);
        ptr = PackedInt64Parser(&(v->_value), ptr, ctx);
        _values.emplace(index, v);
    }
    else
    {
        int64_t result;
        ptr = ReadINT64(ptr, &result);
        auto v = std::make_shared<LVInt64MessageValue>(index, result);
        _values.emplace(index, v);
    }
    return ptr;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
const char *LVMessage::ParseUInt64(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
{    
    if (fieldInfo.isRepeated)
    {
        auto v = std::make_shared<LVRepeatedUInt64MessageValue>(index);
        ptr = PackedUInt64Parser(&(v->_value), ptr, ctx);
        _values.emplace(index, v);
    }
    else
    {
        uint64_t result;
        ptr = ReadUINT64(ptr, &result);
        auto v = std::make_shared<LVUInt64MessageValue>(index, result);
        _values.emplace(index, v);
    }
    return ptr;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
const char *LVMessage::ParseFloat(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
{    
    if (fieldInfo.isRepeated)
    {
        auto v = std::make_shared<LVRepeatedFloatMessageValue>(index);
        ptr = PackedFloatParser(&(v->_value), ptr, ctx);
        _values.emplace(index, v);
    }
    else
    {
        float result;
        ptr = ReadFLOAT(ptr, &result);
        auto v = std::make_shared<LVFloatMessageValue>(index, result);
        _values.emplace(index, v);
    }
    return ptr;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
const char *LVMessage::ParseDouble(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
{    
    if (fieldInfo.isRepeated)
    {
        auto v = std::make_shared<LVRepeatedDoubleMessageValue>(index);
        ptr = PackedDoubleParser(&(v->_value), ptr, ctx);
        _values.emplace(index, v);
    }
    else
    {
        double result;
        ptr = ReadDOUBLE(ptr, &result);
        auto v = std::make_shared<LVDoubleMessageValue>(index, result);
        _values.emplace(index, v);
    }
    return ptr;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
const char *LVMessage::ParseString(google::protobuf::uint32 tag, const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
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
        ptr -= 1;
        do {
            ptr += 1;
            auto str = v->_value.Add();
            ptr = InlineGreedyStringParser(str, ptr, ctx);
            if (!ctx->DataAvailable(ptr))
            {
                break;
            }
        } while (ExpectTag(tag, ptr));
    }
    else
    {
        auto str = std::string();
        ptr = InlineGreedyStringParser(&str, ptr, ctx);
        auto v = std::make_shared<LVStringMessageValue>(index, str);
        _values.emplace(index, v);
    }
    return ptr;
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
const char *LVMessage::ParseNestedMessage(google::protobuf::uint32 tag, const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
{    
    auto metadata = fieldInfo._owner->FindMetadata(fieldInfo.embeddedMessageName);
    if (fieldInfo.isRepeated)
    {
        ptr -= 1;
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
            ptr += 1;
            auto nestedMessage = std::make_shared<LVMessage>(metadata);
            ptr = ctx->ParseMessage(nestedMessage.get(), ptr);
            v->_value.push_back(nestedMessage);
            if (!ctx->DataAvailable(ptr))
            {
                break;
            }
        } while (ExpectTag(26, ptr));
    }
    else
    {
        auto nestedMessage = std::make_shared<LVMessage>(metadata);
        ptr = ctx->ParseMessage(nestedMessage.get(), ptr);
        auto v = std::make_shared<LVNestedMessageMessageValue>(index, nestedMessage);
        _values.emplace(index, v);
    }
    return ptr;
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

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVMessageValue::LVMessageValue(int protobufId) :
    _protobufId(protobufId)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVNestedMessageMessageValue::LVNestedMessageMessageValue(int protobufId, std::shared_ptr<LVMessage> value) :
    LVMessageValue(protobufId),
    _value(value)
{
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVNestedMessageMessageValue::ByteSizeLong()
{
    return WireFormatLite::TagSize(_protobufId, WireFormatLite::TYPE_MESSAGE) + WireFormatLite::MessageSize(*_value);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8* LVNestedMessageMessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{
    target = stream->EnsureSpace(target);
    return WireFormatLite::InternalWriteMessage(_protobufId, *_value, target, stream);        
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVRepeatedNestedMessageMessageValue::LVRepeatedNestedMessageMessageValue(int protobufId) :
    LVMessageValue(protobufId)
{
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVRepeatedNestedMessageMessageValue::ByteSizeLong()
{
    size_t totalSize = 0;
    totalSize += 1UL * _value.size();
    for (const auto& msg : _value)
    {
        totalSize += WireFormatLite::MessageSize(*msg);
    }
    return totalSize;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8* LVRepeatedNestedMessageMessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{
    for (unsigned int i = 0, n = static_cast<unsigned int>(_value.size()); i < n; i++)
    {
        target = stream->EnsureSpace(target);
        target = WireFormatLite::InternalWriteMessage(_protobufId, *_value[i], target, stream);
    }
    return target;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVStringMessageValue::LVStringMessageValue(int protobufId, std::string& value) :
    LVMessageValue(protobufId),
    _value(value)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVStringMessageValue::ByteSizeLong()
{
    return WireFormatLite::TagSize(_protobufId, WireFormatLite::TYPE_STRING) + WireFormatLite::StringSize(_value);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8* LVStringMessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{    
    target = stream->EnsureSpace(target);
    return stream->WriteString(_protobufId, _value, target);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVRepeatedStringMessageValue::LVRepeatedStringMessageValue(int protobufId) :
    LVMessageValue(protobufId)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVRepeatedStringMessageValue::ByteSizeLong()
{    
    size_t totalSize = 0;
    totalSize += 1 * FromIntSize(_value.size());
    for (int i = 0, n = _value.size(); i < n; i++)
    {
        totalSize += WireFormatLite::StringSize(_value.Get(i));
    }
    return totalSize;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8* LVRepeatedStringMessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{    
    for (int i = 0, n = _value.size(); i < n; i++)
    {
        const auto& s = _value[i];
        target = stream->WriteString(_protobufId, s, target);
    }
    return target;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVBooleanMessageValue::LVBooleanMessageValue(int protobufId, bool value) :
    LVMessageValue(protobufId),
    _value(value)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVBooleanMessageValue::ByteSizeLong()
{
    return WireFormatLite::TagSize(_protobufId, WireFormatLite::TYPE_BOOL) + WireFormatLite::kBoolSize;    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8* LVBooleanMessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{    
    target = stream->EnsureSpace(target);
    return WireFormatLite::WriteBoolToArray(_protobufId, _value, target);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVRepeatedBooleanMessageValue::LVRepeatedBooleanMessageValue(int protobufId) :
    LVMessageValue(protobufId)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVRepeatedBooleanMessageValue::ByteSizeLong()
{    
    size_t totalSize = 0;
    unsigned int count = static_cast<unsigned int>(_value.size());
    size_t dataSize = 1UL * count;
    if (dataSize > 0)
    {
        // passing 2 as type to TagSize because that is what WriteLengthDelim passes during serialize
        totalSize += WireFormatLite::TagSize(_protobufId, (WireFormatLite::FieldType)2) + WireFormatLite::Int32Size(static_cast<google::protobuf::int32>(dataSize));
    }
    totalSize += dataSize;
    return totalSize;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8* LVRepeatedBooleanMessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{
    if (_value.size() > 0)
    {
        target = stream->WriteFixedPacked(_protobufId, _value, target);
    }
    return target;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVInt32MessageValue::LVInt32MessageValue(int protobufId, int value) :
    LVMessageValue(protobufId),
    _value(value)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVInt32MessageValue::ByteSizeLong()
{
    return WireFormatLite::TagSize(_protobufId, WireFormatLite::FieldType::TYPE_INT32) +  WireFormatLite::Int32Size(_value);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8* LVInt32MessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{    
    target = stream->EnsureSpace(target);
    return WireFormatLite::WriteInt32ToArray(_protobufId, _value, target);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVUInt32MessageValue::LVUInt32MessageValue(int protobufId, uint32_t value) :
    LVMessageValue(protobufId),
    _value(value)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVUInt32MessageValue::ByteSizeLong()
{
    return WireFormatLite::TagSize(_protobufId, WireFormatLite::FieldType::TYPE_UINT32) +  WireFormatLite::UInt32Size(_value);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8* LVUInt32MessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{    
    target = stream->EnsureSpace(target);
    return WireFormatLite::WriteUInt32ToArray(_protobufId, _value, target);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVEnumMessageValue::LVEnumMessageValue(int protobufId, int value) :
    LVMessageValue(protobufId),
    _value(value)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVEnumMessageValue::ByteSizeLong()
{
    return WireFormatLite::TagSize(_protobufId, WireFormatLite::FieldType::TYPE_ENUM) +  WireFormatLite::EnumSize(_value);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8* LVEnumMessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{    
    target = stream->EnsureSpace(target);
    return WireFormatLite::WriteEnumToArray(_protobufId, _value, target);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVInt64MessageValue::LVInt64MessageValue(int protobufId, int64_t value) :
    LVMessageValue(protobufId),
    _value(value)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVInt64MessageValue::ByteSizeLong()
{
    return WireFormatLite::TagSize(_protobufId, WireFormatLite::FieldType::TYPE_INT64) + WireFormatLite::Int64Size(_value);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8* LVInt64MessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{    
    target = stream->EnsureSpace(target);
    return WireFormatLite::WriteInt64ToArray(_protobufId, _value, target);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVUInt64MessageValue::LVUInt64MessageValue(int protobufId, uint64_t value) :
    LVMessageValue(protobufId),
    _value(value)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVUInt64MessageValue::ByteSizeLong()
{
    return WireFormatLite::TagSize(_protobufId, WireFormatLite::FieldType::TYPE_UINT64) +  WireFormatLite::UInt64Size(_value);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8* LVUInt64MessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{    
    target = stream->EnsureSpace(target);
    return WireFormatLite::WriteUInt64ToArray(_protobufId, _value, target);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVRepeatedInt32MessageValue::LVRepeatedInt32MessageValue(int protobufId) :
    LVMessageValue(protobufId)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVRepeatedInt32MessageValue::ByteSizeLong()
{    
    size_t totalSize = 0;
    size_t dataSize = WireFormatLite::Int32Size(_value);
    if (dataSize > 0)
    {
        // passing 2 as type to TagSize because that is what WriteLengthDelim passes during serialize
        totalSize += WireFormatLite::TagSize(_protobufId, (WireFormatLite::FieldType)2) + WireFormatLite::Int32Size(static_cast<google::protobuf::int32>(dataSize));
    }
    _cachedSize = ToCachedSize(dataSize);
    totalSize += dataSize;
    return totalSize;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8* LVRepeatedInt32MessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{
    if (_cachedSize > 0)
    {
        target = stream->WriteInt32Packed(_protobufId, _value, _cachedSize, target);
    }
    return target;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVRepeatedUInt32MessageValue::LVRepeatedUInt32MessageValue(int protobufId) :
    LVMessageValue(protobufId)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVRepeatedUInt32MessageValue::ByteSizeLong()
{    
    size_t totalSize = 0;
    size_t dataSize = WireFormatLite::UInt32Size(_value);
    if (dataSize > 0)
    {
        // passing 2 as type to TagSize because that is what WriteLengthDelim passes during serialize
        totalSize += WireFormatLite::TagSize(_protobufId, (WireFormatLite::FieldType)2) + WireFormatLite::UInt32Size(static_cast<google::protobuf::int32>(dataSize));
    }
    _cachedSize = ToCachedSize(dataSize);
    totalSize += dataSize;
    return totalSize;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8* LVRepeatedUInt32MessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{
    if (_cachedSize > 0)
    {
        target = stream->WriteUInt32Packed(_protobufId, _value, _cachedSize, target);
    }
    return target;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVRepeatedEnumMessageValue::LVRepeatedEnumMessageValue(int protobufId) :
    LVMessageValue(protobufId)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVRepeatedEnumMessageValue::ByteSizeLong()
{    
    size_t totalSize = 0;
    size_t dataSize = WireFormatLite::EnumSize(_value);
    if (dataSize > 0)
    {
        // passing 2 as type to TagSize because that is what WriteLengthDelim passes during serialize
        totalSize += WireFormatLite::TagSize(_protobufId, (WireFormatLite::FieldType)2) + WireFormatLite::EnumSize(static_cast<google::protobuf::int32>(dataSize));
    }
    _cachedSize = ToCachedSize(dataSize);
    totalSize += dataSize;
    return totalSize;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8* LVRepeatedEnumMessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{
    if (_cachedSize > 0)
    {
        target = stream->WriteEnumPacked(_protobufId, _value, _cachedSize, target);
    }
    return target;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVRepeatedInt64MessageValue::LVRepeatedInt64MessageValue(int protobufId) :
    LVMessageValue(protobufId)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVRepeatedInt64MessageValue::ByteSizeLong()
{    
    size_t totalSize = 0;
    size_t dataSize = WireFormatLite::Int64Size(_value);
    if (dataSize > 0)
    {
        // passing 2 as type to TagSize because that is what WriteLengthDelim passes during serialize
        totalSize += WireFormatLite::TagSize(_protobufId, (WireFormatLite::FieldType)2) + WireFormatLite::Int64Size(static_cast<google::protobuf::int32>(dataSize));
    }
    _cachedSize = ToCachedSize(dataSize);
    totalSize += dataSize;
    return totalSize;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8* LVRepeatedInt64MessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{
    if (_cachedSize > 0)
    {
        target = stream->WriteInt64Packed(_protobufId, _value, _cachedSize, target);
    }
    return target;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVRepeatedUInt64MessageValue::LVRepeatedUInt64MessageValue(int protobufId) :
    LVMessageValue(protobufId)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVRepeatedUInt64MessageValue::ByteSizeLong()
{    
    size_t totalSize = 0;
    size_t dataSize = WireFormatLite::UInt64Size(_value);
    if (dataSize > 0)
    {
        // passing 2 as type to TagSize because that is what WriteLengthDelim passes during serialize
        totalSize += WireFormatLite::TagSize(_protobufId, (WireFormatLite::FieldType)2) + WireFormatLite::UInt64Size(static_cast<google::protobuf::int32>(dataSize));
    }
    _cachedSize = ToCachedSize(dataSize);
    totalSize += dataSize;
    return totalSize;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8* LVRepeatedUInt64MessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{
    if (_cachedSize > 0)
    {
        target = stream->WriteUInt64Packed(_protobufId, _value, _cachedSize, target);
    }
    return target;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVFloatMessageValue::LVFloatMessageValue(int protobufId, float value) :
    LVMessageValue(protobufId),
    _value(value)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVFloatMessageValue::ByteSizeLong()
{    
    return WireFormatLite::TagSize(_protobufId, WireFormatLite::TYPE_STRING) + WireFormatLite::kFloatSize;    
}
    
//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8* LVFloatMessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{    
    target = stream->EnsureSpace(target);
    return WireFormatLite::WriteFloatToArray(_protobufId, _value, target);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVRepeatedFloatMessageValue::LVRepeatedFloatMessageValue(int protobufId) :
    LVMessageValue(protobufId)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVRepeatedFloatMessageValue::ByteSizeLong()
{    
    size_t totalSize = 0;
    unsigned int count = static_cast<unsigned int>(_value.size());
    size_t dataSize = 4UL * count;
    if (dataSize > 0)
    {
        // passing 2 as type to TagSize because that is what WriteLengthDelim passes during serialize
        totalSize += WireFormatLite::TagSize(_protobufId, (WireFormatLite::FieldType)2) + WireFormatLite::Int32Size(static_cast<google::protobuf::int32>(dataSize));
    }
    totalSize += dataSize;
    return totalSize;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8* LVRepeatedFloatMessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{    
    if (_value.size() > 0)
    {
        target = stream->WriteFixedPacked(_protobufId, _value, target);
    }
    return target;
}


//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVDoubleMessageValue::LVDoubleMessageValue(int protobufId, double value) :
    LVMessageValue(protobufId),
    _value(value)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVDoubleMessageValue::ByteSizeLong()
{    
    return WireFormatLite::TagSize(_protobufId, WireFormatLite::FieldType::TYPE_DOUBLE) +  WireFormatLite::kDoubleSize;    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8* LVDoubleMessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{
    target = stream->EnsureSpace(target);
    return WireFormatLite::WriteDoubleToArray(_protobufId, _value, target);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVRepeatedDoubleMessageValue::LVRepeatedDoubleMessageValue(int protobufId) :
    LVMessageValue(protobufId)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVRepeatedDoubleMessageValue::ByteSizeLong()
{    
    size_t totalSize = 0;
    unsigned int count = static_cast<unsigned int>(_value.size());
    size_t dataSize = 8UL * count;
    if (dataSize > 0)
    {
        // passing 2 as type to TagSize because that is what WriteLengthDelim passes during serialize
        totalSize += WireFormatLite::TagSize(_protobufId, (WireFormatLite::FieldType)2) + WireFormatLite::Int32Size(static_cast<google::protobuf::int32>(dataSize));
    }
    totalSize += dataSize;
    return totalSize;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8* LVRepeatedDoubleMessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{    
    if (_value.size() > 0)
    {
        target = stream->WriteFixedPacked(_protobufId, _value, target);
    }
    return target;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
EventData::EventData(ServerContext *_context) :
    _completed(false)
{
    context = _context;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void EventData::WaitForComplete()
{
    std::unique_lock<std::mutex> lck(lockMutex);
    while (!_completed) lock.wait(lck);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void EventData::NotifyComplete()
{
    std::unique_lock<std::mutex> lck(lockMutex);
    _completed = true;
    lock.notify_all();
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
GenericMethodData::GenericMethodData(CallData* call, ServerContext *context, std::shared_ptr<LVMessage> request, std::shared_ptr<LVMessage> response)
    : EventData(context)
{
    _call = call;
    _request = request;
    _response = response;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
ServerStartEventData::ServerStartEventData()
    : EventData(nullptr)
{
    serverStartStatus = 0;
}