#include <query-server.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using namespace std;

static std::unique_ptr<Server> server;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
Status QueryServerImpl::Invoke(ServerContext* context, const queryserver::InvokeRequest* request, queryserver::InvokeResponse* response)
{
    auto data = new InvokeData(context, request, response);
    OccurServerEvent("QueryServer_Invoke", data);
    data->WaitForComplete();
    delete data;
    return Status::OK;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
Status  QueryServerImpl::Query(ServerContext* context, const ::queryserver::QueryRequest* request, ::queryserver::QueryResponse* response) 
{
    auto data = new QueryData(context, request, response);
    OccurServerEvent("QueryServer_Query", data);
    data->WaitForComplete();
    delete data;
    return Status::OK;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
Status  QueryServerImpl::Register(ServerContext* context, const ::queryserver::RegistrationRequest* request, ::grpc::ServerWriter<queryserver::ServerEvent>* writer)
{
    auto data = new RegistrationRequestData(context, request, writer);
    OccurServerEvent("QueryServer_Register", data);
    data->WaitForComplete();
    delete data;
    return Status::OK;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void RunServer(const char* address)
{
	std::string server_address;
    if (address != NULL)
    {
        server_address = address;
    }
    if (server_address.length() == 0)
    {
        server_address = "0.0.0.0:50051";
    }

	QueryServerImpl service;
	grpc::EnableDefaultHealthCheckService(true);
	grpc::reflection::InitProtoReflectionServerBuilderPlugin();
	ServerBuilder builder;
	// Listen on the given address without any authentication mechanism.
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	// Register "service" as the instance through which we'll communicate with
	// clients. In this case it corresponds to an *synchronous* service.
	builder.RegisterService(&service);
	// Finally assemble the server.
	server = builder.BuildAndStart();
	std::cout << "Server listening on " << server_address << std::endl;
	server->Wait();
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void StopServer()
{
	if (server != NULL)
	{
		server->Shutdown();
		server->Wait();
		server = NULL;
	}
}