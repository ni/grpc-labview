//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_server.h>
#include <thread>
#include <sstream>
#include <fstream>
#include <iostream>
#include <future>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerWriter;
using grpc::Status;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
using namespace std;
using namespace queryserver;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LabVIEWgRPCServer::RegisterEvent(string name, LVUserEventRef item, std::shared_ptr<MessageMetadata> requestMetadata, std::shared_ptr<MessageMetadata> responseMetadata)
{
    LVEventData data = { item, requestMetadata, responseMetadata };
    _registeredServerMethods.insert(pair<string, LVEventData>(name, data));
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LabVIEWgRPCServer::SendEvent(string name, EventData *data)
{
    auto eventData = _registeredServerMethods.find(name);
    if (eventData != _registeredServerMethods.end())
    {
        OccurServerEvent(eventData->second.event, data);
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
bool LabVIEWgRPCServer::FindEventData(string name, LVEventData& data)
{
    auto eventData = _registeredServerMethods.find(name);
    if (eventData != _registeredServerMethods.end())
    {
        data = eventData->second;
        return true;
    }
    return false;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int LabVIEWgRPCServer::Run(string address, string serverCertificatePath, string serverKeyPath)
{
    auto serverStarted = new ServerStartEventData;
    _runFuture = std::async(std::launch::async, [=]() { RunServer(address, serverCertificatePath, serverKeyPath, serverStarted); });
    serverStarted->WaitForComplete();
    auto result = serverStarted->serverStartStatus;
    delete serverStarted;
    return result;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
std::string read_keycert(const std::string &filename)
{
    std::string data;
    std::ifstream file(filename.c_str(), std::ios::in);
    if (file.is_open())
    {
        std::stringstream ss;
        ss << file.rdbuf();
        file.close();
        data = ss.str();
    }
    return data;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LabVIEWgRPCServer::HandleRpcs(grpc::ServerCompletionQueue *cq)
{
    // Spawn a new CallData instance to serve new clients.
    new CallData(this, _rpcService.get(), cq);
    void *tag; // uniquely identifies a request.
    bool ok;
    while (true)
    {
        // Block waiting to read the next event from the completion queue. The
        // event is uniquely identified by its tag, which in this case is the
        // memory address of a CallData instance.
        cq->Next(&tag, &ok);
        GPR_ASSERT(ok);
        static_cast<CallData*>(tag)->Proceed();
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LabVIEWgRPCServer::RunServer(
    string address,
    string serverCertificatePath,
    string serverKeyPath,
    ServerStartEventData *serverStarted)
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

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ServerBuilder builder;

    std::shared_ptr<grpc::ServerCredentials> creds;
    if (serverCertificatePath.length() > 1)
    {
        std::string servercert = read_keycert(serverCertificatePath);
        std::string serverkey = read_keycert(serverKeyPath);

        grpc::SslServerCredentialsOptions::PemKeyCertPair pkcp;
        pkcp.private_key = serverkey;
        pkcp.cert_chain = servercert;

        grpc::SslServerCredentialsOptions ssl_opts;
        ssl_opts.pem_root_certs = "";
        ssl_opts.pem_key_cert_pairs.push_back(pkcp);

        creds = grpc::SslServerCredentials(ssl_opts);
    }
    else
    {
        creds = grpc::InsecureServerCredentials();
    }

    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, creds);
    _rpcService = std::unique_ptr<grpc::AsyncGenericService>(new grpc::AsyncGenericService());
    builder.RegisterAsyncGenericService(_rpcService.get());
    auto cq = builder.AddCompletionQueue();
    // Finally assemble the server.
    _server = builder.BuildAndStart();

    if (_server != nullptr)
    {
        cout << "Server listening on " << server_address << endl;
        serverStarted->NotifyComplete();

        HandleRpcs(cq.get());
        _server->Wait();
    }
    else
    {
        serverStarted->serverStartStatus = -1;
        serverStarted->NotifyComplete();
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LabVIEWgRPCServer::StopServer()
{
    if (_server != nullptr)
    {
        _server->Shutdown();
        _server->Wait();
        _runFuture.wait();
        _server = nullptr;
    }
}
