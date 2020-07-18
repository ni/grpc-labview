#ifdef __WIN32__
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <query-server.grpc.pb.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

#ifdef _WIN32
    #define LIBRARY_EXPORT extern "C" __declspec(dllexport)
#else
    #define LIBRARY_EXPORT
#endif

//---------------------------------------------------------------------
//---------------------------------------------------------------------
typedef __int32 MagicCookie;
typedef MagicCookie LVRefNum;
typedef MagicCookie LVUserEventRef;

typedef void* LVgRPCid;

typedef struct {
	__int32 cnt; /* number of bytes that follow */
	char str[1]; /* cnt bytes */
} LStr, * LStrPtr, ** LStrHandle;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class EventData
{
public:
    EventData(ServerContext* context);

private:
	std::mutex lockMutex;
	std::condition_variable lock;

public:
	ServerContext* context;

public:
    void WaitForComplete();
    void NotifyComplete();
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class QueryServerImpl final : public queryserver::QueryServer::Service
{
public:
    Status Invoke(ServerContext* context, const queryserver::InvokeRequest* request, queryserver::InvokeResponse* response) override;
    Status Query(ServerContext* context, const ::queryserver::QueryRequest* request, ::queryserver::QueryResponse* response) override; 
    Status Register(ServerContext*context, const ::queryserver::RegistrationRequest* request, ::grpc::ServerWriter<queryserver::ServerEvent>* writer) override;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class InvokeData : public EventData
{
public:
    InvokeData(ServerContext* context, const queryserver::InvokeRequest* request, queryserver::InvokeResponse* response);

public:
	const queryserver::InvokeRequest* request;
	queryserver::InvokeResponse* response;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class QueryData : public EventData
{
public:
    QueryData(ServerContext* context, const ::queryserver::QueryRequest* request, ::queryserver::QueryResponse* response);

public:
	const queryserver::QueryRequest* request;
	queryserver::QueryResponse* response;
};


//---------------------------------------------------------------------
//---------------------------------------------------------------------
class RegistrationRequestData : public EventData
{
public:
    RegistrationRequestData(ServerContext* context, const ::queryserver::RegistrationRequest* request, ::grpc::ServerWriter<queryserver::ServerEvent>* writer);

public:
    const queryserver::RegistrationRequest* request;
    ::grpc::ServerWriter<::queryserver::ServerEvent>* eventWriter;
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
    __int32 status;
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
    __int32 status;
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
    __int32 serverId;
    __int32 status;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void OccurServerEvent(const char* name, EventData* data);

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void RunServer(const char* address);

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void StopServer();
