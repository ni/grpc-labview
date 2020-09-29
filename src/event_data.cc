//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_server.h>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
using namespace std;
using namespace queryserver;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
// thread_local LVMessageMetadataList* LVMessage::RequestMetadata;
// thread_local LVMessageMetadataList* LVMessage::ResponseMetadata;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LVMessage::InitAsDefaultInstance()
{
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVMessage::LVMessage()
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVMessage::LVMessage(google::protobuf::Arena *arena)
    : google::protobuf::Message(arena)
{
    SharedCtor();
    RegisterArenaDtor(arena);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVMessage::LVMessage(const LVMessage &from)
    : google::protobuf::Message()
{
    _internal_metadata_.MergeFrom<google::protobuf::UnknownFieldSet>(from._internal_metadata_);
    // m_Command.UnsafeSetDefault(&google::protobuf::internal::GetEmptyStringAlreadyInited());
    // if (!from._internal_command().empty())
    // {
    //     m_Command.Set(&google::protobuf::internal::GetEmptyStringAlreadyInited(), from._internal_command(),
    //                  GetArena());
    // }
    // parameter_.UnsafeSetDefault(&google::protobuf::internal::GetEmptyStringAlreadyInited());
    // if (!from._internal_parameter().empty())
    // {
    //     parameter_.Set(&google::protobuf::internal::GetEmptyStringAlreadyInited(), from._internal_parameter(),
    //                    GetArena());
    // }
}

google::protobuf::Message* LVMessage::New() const
{
    return NULL;
} 

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LVMessage::SharedCtor()
{
    // google::protobuf::internal::InitSCC(&scc_info_InvokeRequest_query_5fserver_2eproto.base);
    // command_.UnsafeSetDefault(&google::protobuf::internal::GetEmptyStringAlreadyInited());
    // parameter_.UnsafeSetDefault(&google::protobuf::internal::GetEmptyStringAlreadyInited());
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVMessage::~LVMessage()
{
    SharedDtor();
    _internal_metadata_.Delete<google::protobuf::UnknownFieldSet>();
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LVMessage::SharedDtor()
{
    GOOGLE_DCHECK(GetArena() == nullptr);
    // command_.DestroyNoArena(&google::protobuf::internal::GetEmptyStringAlreadyInited());
    // parameter_.DestroyNoArena(&google::protobuf::internal::GetEmptyStringAlreadyInited());
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LVMessage::ArenaDtor(void *object)
{
    LVMessage *_this = reinterpret_cast<LVMessage *>(object);
    (void)_this;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LVMessage::RegisterArenaDtor(google::protobuf::Arena *)
{
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
// const LVMessage& LVMessage::default_instance()
// {
//     // google::protobuf::internal::InitSCC(&::scc_info_InvokeRequest_query_5fserver_2eproto.base);
//     //return *internal_default_instance();
// }

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LVMessage::Clear()
{
    google::protobuf::uint32 cached_has_bits = 0;
    // Prevent compiler warnings about cached_has_bits being unused
    (void)cached_has_bits;

    // command_.ClearToEmpty(&google::protobuf::internal::GetEmptyStringAlreadyInited(), GetArena());
    // parameter_.ClearToEmpty(&google::protobuf::internal::GetEmptyStringAlreadyInited(), GetArena());
    _internal_metadata_.Clear<google::protobuf::UnknownFieldSet>();
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
const char* LVMessage::_InternalParse(const char* ptr, google::protobuf::internal::ParseContext* ctx)
{
    while (!ctx->Done(&ptr))
    {
        google::protobuf::uint32 tag;
        ptr = google::protobuf::internal::ReadTag(ptr, &tag);
        auto index = (tag >> 3);
        // auto fieldInfo = m_Metadata[index];
        // LVMessageMetadataType dataType = fieldInfo->dataType;
        // switch (dataType)
        // {
        //     case Int32Value:
        //     {
        //         int32_t result;
        //         ptr = google::protobuf::internal::ReadINT32(ptr, &result);
        //     }
        //     case DoubleValue:
        //     {
        //         double result;
        //         ptr = google::protobuf::internal::ReadDOUBLE(ptr, &result);
        //     }
        //     case BoolValue:
        //     {
        //         bool result;
        //         ptr = google::protobuf::internal::ReadBOOL(ptr, &result);
        //     }
        //     case StringValue:
        //     {
        //         auto str = new std::string();
        //         ptr = google::protobuf::internal::InlineGreedyStringParser(str, ptr, ctx); 
        //     }               
        // }
    }
    return ptr;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8* LVMessage::_InternalSerialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{
    // while (!ctx->Done(&ptr))
    // {
    //     google::protobuf::uint32 tag;
    //     ptr = google::protobuf::internal::ReadTag(ptr, &tag);
    //     auto index = (tag >> 3);
    //     auto fieldInfo = m_Metadata[index];
    //     LVMessageMetadataType dataType = fieldInfo->dataType;
    //     switch (dataType)
    //     {
    //         case Int32Value:
    //         {                
    //             int32_t result;
    //             stream->WriteInt32Packed
    //             ptr = google::protobuf::internal::ReadINT32(ptr, &result);
    //         }
    //         case DoubleValue:
    //         {
    //             stream->Wr
    //             double result;
    //             ptr = google::protobuf::internal::ReadDOUBLE(ptr, &result);
    //         }
    //         case BoolValue:
    //         {
    //             bool result;
    //             ptr = google::protobuf::internal::ReadBOOL(ptr, &result);
    //         }
    //         case StringValue:
    //         {
    //             auto str = new std::string();
    //             ptr = google::protobuf::internal::InlineGreedyStringParser(str, ptr, ctx); 
    //         }               
    //     }
    // }
    // google::protobuf::uint32 cached_has_bits = 0;
    // (void)cached_has_bits;

    // // string command = 1;
    // if (this->command().size() > 0)
    // {
    //     google::protobuf::internal::WireFormatLite::VerifyUtf8String(
    //         this->_internal_command().data(), static_cast<int>(this->_internal_command().length()),
    //         google::protobuf::internal::WireFormatLite::SERIALIZE,
    //         "queryserver.InvokeRequest.command");
    //     target = stream->WriteStringMaybeAliased(
    //         1, this->_internal_command(), target);
    // }

    // // string parameter = 2;
    // if (this->parameter().size() > 0)
    // {
    //     google::protobuf::internal::WireFormatLite::VerifyUtf8String(
    //         this->_internal_parameter().data(), static_cast<int>(this->_internal_parameter().length()),
    //         google::protobuf::internal::WireFormatLite::SERIALIZE,
    //         "queryserver.InvokeRequest.parameter");
    //     target = stream->WriteStringMaybeAliased(
    //         2, this->_internal_parameter(), target);
    // }

    // if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields()))
    // {
    //     target = google::protobuf::internal::WireFormat::InternalSerializeUnknownFieldsToArray(
    //         _internal_metadata_.unknown_fields<google::protobuf::UnknownFieldSet>(google::protobuf::UnknownFieldSet::default_instance), target, stream);
    // }
    return target;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVMessage::ByteSizeLong() const
{
    size_t total_size = 0;

    google::protobuf::uint32 cached_has_bits = 0;
    // Prevent compiler warnings about cached_has_bits being unused
    (void)cached_has_bits;

    // string command = 1;
    // if (this->command().size() > 0)
    // {
    //     total_size += 1 + google::protobuf::internal::WireFormatLite::StringSize(this->_internal_command());
    // }

    // // string parameter = 2;
    // if (this->parameter().size() > 0)
    // {
    //     total_size += 1 + google::protobuf::internal::WireFormatLite::StringSize(this->_internal_parameter());
    // }

    // if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields()))
    // {
    //     return google::protobuf::internal::ComputeUnknownFieldsSize(_internal_metadata_, total_size, &_cached_size_);
    // }
    int cached_size = google::protobuf::internal::ToCachedSize(total_size);
    SetCachedSize(cached_size);
    return total_size;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LVMessage::MergeFrom(const google::protobuf::Message &from)
{
    // GOOGLE_DCHECK_NE(&from, this);
    // const LVMessage *source =  dynamic_cast<LVMessage>(&from);
    // if (source == nullptr)
    // {
    //     google::protobuf::internal::ReflectionOps::Merge(from, this);
    // }
    // else
    // {
    //     MergeFrom(*source);
    // }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LVMessage::MergeFrom(const LVMessage &from)
{
    GOOGLE_DCHECK_NE(&from, this);
    _internal_metadata_.MergeFrom<google::protobuf::UnknownFieldSet>(from._internal_metadata_);
    google::protobuf::uint32 cached_has_bits = 0;
    (void)cached_has_bits;

    // if (from.command().size() > 0)
    // {
    //     _internal_set_command(from._internal_command());
    // }
    // if (from.parameter().size() > 0)
    // {
    //     _internal_set_parameter(from._internal_parameter());
    // }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LVMessage::CopyFrom(const google::protobuf::Message &from)
{
    if (&from == this)
        return;
    Clear();
    MergeFrom(from);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LVMessage::CopyFrom(const LVMessage &from)
{
    if (&from == this)
        return;
    Clear();
    MergeFrom(from);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
bool LVMessage::IsInitialized() const
{
    return true;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LVMessage::InternalSwap(LVMessage *other)
{
    using std::swap;
    _internal_metadata_.Swap<google::protobuf::UnknownFieldSet>(&other->_internal_metadata_);
    // command_.Swap(&other->command_, &google::protobuf::internal::GetEmptyStringAlreadyInited(), GetArena());
    // parameter_.Swap(&other->parameter_, &google::protobuf::internal::GetEmptyStringAlreadyInited(), GetArena());
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::Metadata LVMessage::GetMetadata() const
{
    return google::protobuf::Metadata();
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVRequestData::LVRequestData()
{
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
const char* LVRequestData::_InternalParse(const char *ptr, google::protobuf::internal::ParseContext *ctx)
{
    return LVMessage::_InternalParse(ptr, ctx);    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8* LVRequestData::_InternalSerialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{
    return LVMessage::_InternalSerialize(target, stream);    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVResponseData::LVResponseData()
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
const char* LVResponseData::_InternalParse(const char* ptr, google::protobuf::internal::ParseContext* ctx)
{
    return LVMessage::_InternalParse(ptr, ctx);    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8* LVResponseData::_InternalSerialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{
    return LVMessage::_InternalSerialize(target, stream);    
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
GenericMethodData::GenericMethodData(ServerContext *_context, const google::protobuf::Message *_request, google::protobuf::Message *_response)
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
InvokeData::InvokeData(ServerContext *_context, const InvokeRequest *_request, InvokeResponse *_response)
    : EventData(_context)
{
    request = _request;
    response = _response;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
QueryData::QueryData(ServerContext *_context, const QueryRequest *_request, QueryResponse *_response)
    : EventData(_context)
{
    request = _request;
    response = _response;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
RegistrationRequestData::RegistrationRequestData(ServerContext *_context, const RegistrationRequest *_request, ::grpc::ServerWriter<ServerEvent> *_writer)
    : EventData(_context)
{
    request = _request;
    eventWriter = _writer;
}
