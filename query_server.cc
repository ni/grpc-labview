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
LabVIEWQueryServer::LabVIEWQueryServer(LabVIEWQueryServerInstance* instance)
    : m_Instance(instance)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
Status LabVIEWQueryServer::Invoke(ServerContext* context, const InvokeRequest* request, InvokeResponse* response)
{
    auto data = new InvokeData(context, request, response);
    m_Instance->SendEvent("QueryServer_Invoke", data);
    data->WaitForComplete();
    delete data;
    return Status::OK;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
Status LabVIEWQueryServer::Query(ServerContext* context, const QueryRequest* request, QueryResponse* response) 
{
    auto data = new QueryData(context, request, response);
    m_Instance->SendEvent("QueryServer_Query", data);
    data->WaitForComplete();
    delete data;
    return Status::OK;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
Status LabVIEWQueryServer::Register(ServerContext* context, const RegistrationRequest* request, ServerWriter<ServerEvent>* writer)
{
    auto data = new RegistrationRequestData(context, request, writer);
    m_Instance->SendEvent("QueryServer_Register", data);
    data->WaitForComplete();
    delete data;
    return Status::OK;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LabVIEWQueryServerInstance::RegisterEvent(string name, LVUserEventRef item)
{    
	m_RegisteredServerMethods.insert(pair<string,LVUserEventRef>(name, item));
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LabVIEWQueryServerInstance::SendEvent(string name, EventData* data)
{
	auto occurrence = m_RegisteredServerMethods.find(name);
	if (occurrence != m_RegisteredServerMethods.end())
	{
		OccurServerEvent(occurrence->second, data);
	}
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LabVIEWQueryServerInstance::Run(string address)
{
    new thread(RunServer, address, this);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LabVIEWQueryServerInstance::RunServer(string address, LabVIEWQueryServerInstance* instance)
{
	string server_address;
    if (address.length() != 0)
    {
        server_address = address;
    }
    if (server_address.length() == 0)
    {
        server_address = "0.0.0.0:50051";
    }

	LabVIEWQueryServer service(instance);
	grpc::EnableDefaultHealthCheckService(true);
	grpc::reflection::InitProtoReflectionServerBuilderPlugin();
	ServerBuilder builder;
	// Listen on the given address without any authentication mechanism.
	builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
	// Register "service" as the instance through which we'll communicate with
	// clients. In this case it corresponds to an *synchronous* service.
	builder.RegisterService(&service);
	// Finally assemble the server.
	instance->m_Server = builder.BuildAndStart();
	cout << "Server listening on " << server_address << endl;
	instance->m_Server->Wait();
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LabVIEWQueryServerInstance::StopServer()
{
	if (m_Server != NULL)
	{
		m_Server->Shutdown();
		m_Server->Wait();
		m_Server = NULL;
	}
}
