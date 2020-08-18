//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_server.h>
#include <thread>
#include <sstream>
#include <fstream>
#include <iostream>

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
static const char *QueryServer_method_names[] = {
    "/queryserver.QueryServer/Invoke",
    "/queryserver.QueryServer/Query",
    "/queryserver.QueryServer/Register",
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVRpcMethodHandler::LVRpcMethodHandler(MethodFunc func, LabVIEWGRPCService* service, LVMessageMetadataList* requestMetadata, LVMessageMetadataList* responseMetadata) :
    m_Func(func),
    m_Service(service),
    m_RequestMetadata(requestMetadata),
    m_ResponseMetadata(responseMetadata)
{
    m_WrappedHandler = new WrappedHandler(func, service);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LVRpcMethodHandler::RunHandler(const HandlerParameter& param)
{
    LVMessage::RequestMetadata = m_RequestMetadata;
    LVMessage::ResponseMetadata = m_ResponseMetadata;
    m_WrappedHandler->RunHandler(param);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void* LVRpcMethodHandler::Deserialize(grpc_call* call, grpc_byte_buffer* req, grpc::Status* status, void** handler_data)
{
    LVMessage::RequestMetadata = m_RequestMetadata;
    LVMessage::ResponseMetadata = m_ResponseMetadata;
    return m_WrappedHandler->Deserialize(call, req, status, handler_data);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LabVIEWGRPCService::LabVIEWGRPCService(LabVIEWQueryServerInstance *instance)
    : m_Instance(instance)
{
    AddMethod(new grpc::internal::RpcServiceMethod(
        QueryServer_method_names[0],
        grpc::internal::RpcMethod::NORMAL_RPC,
        new grpc::internal::RpcMethodHandler<LabVIEWGRPCService, InvokeRequest, InvokeResponse>(
            [](LabVIEWGRPCService *service,
               ServerContext *ctx,
               const google::protobuf::Message *req,
               google::protobuf::Message *resp) {
                return service->GenericMethod(ctx, req, resp, "QueryServer_Invoke");
            },
            this)));

    AddMethod(new grpc::internal::RpcServiceMethod(
        QueryServer_method_names[1],
        grpc::internal::RpcMethod::NORMAL_RPC,
        new grpc::internal::RpcMethodHandler<LabVIEWGRPCService, QueryRequest, QueryResponse>(
            [](LabVIEWGRPCService *service,
               ServerContext *ctx,
               const google::protobuf::Message *req,
               google::protobuf::Message *resp) {
                return service->GenericMethod(ctx, req, resp, "QueryServer_Query");
            },
            this)));

    AddMethod(new grpc::internal::RpcServiceMethod(
        QueryServer_method_names[2],
        grpc::internal::RpcMethod::SERVER_STREAMING,
        new grpc::internal::ServerStreamingHandler<LabVIEWGRPCService, RegistrationRequest, ServerEvent>(
            [](LabVIEWGRPCService *service,
               ServerContext *ctx,
               const RegistrationRequest *req,
               grpc_impl::ServerWriter<ServerEvent> *writer) {
                return service->Register(ctx, req, writer);
            },
            this)));
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
Status LabVIEWGRPCService::GenericMethod(ServerContext *context, const google::protobuf::Message *request, google::protobuf::Message *response, const char *rpcName)
{
    auto data = new GenericMethodData(context, request, response);
    m_Instance->SendEvent(rpcName, data);
    data->WaitForComplete();
    delete data;
    return Status::OK;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
Status LabVIEWGRPCService::Register(ServerContext *context, const RegistrationRequest *request, ServerWriter<ServerEvent> *writer)
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
    m_RegisteredServerMethods.insert(pair<string, LVUserEventRef>(name, item));
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LabVIEWQueryServerInstance::SendEvent(string name, EventData *data)
{
    auto occurrence = m_RegisteredServerMethods.find(name);
    if (occurrence != m_RegisteredServerMethods.end())
    {
        OccurServerEvent(occurrence->second, data);
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int LabVIEWQueryServerInstance::Run(string address, string serverCertificatePath, string serverKeyPath)
{
    ServerStartEventData serverStarted;
    new thread(RunServer, address, serverCertificatePath, serverKeyPath, this, &serverStarted);
    serverStarted.WaitForComplete();
    return serverStarted.serverStartStatus;
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
void LabVIEWQueryServerInstance::RunServer(
    string address,
    string serverCertificatePath,
    string serverKeyPath,
    LabVIEWQueryServerInstance *instance,
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

    LabVIEWGRPCService service(instance);
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
    // Register "service" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *synchronous* service.
    builder.RegisterService(&service);
    // Finally assemble the server.
    instance->m_Server = builder.BuildAndStart();

    if (instance->m_Server != NULL)
    {
        cout << "Server listening on " << server_address << endl;
        serverStarted->NotifyComplete();
        instance->m_Server->Wait();
    }
    else
    {
        serverStarted->serverStartStatus = -1;
        serverStarted->NotifyComplete();
    }
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
