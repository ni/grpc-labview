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
LabVIEWGRPCService::LabVIEWGRPCService(LabVIEWQueryServerInstance *instance)
    : m_Instance(instance)
{
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
void LabVIEWQueryServerInstance::RegisterEvent(string name, LVUserEventRef item, std::shared_ptr<MessageMetadata> requestMetadata, std::shared_ptr<MessageMetadata> responseMetadata)
{
    LVEventData data = { item, requestMetadata, responseMetadata };
    m_RegisteredServerMethods.insert(pair<string, LVEventData>(name, data));
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void LabVIEWQueryServerInstance::SendEvent(string name, EventData *data)
{
    auto eventData = m_RegisteredServerMethods.find(name);
    if (eventData != m_RegisteredServerMethods.end())
    {
        OccurServerEvent(eventData->second.event, data);
    }
}

bool LabVIEWQueryServerInstance::FindEventData(string name, LVEventData& data)
{
    auto eventData = m_RegisteredServerMethods.find(name);
    if (eventData != m_RegisteredServerMethods.end())
    {
        data = eventData->second;
        return true;
    }
    return false;
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

grpc::AsyncGenericService *_rpcService;

//---------------------------------------------------------------------
// Take in the "service" instance (in this case representing an asynchronous
// server) and the completion queue "cq" used for asynchronous communication
// with the gRPC runtime.
//---------------------------------------------------------------------
CallData::CallData(LabVIEWQueryServerInstance *instance, grpc::AsyncGenericService *service, grpc::ServerCompletionQueue *cq)
    : _instance(instance), _service(service), _cq(cq), _stream(&_ctx), _status(CallStatus::Create)
{
    Proceed();
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
bool CallData::ParseFromByteBuffer(grpc::ByteBuffer *buffer, grpc::protobuf::Message *message)
{
    std::vector<grpc::Slice> slices;
    (void)buffer->Dump(&slices);
    std::string buf;
    buf.reserve(buffer->Length());
    for (auto s = slices.begin(); s != slices.end(); s++)
    {
        buf.append(reinterpret_cast<const char *>(s->begin()), s->size());
    }
    return message->ParseFromString(buf);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
std::unique_ptr<grpc::ByteBuffer> CallData::SerializeToByteBuffer(
    grpc::protobuf::Message *message)
{
    std::string buf;
    message->SerializeToString(&buf);
    grpc::Slice slice(buf);
    return std::unique_ptr<grpc::ByteBuffer>(new grpc::ByteBuffer(&slice, 1));
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void CallData::Proceed()
{
    if (_status == CallStatus::Create)
    {
        // As part of the initial CREATE state, we *request* that the system
        // start processing SayHello requests. In this request, "this" acts are
        // the tag uniquely identifying the request (so that different CallData
        // instances can serve different requests concurrently), in this case
        // the memory address of this CallData instance.
        _service->RequestCall(&_ctx, &_stream, _cq, _cq, this);
        // Make this instance progress to the PROCESS state.
        _status = CallStatus::Read;
    }
    else if (_status == CallStatus::Read)
    {
        // Spawn a new CallData instance to serve new clients while we process
        // the one for this CallData. The instance will deallocate itself as
        // part of its FINISH state.
        new CallData(_instance, _service, _cq);

        _stream.Read(&_rb, this);
        _status = CallStatus::Process;
    }
    else if (_status == CallStatus::Process)
    {
        auto name = _ctx.method();

        LVEventData eventData;
        if (_instance->FindEventData(name, eventData))
        {
            LVMessage request(eventData.requestMetadata->elements);
            LVMessage response(eventData.responsetMetadata->elements);
            ParseFromByteBuffer(&_rb, &request);

            GenericMethodData methodData(&_ctx, &request, &response);
            _instance->SendEvent(name, &methodData);

            methodData.WaitForComplete();
            
            auto wb = SerializeToByteBuffer(&response);
            grpc::WriteOptions options;
            _stream.WriteAndFinish(*wb, options, grpc::Status::OK, this);
        }
        else
        {
            _stream.Finish(grpc::Status::CANCELLED, this);
        }
        _status = CallStatus::Finish;
    }
    else
    {
        assert(_status == CallStatus::Finish);
        delete this;
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void HandleRpcs(LabVIEWQueryServerInstance *_instance, grpc::ServerCompletionQueue *cq_)
{
    // Spawn a new CallData instance to serve new clients.
    new CallData(_instance, _rpcService, cq_);
    void *tag; // uniquely identifies a request.
    bool ok;
    while (true)
    {
        // Block waiting to read the next event from the completion queue. The
        // event is uniquely identified by its tag, which in this case is the
        // memory address of a CallData instance.
        cq_->Next(&tag, &ok);
        GPR_ASSERT(ok);
        static_cast<CallData *>(tag)->Proceed();
    }
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
    //builder.RegisterService(&service);

    _rpcService = new grpc::AsyncGenericService();
    builder.RegisterAsyncGenericService(_rpcService);
    auto cq = builder.AddCompletionQueue();
    // Finally assemble the server.
    instance->m_Server = builder.BuildAndStart();

    if (instance->m_Server != NULL)
    {
        cout << "Server listening on " << server_address << endl;
        serverStarted->NotifyComplete();

        HandleRpcs(instance, cq.get());
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
