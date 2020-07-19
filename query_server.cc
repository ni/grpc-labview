#include <query-server.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerWriter;
using namespace std;
using namespace queryserver;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
static std::unique_ptr<Server> s_Server;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
Status LabVIEWQueryServer::Invoke(ServerContext* context, const InvokeRequest* request, InvokeResponse* response)
{
    auto data = new InvokeData(context, request, response);
    OccurServerEvent("QueryServer_Invoke", data);
    data->WaitForComplete();
    delete data;
    return Status::OK;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
Status LabVIEWQueryServer::Query(ServerContext* context, const QueryRequest* request, QueryResponse* response) 
{
    auto data = new QueryData(context, request, response);
    OccurServerEvent("QueryServer_Query", data);
    data->WaitForComplete();
    delete data;
    return Status::OK;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
Status LabVIEWQueryServer::Register(ServerContext* context, const RegistrationRequest* request, ServerWriter<ServerEvent>* writer)
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

	LabVIEWQueryServer service;
	grpc::EnableDefaultHealthCheckService(true);
	grpc::reflection::InitProtoReflectionServerBuilderPlugin();
	ServerBuilder builder;
	// Listen on the given address without any authentication mechanism.
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	// Register "service" as the instance through which we'll communicate with
	// clients. In this case it corresponds to an *synchronous* service.
	builder.RegisterService(&service);
	// Finally assemble the server.
	s_Server = builder.BuildAndStart();
	std::cout << "Server listening on " << server_address << std::endl;
	s_Server->Wait();
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void StopServer()
{
	if (s_Server != NULL)
	{
		s_Server->Shutdown();
		s_Server->Wait();
		s_Server = NULL;
	}
}
