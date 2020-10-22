//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_server.h>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
using namespace std;
using namespace queryserver;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
CallData::CallData(LabVIEWgRPCServer* server, grpc::AsyncGenericService *service, grpc::ServerCompletionQueue *cq)
    : _server(server), _service(service), _cq(cq), _stream(&_ctx), _status(CallStatus::Create)
{
    Proceed();
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
bool CallData::ParseFromByteBuffer(grpc::ByteBuffer *buffer, grpc::protobuf::Message *message)
{
    std::vector<grpc::Slice> slices;
    (void)buffer->Dump(&slices);
    std::string buf;
    buf.reserve(buffer->Length());
    for (auto s = slices.begin(); s != slices.end(); s++)
    {
        buf.append(reinterpret_cast<const char *>(s->begin()), s->size());
    }
    return message->ParseFromString(buf);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
std::unique_ptr<grpc::ByteBuffer> CallData::SerializeToByteBuffer(
    grpc::protobuf::Message *message)
{
    std::string buf;
    message->SerializeToString(&buf);
    grpc::Slice slice(buf);
    return std::unique_ptr<grpc::ByteBuffer>(new grpc::ByteBuffer(&slice, 1));
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void CallData::Proceed()
{
    if (_status == CallStatus::Create)
    {
        // As part of the initial CREATE state, we *request* that the system
        // start processing SayHello requests. In this request, "this" acts are
        // the tag uniquely identifying the request (so that different CallData
        // instances can serve different requests concurrently), in this case
        // the memory address of this CallData instance.
        _service->RequestCall(&_ctx, &_stream, _cq, _cq, this);
        // Make this instance progress to the PROCESS state.
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
        if (_server->FindEventData(name, eventData))
        {
            LVMessage request(eventData.requestMetadata->elements);
            LVMessage response(eventData.responsetMetadata->elements);
            ParseFromByteBuffer(&_rb, &request);

            GenericMethodData methodData(&_ctx, &request, &response);
            _server->SendEvent(name, &methodData);

            methodData.WaitForComplete();
            
            auto wb = SerializeToByteBuffer(&response);
            grpc::WriteOptions options;
            _stream.WriteAndFinish(*wb, options, grpc::Status::OK, this);
        }
        else
        {
            _stream.Finish(grpc::Status::CANCELLED, this);
        }
        _status = CallStatus::Finish;
    }
    else
    {
        assert(_status == CallStatus::Finish);
        delete this;
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVMessage::LVMessage(LVMessageMetadataList &metadata) : _metadata(metadata)
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
    return NULL;
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
const char *LVMessage::_InternalParse(const char *ptr, google::protobuf::internal::ParseContext *ctx)
{
    while (!ctx->Done(&ptr))
    {
        google::protobuf::uint32 tag;
        ptr = google::protobuf::internal::ReadTag(ptr, &tag);
        auto index = (tag >> 3);
        auto fieldInfo = _metadata[index];
        LVMessageMetadataType dataType = fieldInfo.type;
        switch (dataType)
        {
            case LVMessageMetadataType::Int32Value:
            {
                int32_t result;
                ptr = google::protobuf::internal::ReadINT32(ptr, &result);
                auto v = std::make_shared<LVInt32MessageValue>(index, result);
                _values.push_back(v);
            }
            break;
            case LVMessageMetadataType::FloatValue:
            {
                float result;
                ptr = google::protobuf::internal::ReadFLOAT(ptr, &result);
                auto v = std::make_shared<LVFloatMessageValue>(index, result);
                _values.push_back(v);
            }
            case LVMessageMetadataType::DoubleValue:
            {
                double result;
                ptr = google::protobuf::internal::ReadDOUBLE(ptr, &result);
                auto v = std::make_shared<LVDoubleMessageValue>(index, result);
                _values.push_back(v);
            }
            break;
            case LVMessageMetadataType::BoolValue:
            {
                bool result;
                ptr = google::protobuf::internal::ReadBOOL(ptr, &result);
                auto v = std::make_shared<LVBooleanMessageValue>(index, result);
                _values.push_back(v);
            }
            break;
            case LVMessageMetadataType::StringValue:
            {
                auto str = std::string();
                ptr = google::protobuf::internal::InlineGreedyStringParser(&str, ptr, ctx);
                auto v = std::make_shared<LVStringMessageValue>(index, str);
                _values.push_back(v);
            }
            break;
        }
    }
    return ptr;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8 *LVMessage::_InternalSerialize(google::protobuf::uint8 *target, google::protobuf::io::EpsCopyOutputStream *stream) const
{
    for (auto e : _values)
    {
        target = e->Serialize(target, stream);
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
        totalSize += e->ByteSizeLong();
    }
    int cachedSize = google::protobuf::internal::ToCachedSize(totalSize);
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
LVStringMessageValue::LVStringMessageValue(int protobufId, std::string& value) :
    LVMessageValue(protobufId),
    _value(value)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVStringMessageValue::ByteSizeLong()
{
    return 1 + google::protobuf::internal::WireFormatLite::StringSize(_value);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8* LVStringMessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{    
    return stream->WriteString(_protobufId, _value, target);
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
    return 1 + google::protobuf::internal::WireFormatLite::kBoolSize;    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8* LVBooleanMessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{    
    target = stream->EnsureSpace(target);
    return google::protobuf::internal::WireFormatLite::WriteBoolToArray(_protobufId, _value, target);
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
    return 1 + google::protobuf::internal::WireFormatLite::Int32Size(_value);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8* LVInt32MessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{    
    target = stream->EnsureSpace(target);
    return google::protobuf::internal::WireFormatLite::WriteInt32ToArray(_protobufId, _value, target);
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
    return 1 + google::protobuf::internal::WireFormatLite::kFloatSize;    
}
    
//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8* LVFloatMessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{    
    target = stream->EnsureSpace(target);
    return google::protobuf::internal::WireFormatLite::WriteFloatToArray(_protobufId, _value, target);
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
    return 1 + google::protobuf::internal::WireFormatLite::kDoubleSize;    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8* LVDoubleMessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{
    target = stream->EnsureSpace(target);
    return google::protobuf::internal::WireFormatLite::WriteDoubleToArray(_protobufId, _value, target);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
EventData::EventData(ServerContext *_context)
{
    context = _context;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void EventData::WaitForComplete()
{
    std::unique_lock<std::mutex> lck(lockMutex);
    lock.wait(lck);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void EventData::NotifyComplete()
{
    std::unique_lock<std::mutex> lck(lockMutex);
    lock.notify_all();
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
GenericMethodData::GenericMethodData(ServerContext *_context, LVMessage *_request, LVMessage *_response)
    : EventData(_context)
{
    request = _request;
    response = _response;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
ServerStartEventData::ServerStartEventData()
    : EventData(NULL)
{
    serverStartStatus = 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
RegistrationRequestData::RegistrationRequestData(ServerContext *_context, const RegistrationRequest *_request, ::grpc::ServerWriter<ServerEvent> *_writer)
    : EventData(_context)
{
    request = _request;
    eventWriter = _writer;
}
