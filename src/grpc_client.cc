//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_client.h>
#include <lv_interop.h>
#include <lv_message.h>
#include <cluster_copier.h>
#include <grpcpp/impl/codegen/client_context.h>
#include <grpcpp/impl/codegen/client_unary_call.h>

namespace grpc_labview
{
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    LabVIEWgRPCClient::LabVIEWgRPCClient()
    {    
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LabVIEWgRPCClient::Connect(const char* address, const std::string& certificatePath)
    {        
        std::shared_ptr<grpc::ChannelCredentials> creds;
        if (!certificatePath.empty())
        {
            std::string cacert = read_keycert(certificatePath);
            grpc::SslCredentialsOptions ssl_opts;
            ssl_opts.pem_root_certs=cacert;
            creds = grpc::SslCredentials(ssl_opts);
        }
        else
        {
            creds = grpc::InsecureChannelCredentials();
        }
        Channel = grpc::CreateChannel(address, creds);
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t CreateClient(const char* address, const char* certificatePath, grpc_labview::gRPCid** clientId)
{
    grpc_labview::InitCallbacks();

    auto client = new grpc_labview::LabVIEWgRPCClient();
    client->Connect(address, certificatePath);
    *clientId = client;
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t CloseClient(grpc_labview::gRPCid* clientId)
{
    auto client = clientId->CastTo<grpc_labview::LabVIEWgRPCClient>();
    if (!client)
    {
        return -1;
    }
    delete client;
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientUnaryCall(grpc_labview::gRPCid* clientId, grpc_labview::MagicCookie* occurrence, const char* methodName, const char* requestMessageName, const char* responseMessageName, int8_t* requestCluster, grpc_labview::gRPCid** callId)
{
    auto client = clientId->CastTo<grpc_labview::LabVIEWgRPCClient>();
    if (!client)
    {
        return -1;
    }
    auto requestMetadata = client->FindMetadata(requestMessageName);
    if (requestMetadata == nullptr)
    {
        return -2;
    }
    auto responseMetadata = client->FindMetadata(responseMessageName);
    if (responseMetadata == nullptr)
    {
        return -3;
    }

    auto clientCall = new grpc_labview::ClientCall();
    *callId = clientCall;
    clientCall->occurrence = *occurrence;
    clientCall->request = std::make_shared<grpc_labview::LVMessage>(requestMetadata);
    clientCall->response = std::make_shared<grpc_labview::LVMessage>(responseMetadata);

    grpc_labview::ClusterDataCopier::CopyFromCluster(*clientCall->request.get(), requestCluster);

    clientCall->_runFuture = std::async(
        std::launch::async, 
        [=]() 
        {
            grpc::internal::RpcMethod method(methodName, grpc::internal::RpcMethod::NORMAL_RPC);
            clientCall->status = grpc::internal::BlockingUnaryCall(client->Channel.get(), method, &clientCall->context, *clientCall->request.get(), clientCall->response.get());
            grpc_labview::SignalOccurrence(clientCall->occurrence);
            return 0;
        });
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t CompleteClientUnaryCall(grpc_labview::gRPCid* callId, int8_t* responseCluster)
{
    auto call = callId->CastTo<grpc_labview::ClientCall>();
    if (!call)
    {
        return -1;
    }

    int32_t result = 0;
    if (call->status.ok())
    {
        grpc_labview::ClusterDataCopier::CopyToCluster(*call->response.get(), responseCluster);
    }
    else
    {
        result = -(1000 + call->status.error_code());
    }
    delete call;
    return result;
}


//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientBeginClientStreamingCall(grpc_labview::gRPCid* clientId, const char* methodName, const char* requestMessageName, const char* responseMessageName, grpc_labview::gRPCid** callId)
{
    auto client = clientId->CastTo<grpc_labview::LabVIEWgRPCClient>();
    if (!client)
    {
        return -1;
    }
    auto requestMetadata = client->FindMetadata(requestMessageName);
    if (requestMetadata == nullptr)
    {
        return -2;
    }
    auto responseMetadata = client->FindMetadata(responseMessageName);
    if (responseMetadata == nullptr)
    {
        return -3;
    }

    auto clientCall = new grpc_labview::ClientStreamingClientCall();
    *callId = clientCall;
    clientCall->request = std::make_shared<grpc_labview::LVMessage>(requestMetadata);
    clientCall->response = std::make_shared<grpc_labview::LVMessage>(responseMetadata);

    grpc::internal::RpcMethod method(methodName, grpc::internal::RpcMethod::CLIENT_STREAMING);
    auto writer = grpc_impl::internal::ClientWriterFactory<grpc_labview::LVMessage>::Create(client->Channel.get(), method, &clientCall->context, clientCall->response.get());
    clientCall->_writer = std::shared_ptr<grpc_impl::ClientWriterInterface<grpc_labview::LVMessage>>(writer);
    return 0;    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientBeginServerStreamingCall(grpc_labview::gRPCid* clientId, const char* methodName, const char* requestMessageName, const char* responseMessageName, int8_t* requestCluster, grpc_labview::gRPCid** callId)
{    
    auto client = clientId->CastTo<grpc_labview::LabVIEWgRPCClient>();
    if (!client)
    {
        return -1;
    }
    auto requestMetadata = client->FindMetadata(requestMessageName);
    if (requestMetadata == nullptr)
    {
        return -2;
    }
    auto responseMetadata = client->FindMetadata(responseMessageName);
    if (responseMetadata == nullptr)
    {
        return -3;
    }

    auto clientCall = new grpc_labview::ServerStreamingClientCall();
    *callId = clientCall;
    clientCall->request = std::make_shared<grpc_labview::LVMessage>(requestMetadata);
    clientCall->response = std::make_shared<grpc_labview::LVMessage>(responseMetadata);

    grpc_labview::ClusterDataCopier::CopyFromCluster(*clientCall->request.get(), requestCluster);

    grpc::internal::RpcMethod method(methodName, grpc::internal::RpcMethod::SERVER_STREAMING);
    auto reader = grpc_impl::internal::ClientReaderFactory<grpc_labview::LVMessage>::Create<grpc_labview::LVMessage>(client->Channel.get(), method, &clientCall->context, *clientCall->request.get());
    clientCall->_reader = std::shared_ptr<grpc_impl::ClientReader<grpc_labview::LVMessage>>(reader);
    return 0;    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientBeginBidiStreamingCall(grpc_labview::gRPCid* clientId, const char* methodName, const char* requestMessageName, const char* responseMessageName, grpc_labview::gRPCid** callId)
{
    auto client = clientId->CastTo<grpc_labview::LabVIEWgRPCClient>();
    if (!client)
    {
        return -1;
    }
    auto requestMetadata = client->FindMetadata(requestMessageName);
    if (requestMetadata == nullptr)
    {
        return -2;
    }
    auto responseMetadata = client->FindMetadata(responseMessageName);
    if (responseMetadata == nullptr)
    {
        return -3;
    }

    auto clientCall = new grpc_labview::BidiStreamingClientCall();
    *callId = clientCall;
    clientCall->request = std::make_shared<grpc_labview::LVMessage>(requestMetadata);
    clientCall->response = std::make_shared<grpc_labview::LVMessage>(responseMetadata);

    grpc::internal::RpcMethod method(methodName, grpc::internal::RpcMethod::BIDI_STREAMING);
    auto readerWriter = grpc_impl::internal::ClientReaderWriterFactory<grpc_labview::LVMessage, grpc_labview::LVMessage>::Create(client->Channel.get(), method, &clientCall->context);
    clientCall->_readerWriter = std::shared_ptr<grpc_impl::ClientReaderWriterInterface<grpc_labview::LVMessage, grpc_labview::LVMessage>>(readerWriter);
    return 0;    
}
