//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_server.h>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
using namespace std;
using namespace queryserver;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVMessage::LVMessage(const LVMessageMetadataList &metadata) : _metadata(metadata)
{
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVMessage::~LVMessage()
{
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::Message *LVMessage::New() const
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
        // auto fieldInfo = m_Metadata[index];
        LVMessageMetadataType dataType = LVMessageMetadataType::StringValue; // fieldInfo->dataType;
        switch (dataType)
        {
            case LVMessageMetadataType::Int32Value:
            {
                int32_t result;
                ptr = google::protobuf::internal::ReadINT32(ptr, &result);
            }
            break;
            case LVMessageMetadataType::DoubleValue:
            {
                double result;
                ptr = google::protobuf::internal::ReadDOUBLE(ptr, &result);
            }
            break;
            case LVMessageMetadataType::BoolValue:
            {
                bool result;
                ptr = google::protobuf::internal::ReadBOOL(ptr, &result);
            }
            break;
            case LVMessageMetadataType::StringValue:
            {
                auto str = std::string();
                ptr = google::protobuf::internal::InlineGreedyStringParser(&str, ptr, ctx);

                auto v = new LVStringMessageValue();
                v->protobufId = index;
                v->value = str;
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

    for (auto e : _values)
    {
        auto stringValue = dynamic_cast<LVStringMessageValue*>(e);
        if (stringValue != nullptr)
        {
            target = stream->WriteString(1, stringValue->value, target);
        }
    }


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
    size_t totalSize = 0;

    for (auto e : _values)
    {
        auto stringValue = dynamic_cast<LVStringMessageValue*>(e);
        if (stringValue != nullptr)
        {
            totalSize += 1 + google::protobuf::internal::WireFormatLite::StringSize(stringValue->value);
        }
    }

    //totalSize += 1 + google::protobuf::internal::WireFormatLite::StringSize(std::string("148.148"));

    //     google::protobuf::uint32 cached_has_bits = 0;
    //     // Prevent compiler warnings about cached_has_bits being unused
    //     (void)cached_has_bits;

    //     // string command = 1;
    //     // if (this->command().size() > 0)
    //     // {
    //     //     total_size += 1 + google::protobuf::internal::WireFormatLite::StringSize(this->_internal_command());
    //     // }

    //     // // string parameter = 2;
    //     // if (this->parameter().size() > 0)
    //     // {
    //     //     total_size += 1 + google::protobuf::internal::WireFormatLite::StringSize(this->_internal_parameter());
    //     // }

    //     // if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields()))
    //     // {
    //     //     return google::protobuf::internal::ComputeUnknownFieldsSize(_internal_metadata_, total_size, &_cached_size_);
    //     // }
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
LVRequestData::LVRequestData(const LVMessageMetadataList &metadata) : LVMessage(metadata)
{
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
const char *LVRequestData::_InternalParse(const char *ptr, google::protobuf::internal::ParseContext *ctx)
{
    return LVMessage::_InternalParse(ptr, ctx);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8 *LVRequestData::_InternalSerialize(google::protobuf::uint8 *target, google::protobuf::io::EpsCopyOutputStream *stream) const
{
    return LVMessage::_InternalSerialize(target, stream);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVResponseData::LVResponseData(const LVMessageMetadataList &metadata) : LVMessage(metadata)
{
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
const char *LVResponseData::_InternalParse(const char *ptr, google::protobuf::internal::ParseContext *ctx)
{
    return LVMessage::_InternalParse(ptr, ctx);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8 *LVResponseData::_InternalSerialize(google::protobuf::uint8 *target, google::protobuf::io::EpsCopyOutputStream *stream) const
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
InvokeData::InvokeData(ServerContext *_context, const InvokeRequest *_request, InvokeResponse *_response)
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
