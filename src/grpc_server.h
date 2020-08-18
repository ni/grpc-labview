//---------------------------------------------------------------------
// LabVIEW implementation of a gRPC Server
//---------------------------------------------------------------------
#pragma once

#ifdef __WIN32__
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <query_server.grpc.pb.h>
#include <lv_interop.h>
#include <condition_variable>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerWriter;
using namespace queryserver;
using namespace std;

//---------------------------------------------------------------------
// QueryServer LabVIEW definitions
//---------------------------------------------------------------------
typedef void* LVgRPCid;
typedef void* LVgRPCServerid;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LabVIEWQueryServerInstance;
class LabVIEWGRPCService;
class LVMessage;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class EventData
{
public:
    EventData(ServerContext* context);

private:
    mutex lockMutex;
    condition_variable lock;

public:
    ServerContext* context;

public:
    void WaitForComplete();
    void NotifyComplete();
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
enum LVMessageMetadataType
{
    Int32Value,
    DoubleValue,
    BoolValue,
    StringValue
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LVMessageMetadata
{
public:
    int protobufIndex;
    int LVClusterOffset;
    LVMessageMetadataType type;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
typedef std::list<LVMessageMetadata> LVMessageMetadataList;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LVMessage : public google::protobuf::Message
{
public:
    static thread_local LVMessageMetadataList* RequestMetadata;
    static thread_local LVMessageMetadataList* ResponseMetadata;

    LVMessage();
    LVMessage(const LVMessage &from);
    LVMessage(google::protobuf::Arena *arena);

    ~LVMessage();

    google::protobuf::Message* New() const final; 
    void SharedCtor();
    void SharedDtor();
    void ArenaDtor(void* object);
    void RegisterArenaDtor(google::protobuf::Arena*);
    const LVMessage &default_instance();
    void InitAsDefaultInstance();

    void Clear()  final;
    bool IsInitialized() const final;

    const char *_InternalParse(const char *ptr, google::protobuf::internal::ParseContext *ctx)  override;
    google::protobuf::uint8 *_InternalSerialize(google::protobuf::uint8 *target, google::protobuf::io::EpsCopyOutputStream *stream) const override;
    void SetCachedSize(int size) const final;
    int GetCachedSize(void) const final;
    size_t ByteSizeLong() const final;
    
    void MergeFrom(const google::protobuf::Message &from) final;
    void MergeFrom(const LVMessage &from);
    void CopyFrom(const google::protobuf::Message &from) final;
    void CopyFrom(const LVMessage &from);
    void InternalSwap(LVMessage *other);
    google::protobuf::Metadata GetMetadata() const final;

private:
  mutable google::protobuf::internal::CachedSize _cached_size_;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LVRequestData : public LVMessage
{   
public:     
    LVRequestData();

    const char* _InternalParse(const char* ptr, google::protobuf::internal::ParseContext* ctx) final;
    google::protobuf::uint8 *_InternalSerialize(google::protobuf::uint8 *target, google::protobuf::io::EpsCopyOutputStream *stream) const final;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LVResponseData : public LVMessage
{   
public: 
    LVResponseData();

    const char* _InternalParse(const char* ptr, google::protobuf::internal::ParseContext* ctx) final;
    google::protobuf::uint8* _InternalSerialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const final;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LVRpcMethodHandler : public grpc::internal::MethodHandler
{
public:
    typedef std::function<::grpc::Status(LabVIEWGRPCService*, grpc_impl::ServerContext*, const LVRequestData*, LVResponseData*)> MethodFunc;
    typedef grpc::internal::RpcMethodHandler<LabVIEWGRPCService, LVRequestData, LVResponseData> WrappedHandler;

    LVRpcMethodHandler(MethodFunc func, LabVIEWGRPCService* service, LVMessageMetadataList* requestMetadats, LVMessageMetadataList* responseMetadata);
    void RunHandler(const HandlerParameter &param) final;
    void* Deserialize(grpc_call* call, grpc_byte_buffer* req, ::grpc::Status* status, void** /*handler_data*/) final;

private:
    MethodFunc m_Func;
    LabVIEWGRPCService* m_Service;
    WrappedHandler* m_WrappedHandler;
    LVMessageMetadataList* m_RequestMetadata;
    LVMessageMetadataList* m_ResponseMetadata;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LabVIEWGRPCService final : public grpc::Service
{
public:
    LabVIEWGRPCService(LabVIEWQueryServerInstance* instance);
    void SopServer();
    void RegisterEvent(string eventName, LVUserEventRef reference);

    Status GenericMethod(ServerContext* context, const google::protobuf::Message* request, google::protobuf::Message* response, const char* rpcName);

    // RPC Methods
    Status Register(ServerContext*context, const RegistrationRequest* request, ServerWriter<ServerEvent>* writer);

private:
    LabVIEWQueryServerInstance* m_Instance;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class ServerStartEventData : public EventData
{
public:
    ServerStartEventData();

public:
    int serverStartStatus;
};


//---------------------------------------------------------------------
//---------------------------------------------------------------------
class GenericMethodData : public EventData
{
public:
    GenericMethodData(ServerContext* context, const google::protobuf::Message* request, google::protobuf::Message* response);

public:
    const google::protobuf::Message* request;
    const google::protobuf::Message* response;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class InvokeData : public EventData
{
public:
    InvokeData(ServerContext* context, const InvokeRequest* request, InvokeResponse* response);

public:
    const InvokeRequest* request;
    InvokeResponse* response;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class QueryData : public EventData
{
public:
    QueryData(ServerContext* context, const QueryRequest* request, QueryResponse* response);

public:
    const QueryRequest* request;
    QueryResponse* response;
};


//---------------------------------------------------------------------
//---------------------------------------------------------------------
class RegistrationRequestData : public EventData
{
public:
    RegistrationRequestData(ServerContext* context, const RegistrationRequest* request, ServerWriter<queryserver::ServerEvent>* writer);

public:
    const queryserver::RegistrationRequest* request;
    ::grpc::ServerWriter<::queryserver::ServerEvent>* eventWriter;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LabVIEWQueryServerInstance
{
public:
    int Run(string address, string serverCertificatePath, string serverKeyPath);
    void StopServer();
    void RegisterEvent(string eventName, LVUserEventRef reference);
    void SendEvent(string name, EventData* data);

private:
    unique_ptr<Server> m_Server;
    map<string, LVUserEventRef> m_RegisteredServerMethods;

private:
    static void RunServer(string address, string serverCertificatePath, string serverKeyPath, LabVIEWQueryServerInstance* instance, ServerStartEventData* serverStarted);
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
struct LVInvokeRequest
{
    LStrHandle command;
    LStrHandle parameter;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
struct LVInvokeResponse
{
    int32_t status;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
struct LVQueryRequest
{
    LStrHandle query;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
struct LVQueryResponse
{
    LStrHandle message;
    int32_t status;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
struct LVRegistrationRequest
{
    LStrHandle eventName;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
struct LVServerEvent
{
    LStrHandle eventData;
    int32_t serverId;
    int32_t status;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void OccurServerEvent(LVUserEventRef event, EventData* data);
