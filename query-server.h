//---------------------------------------------------------------------
// Implementation objects for the LabVIEW implementation of the
// gRPC QueryServer
//---------------------------------------------------------------------
#pragma once

#ifdef __WIN32__
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <query-server.grpc.pb.h>
#include <condition_variable>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerWriter;
using namespace queryserver;

#ifdef _WIN32
    #define LIBRARY_EXPORT extern "C" __declspec(dllexport)
#else
    #define LIBRARY_EXPORT extern "C"
#endif

//---------------------------------------------------------------------
// LabVIEW definitions
//---------------------------------------------------------------------
typedef __int32_t MagicCookie;
typedef MagicCookie LVRefNum;
typedef MagicCookie LVUserEventRef;

typedef struct {
	__int32_t cnt; /* number of bytes that follow */
	char str[1]; /* cnt bytes */
} LStr, * LStrPtr, ** LStrHandle;

//---------------------------------------------------------------------
// QueryServer LabVIEW definitions
//---------------------------------------------------------------------
typedef void* LVgRPCid;

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
class LabVIEWQueryServer final : public queryserver::QueryServer::Service
{
public:
    // Overrides
    Status Invoke(ServerContext* context, const InvokeRequest* request, InvokeResponse* response) override;
    Status Query(ServerContext* context, const QueryRequest* request, QueryResponse* response) override; 
    Status Register(ServerContext*context, const RegistrationRequest* request, ServerWriter<ServerEvent>* writer) override;
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
struct LVInvokeRequest
{
    LStrHandle command;
    LStrHandle parameter;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
struct LVInvokeResponse
{
    __int32_t status;
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
    __int32_t status;
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
    __int32_t serverId;
    __int32_t status;
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
