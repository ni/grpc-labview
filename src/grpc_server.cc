//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_server.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
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
using grpc::Status;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int ClusterElementSize(LVMessageMetadataType type, bool repeated)
{
#ifndef _PS_4
    if (repeated)
    {
        return 8;
    }
    switch (type)
    {
    case LVMessageMetadataType::BoolValue:
        return 1;
    case LVMessageMetadataType::EnumValue:
    case LVMessageMetadataType::Int32Value:
    case LVMessageMetadataType::UInt32Value:
    case LVMessageMetadataType::FloatValue:
        return 4;
    case LVMessageMetadataType::Int64Value:
    case LVMessageMetadataType::UInt64Value:
    case LVMessageMetadataType::DoubleValue:
    case LVMessageMetadataType::StringValue:
    case LVMessageMetadataType::MessageValue:
        return 8;
    }
#else
    if (repeated)
    {
        return 4;
    }
    switch (type)
    {
    case LVMessageMetadataType::BoolValue:
        return 1;
    case LVMessageMetadataType::Int32Value:
    case LVMessageMetadataType::UInt32Value:
    case LVMessageMetadataType::EnumValue:
    case LVMessageMetadataType::FloatValue:
        return 4;
    case LVMessageMetadataType::StringValue:
    case LVMessageMetadataType::MessageValue:
        return 4;
    case LVMessageMetadataType::Int64Value:
    case LVMessageMetadataType::UInt64Value:
    case LVMessageMetadataType::DoubleValue:
        return 8;
    }
#endif
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int AlignClusterOffset(int clusterOffset, LVMessageMetadataType type, bool repeated)
{
#ifndef _PS_4
    if (clusterOffset == 0)
    {
        return 0;
    }
    auto multiple = ClusterElementSize(type, repeated);    
    int remainder = abs(clusterOffset) % multiple;
    if (remainder == 0)
    {
        return clusterOffset;
    }
    return clusterOffset + multiple - remainder;
#else
    return clusterOffset;
#endif
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void MessageElementMetadataOwner::RegisterMetadata(std::shared_ptr<MessageMetadata> requestMetadata)
{
    std::lock_guard<std::mutex> lock(_mutex);
    
    _registeredMessageMetadata.insert({requestMetadata->messageName, requestMetadata});
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
std::shared_ptr<MessageMetadata> MessageElementMetadataOwner::FindMetadata(const std::string& name)
{
    auto it = _registeredMessageMetadata.find(name);
    if (it != _registeredMessageMetadata.end())
    {
        return (*it).second;
    }
    return nullptr;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void MessageElementMetadataOwner::UpdateMetadataClusterLayout(std::shared_ptr<MessageMetadata>& metadata)
{
    if (metadata->clusterSize != 0)
    {
        return;
    }    
    int clusterOffset = 0;
    for (auto element: metadata->_elements)
    {
        auto elementType = element->type;
        auto nestedElement = element;
        while (elementType == LVMessageMetadataType::MessageValue)
        {
            auto nestedMetadata = FindMetadata(element->embeddedMessageName);
            auto nestedElement = nestedMetadata->_elements.front();
            elementType = nestedElement->type;
        }    
        clusterOffset = AlignClusterOffset(clusterOffset, element->type, element->isRepeated);
        element->clusterOffset = clusterOffset;
        if (element->type == LVMessageMetadataType::MessageValue)
        {                
            auto nestedMetadata = FindMetadata(element->embeddedMessageName);
            UpdateMetadataClusterLayout(nestedMetadata);
            clusterOffset += nestedMetadata->clusterSize;
        }
        else
        {
            clusterOffset += ClusterElementSize(element->type, element->isRepeated);
        }
    }
    metadata->clusterSize = AlignClusterOffset(clusterOffset, LVMessageMetadataType::StringValue, true);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void MessageElementMetadataOwner::FinalizeMetadata()
{
    for (auto metadata: _registeredMessageMetadata)
    {
        UpdateMetadataClusterLayout(metadata.second);
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LabVIEWgRPCServer::LabVIEWgRPCServer() :
    _shutdown(false),
    _genericMethodEvent(0)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LabVIEWgRPCServer::RegisterEvent(std::string name, LVUserEventRef item, std::string requestMetadata, std::string responseMetadata)
{
    std::lock_guard<std::mutex> lock(_mutex);

    LVEventData data = { item, requestMetadata, responseMetadata };
    _registeredServerMethods.insert(std::pair<std::string, LVEventData>(name, data));
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LabVIEWgRPCServer::RegisterGenericMethodEvent(LVUserEventRef item)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _genericMethodEvent = item;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LabVIEWgRPCServer::SendEvent(std::string name, EventData *data)
{
    if (HasGenericMethodEvent())
    {
        OccurServerEvent(_genericMethodEvent, data, name);
    }
    else
    {
        auto eventData = _registeredServerMethods.find(name);
        if (eventData != _registeredServerMethods.end())
        {
            OccurServerEvent(eventData->second.event, data);
        }
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
bool LabVIEWgRPCServer::FindEventData(std::string name, LVEventData& data)
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
bool LabVIEWgRPCServer::HasGenericMethodEvent()
{
    return _genericMethodEvent != 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int LabVIEWgRPCServer::Run(std::string address, std::string serverCertificatePath, std::string serverKeyPath)
{
    FinalizeMetadata();
    
    auto serverStarted = new ServerStartEventData;
    _runThread = std::make_unique<std::thread>(StaticRunServer, this, address, serverCertificatePath, serverKeyPath, serverStarted);
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
        static_cast<CallDataBase*>(tag)->Proceed(ok);
        if (_shutdown)
        {
            break;
        }
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LabVIEWgRPCServer::StaticRunServer(
    LabVIEWgRPCServer* server,
    std::string address,
    std::string serverCertificatePath,
    std::string serverKeyPath,
    ServerStartEventData *serverStarted)
{
    server->RunServer(address, serverCertificatePath, serverKeyPath, serverStarted);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LabVIEWgRPCServer::RunServer(
    std::string address,
    std::string serverCertificatePath,
    std::string serverKeyPath,
    ServerStartEventData *serverStarted)
{
    std::string server_address;
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
        std::cout << "Server listening on " << server_address << std::endl;
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
    _shutdown = true;
    if (_server != nullptr)
    {
        _server->Shutdown();
        _server->Wait();
        _runThread->join();
        _server = nullptr;
    }
}
