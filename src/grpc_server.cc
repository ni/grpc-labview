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

// //---------------------------------------------------------------------
// //---------------------------------------------------------------------
// LVRpcMethodHandler::LVRpcMethodHandler(MethodFunc func, LabVIEWGRPCService *service, LVMessageMetadataList *requestMetadata, LVMessageMetadataList *responseMetadata) : m_Func(func),
//                                                                                                                                                                         m_Service(service),
//                                                                                                                                                                         m_RequestMetadata(requestMetadata),
//                                                                                                                                                                         m_ResponseMetadata(responseMetadata)
// {
//     m_WrappedHandler = new WrappedHandler(func, service);
// }

// //---------------------------------------------------------------------
// //---------------------------------------------------------------------
// void LVRpcMethodHandler::RunHandler(const HandlerParameter &param)
// {
//     LVMessage::RequestMetadata = m_RequestMetadata;
//     LVMessage::ResponseMetadata = m_ResponseMetadata;
//     m_WrappedHandler->RunHandler(param);
// }

// //---------------------------------------------------------------------
// //---------------------------------------------------------------------
// void *LVRpcMethodHandler::Deserialize(grpc_call *call, grpc_byte_buffer *req, grpc::Status *status, void **handler_data)
// {
//     LVMessage::RequestMetadata = m_RequestMetadata;
//     LVMessage::ResponseMetadata = m_ResponseMetadata;
//     return m_WrappedHandler->Deserialize(call, req, status, handler_data);
// }

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

// class LVReactor : public grpc::experimental::ServerGenericBidiReactor
// {
// public:
//     LVReactor()
//     {
//         this->Finish(Status(grpc::StatusCode::UNIMPLEMENTED, ""));
//     }

//     void OnDone() override { delete this; }
// };

// class LVCallbackGenericService : public grpc::experimental::CallbackGenericService
// {
// public:
//     grpc::experimental::ServerGenericBidiReactor *CreateReactor(grpc::experimental::GenericCallbackServerContext *ctx)
//     {
//         return new LVReactor();
//     }
// };

grpc::AsyncGenericService *_rpcService;

bool ParseFromByteBuffer(grpc::ByteBuffer* buffer, grpc::protobuf::Message* message) {
  std::vector<grpc::Slice> slices;
  (void)buffer->Dump(&slices);
  std::string buf;
  buf.reserve(buffer->Length());
  for (auto s = slices.begin(); s != slices.end(); s++) {
    buf.append(reinterpret_cast<const char*>(s->begin()), s->size());
  }
  return message->ParseFromString(buf);
}

std::unique_ptr<grpc::ByteBuffer> SerializeToByteBuffer(
    grpc::protobuf::Message* message) {
  std::string buf;
  message->SerializeToString(&buf);
  grpc::Slice slice(buf);
  return std::unique_ptr<grpc::ByteBuffer>(new grpc::ByteBuffer(&slice, 1));
}

class CallData
{
public:
    // Take in the "service" instance (in this case representing an asynchronous
    // server) and the completion queue "cq" used for asynchronous communication
    // with the gRPC runtime.
    CallData(LabVIEWQueryServerInstance* instance, grpc::AsyncGenericService *service, grpc::ServerCompletionQueue *cq)
        : _instance(instance), service_(service), cq_(cq), stream_(&ctx_), status_(CREATE)
    {
        // Invoke the serving logic right away.
        Proceed();
    }

    void Proceed()
    {
        if (status_ == CREATE)
        {
            // As part of the initial CREATE state, we *request* that the system
            // start processing SayHello requests. In this request, "this" acts are
            // the tag uniquely identifying the request (so that different CallData
            // instances can serve different requests concurrently), in this case
            // the memory address of this CallData instance.
            service_->RequestCall(&ctx_, &stream_, cq_, cq_, this);
            // Make this instance progress to the PROCESS state.
            status_ = READ;
        }
        else if (status_ == READ)
        {
            // Spawn a new CallData instance to serve new clients while we process
            // the one for this CallData. The instance will deallocate itself as
            // part of its FINISH state.
            new CallData(_instance, service_, cq_);

            stream_.Read(&rb, this);
            status_ = PROCESS;
        }
        else if (status_ == PROCESS)
        {
            //LVRequestData request;
            auto name = ctx_.method();
            QueryRequest request;
            ParseFromByteBuffer(&rb, &request);

            auto query = request.query();

            QueryResponse response;
            response.set_message("148.148");

            auto wb = SerializeToByteBuffer(&response);
            grpc::WriteOptions options;
            stream_.WriteAndFinish(*wb, options, grpc::Status::OK, this);
            status_ = FINISH;
        }
        else
        {
            GPR_ASSERT(status_ == FINISH);
            // Once in the FINISH state, deallocate ourselves (CallData).
            delete this;
        }
    }

private:
    // The means of communication with the gRPC runtime for an asynchronous
    // server.
    grpc::AsyncGenericService *service_;
    // The producer-consumer queue where for asynchronous server notifications.
    grpc::ServerCompletionQueue *cq_;
    // Context for the rpc, allowing to tweak aspects of it such as the use
    // of compression, authentication, as well as to send metadata back to the
    // client.
    grpc::GenericServerContext ctx_;
    grpc::GenericServerAsyncReaderWriter stream_;
    grpc::ByteBuffer rb;

    LabVIEWQueryServerInstance* _instance;

    // Let's implement a tiny state machine with the following states.
    enum CallStatus
    {
        CREATE,
        READ,
        PROCESS,
        FINISH
    };
    CallStatus status_; // The current serving state.}
};

void HandleRpcs(LabVIEWQueryServerInstance* _instance, grpc::ServerCompletionQueue *cq_)
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

        HandleRpcs(instance, cq.get());

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
