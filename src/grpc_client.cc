//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_client.h>
#include <lv_interop.h>
#include <lv_message.h>
#include <cluster_copier.h>
#include <grpcpp/impl/codegen/client_context.h>
#include <grpcpp/impl/codegen/client_unary_call.h>
#include <ctime>
#include <chrono>
#include <token_manager.h>

grpc_labview::TokenManager<grpc_labview::gRPCid> gClientTokenManager;
typedef grpc_labview::TokenManager<grpc_labview::gRPCid>::token_type grpcApiClientToken;

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
    ClientCall::ClientCall(int32_t timeoutMs)
    {
        if (timeoutMs >= 0)
        {
            auto deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(timeoutMs);
            this->_context.set_deadline(deadline);
        }
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
    void ClientCall::Cancel()
    {
        _context.TryCancel();
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

int32_t ClientCleanUpProc(grpc_labview::gRPCid* clientId);

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t CreateClient(const char* address, const char* certificatePath, grpcApiClientToken* clientTokenOut)
{
    grpc_labview::InitCallbacks();

    auto client = new grpc_labview::LabVIEWgRPCClient();
    client->Connect(address, certificatePath);
    *clientTokenOut = gClientTokenManager.CreateTokenForPtr(client);
    grpc_labview::RegisterCleanupProc(ClientCleanUpProc, client);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int32_t CloseClient(grpc_labview::LabVIEWgRPCClient* client)
{
    if (!client)
    {
        return -1;
    }

    grpc_labview::DeregisterCleanupProc(ClientCleanUpProc, client);
    return 0;
}

LIBRARY_EXPORT int32_t CloseClient(grpcApiClientToken clientToken)
{
    auto client = gClientTokenManager.GetPtrForToken<grpc_labview::LabVIEWgRPCClient>(clientToken);
    if (!client)
    {
        return -1;
    }

    CloseClient(client.get());
    gClientTokenManager.DestroyTokenForPtr(clientToken);
    return 0;
}

int32_t ClientCleanUpProc(grpc_labview::gRPCid* clientId)
{
    auto client = clientId->CastTo<grpc_labview::LabVIEWgRPCClient>();
    if (!client)
    {
        return -1;
    }

    for (auto activeClientCall = client->ActiveClientCalls.begin(); activeClientCall != client->ActiveClientCalls.end(); activeClientCall++)
    {
        (*activeClientCall)->_context.TryCancel();
    }
    return CloseClient(client);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientUnaryCall(
    grpcApiClientToken clientToken,
    grpc_labview::MagicCookie* occurrence,
    const char* methodName,
    const char* requestMessageName,
    const char* responseMessageName,
    int8_t* requestCluster,
    grpcApiClientToken* callIdOut,
    int32_t timeoutMs)
{
    int32_t status = 0;
    auto client = gClientTokenManager.GetPtrForToken<grpc_labview::LabVIEWgRPCClient>(clientToken);
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

    auto clientCall = new grpc_labview::ClientCall(timeoutMs);
    *callIdOut = gClientTokenManager.CreateTokenForPtr(clientCall);
    clientCall->_client = client.get();
    clientCall->_methodName = methodName;
    clientCall->_occurrence = *occurrence;
    clientCall->_request = std::make_shared<grpc_labview::LVMessage>(requestMetadata);
    clientCall->_response = std::make_shared<grpc_labview::LVMessage>(responseMetadata);

    grpc_labview::ClusterDataCopier::CopyFromCluster(*clientCall->_request.get(), requestCluster);   

    clientCall->_runFuture = std::async(
        std::launch::async, 
        [clientCall]() 
        {
            grpc::internal::RpcMethod method(clientCall->_methodName.c_str(), grpc::internal::RpcMethod::NORMAL_RPC);
            clientCall->_status = grpc::internal::BlockingUnaryCall(clientCall->_client->Channel.get(), method, &clientCall->_context, *clientCall->_request.get(), clientCall->_response.get());
            grpc_labview::SignalOccurrence(clientCall->_occurrence);
            return 0;
        });

    client->ActiveClientCalls.push_back(clientCall);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t CompleteClientUnaryCall2(
    grpcApiClientToken callId,
    int8_t* responseCluster,
    grpc_labview::LStrHandle* errorMessage,
    grpc_labview::AnyCluster* errorDetailsCluster)
{
    auto call = gClientTokenManager.GetPtrForToken<grpc_labview::ClientCall>(callId);
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
    call->_client->ActiveClientCalls.remove(call.get());
    return result;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t CompleteClientUnaryCall(grpcApiClientToken callId, int8_t* responseCluster)
{
    return CompleteClientUnaryCall2(callId, responseCluster, nullptr, nullptr);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientBeginClientStreamingCall(
    grpcApiClientToken clientId,
    const char* methodName,
    const char* requestMessageName,
    const char* responseMessageName,
    grpcApiClientToken* callIdOut,
    int32_t timeoutMs)
{
    auto client = gClientTokenManager.GetPtrForToken<grpc_labview::LabVIEWgRPCClient>(clientId);
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

    auto clientCall = new grpc_labview::ClientStreamingClientCall(timeoutMs);
    *callIdOut = gClientTokenManager.CreateTokenForPtr(clientCall);
    clientCall->_client = client.get();
    clientCall->_request = std::make_shared<grpc_labview::LVMessage>(requestMetadata);
    clientCall->_response = std::make_shared<grpc_labview::LVMessage>(responseMetadata);

    grpc::internal::RpcMethod method(methodName, grpc::internal::RpcMethod::CLIENT_STREAMING);
    auto writer = grpc::internal::ClientWriterFactory<grpc_labview::LVMessage>::Create(client->Channel.get(), method, &clientCall->_context, clientCall->_response.get());
    clientCall->_writer = std::shared_ptr<grpc::ClientWriterInterface<grpc_labview::LVMessage>>(writer);

    client->ActiveClientCalls.push_back(clientCall);
    return 0;    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientBeginServerStreamingCall(
    grpcApiClientToken clientId,
    const char* methodName,
    const char* requestMessageName,
    const char* responseMessageName,
    int8_t* requestCluster,
    grpcApiClientToken* callIdOut,
    int32_t timeoutMs)
{    
    auto client = gClientTokenManager.GetPtrForToken<grpc_labview::LabVIEWgRPCClient>(clientId);
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

    auto clientCall = new grpc_labview::ServerStreamingClientCall(timeoutMs);
    *callIdOut = gClientTokenManager.CreateTokenForPtr(clientCall);
    clientCall->_client = client.get();
    clientCall->_request = std::make_shared<grpc_labview::LVMessage>(requestMetadata);
    clientCall->_response = std::make_shared<grpc_labview::LVMessage>(responseMetadata);

    grpc_labview::ClusterDataCopier::CopyFromCluster(*clientCall->_request.get(), requestCluster);

    grpc::internal::RpcMethod method(methodName, grpc::internal::RpcMethod::SERVER_STREAMING);
    auto reader = grpc::internal::ClientReaderFactory<grpc_labview::LVMessage>::Create<grpc_labview::LVMessage>(client->Channel.get(), method, &clientCall->_context, *clientCall->_request.get());
    clientCall->_reader = std::shared_ptr<grpc::ClientReader<grpc_labview::LVMessage>>(reader);

    client->ActiveClientCalls.push_back(clientCall);
    return 0;    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientBeginBidiStreamingCall(
    grpcApiClientToken clientId,
    const char* methodName,
    const char* requestMessageName,
    const char* responseMessageName,
    grpcApiClientToken* callIdOut,
    int32_t timeoutMs)
{
    auto client = gClientTokenManager.GetPtrForToken<grpc_labview::LabVIEWgRPCClient>(clientId);
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

    auto clientCall = new grpc_labview::BidiStreamingClientCall(timeoutMs);
    *callIdOut = gClientTokenManager.CreateTokenForPtr(clientCall);
    clientCall->_client = client.get();
    clientCall->_request = std::make_shared<grpc_labview::LVMessage>(requestMetadata);
    clientCall->_response = std::make_shared<grpc_labview::LVMessage>(responseMetadata);

    grpc::internal::RpcMethod method(methodName, grpc::internal::RpcMethod::BIDI_STREAMING);
    auto readerWriter = grpc::internal::ClientReaderWriterFactory<grpc_labview::LVMessage, grpc_labview::LVMessage>::Create(client->Channel.get(), method, &clientCall->_context);
    clientCall->_readerWriter = std::shared_ptr<grpc::ClientReaderWriterInterface<grpc_labview::LVMessage, grpc_labview::LVMessage>>(readerWriter);

    client->ActiveClientCalls.push_back(clientCall);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientBeginReadFromStream(grpcApiClientToken callId, grpc_labview::MagicCookie* occurrencePtr)
{
    auto reader = gClientTokenManager.GetPtrForToken<grpc_labview::StreamReader>(callId);
    if (!reader)
    {
        return -1;
    }
    auto call = gClientTokenManager.GetPtrForToken<grpc_labview::ClientCall>(callId);
    if (!call)
    {
        return -2;
    }

    auto occurrence = *occurrencePtr;

    reader->_readFuture = std::async(
        std::launch::async, 
        [call, reader, occurrence]() 
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
LIBRARY_EXPORT int32_t ClientCompleteReadFromStream(grpcApiClientToken callId, int* success, int8_t* responseCluster)
{
    auto reader = gClientTokenManager.GetPtrForToken<grpc_labview::StreamReader>(callId);
    if (!reader)
    {
        return -1;
    }

    auto call = gClientTokenManager.GetPtrForToken<grpc_labview::ClientCall>(callId);
    if (!call)
    {
        return -2;
    }

    reader->_readFuture.wait();
    *success = reader->_readFuture.get();
    if (*success)
    {
        grpc_labview::ClusterDataCopier::CopyToCluster(*call->_response.get(), responseCluster);
    }
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientWriteToStream(grpcApiClientToken callId, int8_t* requestCluster, int* success)
{
    auto writer = gClientTokenManager.GetPtrForToken<grpc_labview::StreamWriter>(callId);
    if (!writer)
    {
        return -1;
    }

    auto clientCall = gClientTokenManager.GetPtrForToken<grpc_labview::ClientCall>(callId);
    if (!clientCall)
    {
        return -2;
    }
    grpc_labview::ClusterDataCopier::CopyFromCluster(*clientCall->_request.get(), requestCluster);
    *success = writer->Write(clientCall->_request.get());
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientWritesComplete(grpcApiClientToken callId)
{
    auto writer = gClientTokenManager.GetPtrForToken<grpc_labview::StreamWriter>(callId);
    if (!writer)
    {
        return -1;
    }
    writer->WritesComplete();
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t FinishClientCompleteClientStreamingCall(
    grpcApiClientToken callId,
    int8_t* responseCluster,
    grpc_labview::LStrHandle* errorMessage,
    grpc_labview::AnyCluster* errorDetailsCluster)
{    
    auto call = gClientTokenManager.GetPtrForToken<grpc_labview::ClientCall>(callId);
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

    call->_client->ActiveClientCalls.remove(call.get());
    gClientTokenManager.DestroyTokenForPtr(callId);
    return result;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientCompleteClientStreamingCall(grpcApiClientToken callId, grpc_labview::MagicCookie* occurrencePtr)
{
    auto call = gClientTokenManager.GetPtrForToken<grpc_labview::ClientCall>(callId);
    if (!call)
    {
        return -1;
    }
    auto occurrence = *occurrencePtr;
    call->_runFuture = std::async(
        std::launch::async,
        [call, occurrence]()
        {
            call->Finish();
            grpc_labview::SignalOccurrence(occurrence);
            return 0;
        });
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientCompleteStreamingCall(
    grpcApiClientToken callId,
    grpc_labview::LStrHandle* errorMessage,
    grpc_labview::AnyCluster* errorDetailsCluster)
{
    auto call = gClientTokenManager.GetPtrForToken<grpc_labview::ClientCall>(callId);
    if (!call)
    {
        return -1;
    }

    // We've already got a shared_ptr for this token, so calling DestroyToken now
    // will just prevent any other API calls from grabbing the pointer.
    gClientTokenManager.DestroyTokenForPtr(callId);

    call->Finish();
    int32_t result = 0;   
    if (!call->_status.ok())
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

    call->_client->ActiveClientCalls.remove(call.get());
    return result;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientCancelCall(
    grpcApiClientToken callId,
    grpc_labview::LStrHandle* errorMessage,
    grpc_labview::AnyCluster* errorDetailsCluster)
{
    auto call = gClientTokenManager.GetPtrForToken<grpc_labview::ClientCall>(callId);
    if (!call)
    {
        return -1;
    }

    // We've already got a shared_ptr for this token, so calling DestroyToken now
    // will just prevent any other API calls from grabbing the pointer.
    gClientTokenManager.DestroyTokenForPtr(callId);
    call->Cancel();

    int32_t result = 0;
    if (!call->_status.ok())
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

    call->_client->ActiveClientCalls.remove(call.get());
    return result;
}