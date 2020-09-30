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
enum class LVMessageMetadataType
{
    Int32Value,
    DoubleValue,
    BoolValue,
    StringValue,
    MessageValue
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class MessageElementMetadata
{
public:
    std::string embeddedMessageName;
    
    int protobufIndex;
    
    int clusterOffset;

    LVMessageMetadataType type;    

    bool isRepeated;    
};

struct LVMesageElementMetadata
{
    LStrHandle embeddedMessageName;
    int protobufIndex;
    int clusterOffset;
    int valueType;
    bool isRepeated;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
using LVMessageMetadataList = std::map<int, MessageElementMetadata>;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
struct MessageMetadata
{
    std::string messageName;
    LVMessageMetadataList elements;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
struct LVMessageMetadata
{
    LStrHandle messageName;
    LV1DArrayHandle elements;
};

class LVMessageValue
{
public:
    int protobufId;    

    virtual void* RawValue() = 0;
};

class LVStringMessageValue : public LVMessageValue
{
public:
    std::string value;

    void* RawValue() override { return (void*)(value.c_str()); };
};

class LVInt32MessageValue : public LVMessageValue
{
public:
    int value;    

    void* RawValue() override { return &value; };
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LVMessage : public google::protobuf::Message
{
public:
    LVMessage(const LVMessageMetadataList& metadata);

    ~LVMessage();

    google::protobuf::Message* New() const final; 
    void SharedCtor();
    void SharedDtor();
    void ArenaDtor(void* object);
    void RegisterArenaDtor(google::protobuf::Arena*);

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

public:
    std::vector<LVMessageValue*> _values;
    const LVMessageMetadataList& _metadata;

private:
    mutable google::protobuf::internal::CachedSize _cached_size_;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LVRequestData : public LVMessage
{   
public:     
    LVRequestData(const LVMessageMetadataList& metadata);

    const char* _InternalParse(const char* ptr, google::protobuf::internal::ParseContext* ctx) final;
    google::protobuf::uint8 *_InternalSerialize(google::protobuf::uint8 *target, google::protobuf::io::EpsCopyOutputStream *stream) const final;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LVResponseData : public LVMessage
{   
public: 
    LVResponseData(const LVMessageMetadataList& metadata);

    const char* _InternalParse(const char* ptr, google::protobuf::internal::ParseContext* ctx) final;
    google::protobuf::uint8* _InternalSerialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const final;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LabVIEWGRPCService final : public grpc::Service
{
public:
    LabVIEWGRPCService(LabVIEWQueryServerInstance* instance);
    void SopServer();
    void RegisterEvent(string eventName, LVUserEventRef reference, std::shared_ptr<MessageMetadata> requestMetadata, std::shared_ptr<MessageMetadata> responseMetadata);

    //Status GenericMethod(ServerContext* context, const google::protobuf::Message* request, google::protobuf::Message* response, const char* rpcName);

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
    GenericMethodData(ServerContext* context, LVMessage* request, LVMessage* response);

public:
    LVMessage* request;
    LVMessage* response;
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
class RegistrationRequestData : public EventData
{
public:
    RegistrationRequestData(ServerContext* context, const RegistrationRequest* request, ServerWriter<queryserver::ServerEvent>* writer);

public:
    const queryserver::RegistrationRequest* request;
    ::grpc::ServerWriter<::queryserver::ServerEvent>* eventWriter;
};

struct LVEventData
{
    LVUserEventRef event;
    std::shared_ptr<MessageMetadata> requestMetadata;
    std::shared_ptr<MessageMetadata> responsetMetadata;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LabVIEWQueryServerInstance
{
public:
    int Run(string address, string serverCertificatePath, string serverKeyPath);
    void StopServer();
    void RegisterEvent(string eventName, LVUserEventRef reference, std::shared_ptr<MessageMetadata> requestMetadata, std::shared_ptr<MessageMetadata> responseMetadata);
    void SendEvent(string name, EventData* data);

    bool FindEventData(string name, LVEventData& data);

private:
    unique_ptr<Server> m_Server;
    map<string, LVEventData> m_RegisteredServerMethods;

private:
    static void RunServer(string address, string serverCertificatePath, string serverKeyPath, LabVIEWQueryServerInstance* instance, ServerStartEventData* serverStarted);
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class CallData
{
public:
    CallData(LabVIEWQueryServerInstance *instance, grpc::AsyncGenericService *service, grpc::ServerCompletionQueue *cq);
    void Proceed();

private:
    bool ParseFromByteBuffer(grpc::ByteBuffer *buffer, grpc::protobuf::Message *message);
    std::unique_ptr<grpc::ByteBuffer> SerializeToByteBuffer(grpc::protobuf::Message *message);

private:
    grpc::AsyncGenericService *_service;
    grpc::ServerCompletionQueue *_cq;
    grpc::GenericServerContext _ctx;
    grpc::GenericServerAsyncReaderWriter _stream;
    grpc::ByteBuffer _rb;

    LabVIEWQueryServerInstance *_instance;

    enum class CallStatus
    {
        Create,
        Read,
        Process,
        Finish
    };
    CallStatus _status;
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
