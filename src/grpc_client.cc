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

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    ClientCall::~ClientCall()
    {        
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ClientCall::Finish()
    {        
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    ServerStreamingClientCall::~ServerStreamingClientCall()
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ServerStreamingClientCall::Finish()
    {
        _status = _reader->Finish();
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool ServerStreamingClientCall::Read(LVMessage* message)
    {
        bool result = _reader->Read(message);
        return result;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    ClientStreamingClientCall::~ClientStreamingClientCall()
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ClientStreamingClientCall::Finish()
    {
        WritesComplete();
        _status = _writer->Finish();
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ClientStreamingClientCall::WritesComplete()
    {
        if (!_writesComplete)
        {
            _writesComplete = true;
           _writer->WritesDone();
        }
    }
    
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool ClientStreamingClientCall::Write(LVMessage* message)
    {
        return _writer->Write(*message);        
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    BidiStreamingClientCall::~BidiStreamingClientCall()
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void BidiStreamingClientCall::Finish()
    {
        WritesComplete();
        _status = _readerWriter->Finish();
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void BidiStreamingClientCall::WritesComplete()
    {
        if (!_writesComplete)
        {
            _writesComplete = true;
           _readerWriter->WritesDone();
        }
    }
    
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool BidiStreamingClientCall::Read(LVMessage* message)
    {
        return _readerWriter->Read(message);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool BidiStreamingClientCall::Write(LVMessage* message)
    {
        return _readerWriter->Write(*message);
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
    clientCall->_occurrence = *occurrence;
    clientCall->_request = std::make_shared<grpc_labview::LVMessage>(requestMetadata);
    clientCall->_response = std::make_shared<grpc_labview::LVMessage>(responseMetadata);

    grpc_labview::ClusterDataCopier::CopyFromCluster(*clientCall->_request.get(), requestCluster);

    clientCall->_runFuture = std::async(
        std::launch::async, 
        [=]() 
        {
            grpc::internal::RpcMethod method(methodName, grpc::internal::RpcMethod::NORMAL_RPC);
            clientCall->_status = grpc::internal::BlockingUnaryCall(client->Channel.get(), method, &clientCall->_context, *clientCall->_request.get(), clientCall->_response.get());
            grpc_labview::SignalOccurrence(clientCall->_occurrence);
            return 0;
        });
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t CompleteClientUnaryCall2(grpc_labview::gRPCid* callId, int8_t* responseCluster, grpc_labview::LStrHandle* errorMessage, grpc_labview::AnyCluster* errorDetailsCluster)
{
    auto call = callId->CastTo<grpc_labview::ClientCall>();
    if (!call)
    {
        return -1;
    }

    int32_t result = 0;
    if (call->_status.ok())
    {
        grpc_labview::ClusterDataCopier::CopyToCluster(*call->_response.get(), responseCluster);
    }
    else
    {
        result = -(1000 + call->_status.error_code());
        if (errorMessage != nullptr)
        {
            grpc_labview::SetLVString(errorMessage, call->_status.error_message());
        }
        if (errorDetailsCluster != nullptr)
        {
        }
    }
    delete call;
    return result;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t CompleteClientUnaryCall(grpc_labview::gRPCid* callId, int8_t* responseCluster)
{
    return CompleteClientUnaryCall2(callId, responseCluster, nullptr, nullptr);
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
    clientCall->_request = std::make_shared<grpc_labview::LVMessage>(requestMetadata);
    clientCall->_response = std::make_shared<grpc_labview::LVMessage>(responseMetadata);

    grpc::internal::RpcMethod method(methodName, grpc::internal::RpcMethod::CLIENT_STREAMING);
    auto writer = grpc_impl::internal::ClientWriterFactory<grpc_labview::LVMessage>::Create(client->Channel.get(), method, &clientCall->_context, clientCall->_response.get());
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
    clientCall->_request = std::make_shared<grpc_labview::LVMessage>(requestMetadata);
    clientCall->_response = std::make_shared<grpc_labview::LVMessage>(responseMetadata);

    grpc_labview::ClusterDataCopier::CopyFromCluster(*clientCall->_request.get(), requestCluster);

    grpc::internal::RpcMethod method(methodName, grpc::internal::RpcMethod::SERVER_STREAMING);
    auto reader = grpc_impl::internal::ClientReaderFactory<grpc_labview::LVMessage>::Create<grpc_labview::LVMessage>(client->Channel.get(), method, &clientCall->_context, *clientCall->_request.get());
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
    clientCall->_request = std::make_shared<grpc_labview::LVMessage>(requestMetadata);
    clientCall->_response = std::make_shared<grpc_labview::LVMessage>(responseMetadata);

    grpc::internal::RpcMethod method(methodName, grpc::internal::RpcMethod::BIDI_STREAMING);
    auto readerWriter = grpc_impl::internal::ClientReaderWriterFactory<grpc_labview::LVMessage, grpc_labview::LVMessage>::Create(client->Channel.get(), method, &clientCall->_context);
    clientCall->_readerWriter = std::shared_ptr<grpc_impl::ClientReaderWriterInterface<grpc_labview::LVMessage, grpc_labview::LVMessage>>(readerWriter);
    return 0;    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientBeginReadFromStream(grpc_labview::gRPCid* callId, grpc_labview::MagicCookie* occurrencePtr)
{
    auto reader = callId->CastTo<grpc_labview::StreamReader>();
    auto call = callId->CastTo<grpc_labview::ClientCall>();
    auto occurrence = *occurrencePtr;

    reader->_readFuture = std::async(
        std::launch::async, 
        [=]() 
        {
            call->_response->Clear();
            auto result = reader->Read(call->_response.get());
            grpc_labview::SignalOccurrence(occurrence);
            return result;
        });

    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientCompleteReadFromStream(grpc_labview::gRPCid* callId, int* success, int8_t* responseCluster)
{
    auto reader = callId->CastTo<grpc_labview::StreamReader>();
    auto call = callId->CastTo<grpc_labview::ClientCall>();
    if (!call)
    {
        return -1;
    }

    *success = reader->_readFuture.get();
    if (reader->_readFuture.get())
    {
        grpc_labview::ClusterDataCopier::CopyToCluster(*call->_response.get(), responseCluster);
    }
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientWriteToStream(grpc_labview::gRPCid* callId, int8_t* requestCluster, int* success)
{
    auto writer = callId->CastTo<grpc_labview::StreamWriter>();
    if (!writer)
    {
        return -1;
    }
    auto clientCall = callId->CastTo<grpc_labview::ClientCall>();
    if (!clientCall)
    {
        return -1;
    }
    grpc_labview::ClusterDataCopier::CopyFromCluster(*clientCall->_request.get(), requestCluster);
    *success = writer->Write(clientCall->_request.get());
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientWritesComplete(grpc_labview::gRPCid* callId)
{
    auto writer = callId->CastTo<grpc_labview::StreamWriter>();
    if (!writer)
    {
        return -1;
    }
    writer->WritesComplete();
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientCompleteClientStreamingCall(grpc_labview::gRPCid* callId, int8_t* responseCluster, grpc_labview::LStrHandle* errorMessage, grpc_labview::AnyCluster* errorDetailsCluster)
{
    auto call = callId->CastTo<grpc_labview::ClientCall>();
    if (!call)
    {
        return -1;
    }
    call->Finish();
    if (call->_status.ok())
    {
        grpc_labview::ClusterDataCopier::CopyToCluster(*call->_response.get(), responseCluster);
    }
    delete call;
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientCompleteStreamingCall(grpc_labview::gRPCid* callId, grpc_labview::LStrHandle* errorMessage, grpc_labview::AnyCluster* errorDetailsCluster)
{
    auto call = callId->CastTo<grpc_labview::ClientCall>();
    if (!call)
    {
        return -1;
    }
    call->Finish();   
    delete call;
    return 0;
}
