//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_client.h>
#include <lv_interop.h>
#include <lv_message.h>
#include <lv_message_efficient.h>
#include <cluster_copier.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/impl/codegen/client_context.h>
#include <grpcpp/impl/codegen/client_unary_call.h>
#include <grpcpp/support/channel_arguments.h>
#include <ctime>
#include <chrono>
#include <feature_toggles.h>

namespace grpc_labview
{
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    LabVIEWgRPCClient::LabVIEWgRPCClient()
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LabVIEWgRPCClient::Connect(const char *address, const std::string &certificatePath)
    {
        std::shared_ptr<grpc::ChannelCredentials> creds;
        if (!certificatePath.empty())
        {
            std::string cacert = read_keycert(certificatePath);
            grpc::SslCredentialsOptions ssl_opts;
            ssl_opts.pem_root_certs = cacert;
            creds = grpc::SslCredentials(ssl_opts);
        }
        else
        {
            creds = grpc::InsecureChannelCredentials();
        }
        grpc::ChannelArguments args;
        args.SetMaxReceiveMessageSize(-1);
        args.SetMaxSendMessageSize(-1);
        Channel = grpc::CreateCustomChannel(address, creds, args);
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
        _context.get()->Cancel();
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
    bool ServerStreamingClientCall::Read(LVMessage *message)
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
    bool ClientStreamingClientCall::Write(LVMessage *message)
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
    bool BidiStreamingClientCall::Read(LVMessage *message)
    {
        return _readerWriter->Read(message);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool BidiStreamingClientCall::Write(LVMessage *message)
    {
        return _readerWriter->Write(*message);
    }

    void ClientContext::set_deadline(int32_t timeoutMs)
    {
        auto deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(timeoutMs);
        gRPCClientContext.set_deadline(deadline);
    }

    void ClientContext::Cancel()
    {
        gRPCClientContext.TryCancel();
    }
}

int32_t ClientCleanUpProc(grpc_labview::gRPCid *clientId);
void CheckActiveAndSignalOccurenceForClientCall(grpc_labview::ClientCall *clientCall);

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t CreateClient(const char *address, const char *certificatePath, grpc_labview::gRPCid **clientId)
{
    grpc_labview::InitCallbacks();

    auto client = new grpc_labview::LabVIEWgRPCClient();
    client->Connect(address, certificatePath);
    *clientId = grpc_labview::gPointerManager.RegisterPointer(client);
    grpc_labview::RegisterCleanupProc(ClientCleanUpProc, client);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
int32_t CloseClient(grpc_labview::LabVIEWgRPCClient *client)
{
    if (!client)
    {
        return -1;
    }

    grpc_labview::DeregisterCleanupProc(ClientCleanUpProc, client);
    return 0;
}

// Signal a lv occurence for an active client call from async c++ thread
void CheckActiveAndSignalOccurenceForClientCall(grpc_labview::ClientCall *clientCall)
{
    if (clientCall == nullptr)
    {
        return;
    }
    std::unique_lock<std::mutex> lock(clientCall->_client->clientLock);
    if (clientCall->_client->ActiveClientCalls[clientCall])
    {
        grpc_labview::SignalOccurrence(clientCall->_occurrence);
    }
}

LIBRARY_EXPORT int32_t CloseClient(grpc_labview::gRPCid *clientId)
{
    auto client = clientId->CastTo<grpc_labview::LabVIEWgRPCClient>();
    if (!client)
    {
        return -1;
    }

    CloseClient(client.get());
    grpc_labview::gPointerManager.UnregisterPointer(clientId);
    return 0;
}

int32_t ClientCleanUpProc(grpc_labview::gRPCid *clientId)
{
    auto client = clientId->CastTo<grpc_labview::LabVIEWgRPCClient>();
    if (!client)
    {
        return -1;
    }
    std::unique_lock<std::mutex> lock(client->clientLock);
    for (auto activeClientCall = client->ActiveClientCalls.begin(); activeClientCall != client->ActiveClientCalls.end(); activeClientCall++)
    {
        activeClientCall->first->Cancel();
    }
    client->ActiveClientCalls.clear();
    lock.unlock();
    return CloseClient(client.get());
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t CreateClientContext(grpc_labview::gRPCid **contextId)
{
    auto clientContext = std::make_shared<grpc_labview::ClientContext>();
    *contextId = grpc_labview::gPointerManager.RegisterPointer(clientContext);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t CloseClientContext(grpc_labview::gRPCid *contextId)
{
    auto context = contextId->CastTo<grpc_labview::ClientContext>();
    if (!context)
    {
        return -1;
    }
    grpc_labview::gPointerManager.UnregisterPointer(contextId);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientUnaryCall2(
    grpc_labview::gRPCid *clientId,
    grpc_labview::MagicCookie *occurrence,
    const char *methodName,
    const char *requestMessageName,
    const char *responseMessageName,
    int8_t *requestCluster,
    grpc_labview::gRPCid **callId,
    int32_t timeoutMs,
    grpc_labview::gRPCid *contextId,
    int8_t *responseCluster)
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

    auto clientContext = contextId->CastTo<grpc_labview::ClientContext>();
    if (!clientContext)
    {
        clientContext = std::make_shared<grpc_labview::ClientContext>();
    }
    if (timeoutMs > 0)
    {
        clientContext->set_deadline(timeoutMs);
    }

    auto clientCall = new grpc_labview::ClientCall();
    std::unique_lock<std::mutex> lock(client->clientLock);
    client->ActiveClientCalls[clientCall] = true;
    lock.unlock();
    *callId = grpc_labview::gPointerManager.RegisterPointer(clientCall);
    clientCall->_client = client;
    clientCall->_methodName = methodName;
    clientCall->_occurrence = *occurrence;
    clientCall->_context = clientContext;

    auto featureConfig = grpc_labview::FeatureConfig::getInstance();
    if (featureConfig.isFeatureEnabled("EfficientMessageCopy") && responseCluster != nullptr)
    {
        clientCall->_useLVEfficientMessage = true;
    }

    if (clientCall->_useLVEfficientMessage)
    {
        clientCall->_request = std::make_shared<grpc_labview::LVMessageEfficient>(requestMetadata);
        clientCall->_response = std::make_shared<grpc_labview::LVMessageEfficient>(responseMetadata);
        clientCall->_request->SetLVClusterHandle(reinterpret_cast<const char *>(requestCluster));
        clientCall->_response->SetLVClusterHandle(reinterpret_cast<const char *>(responseCluster));
    }
    else
    {
        clientCall->_request = std::make_shared<grpc_labview::LVMessage>(requestMetadata);
        clientCall->_response = std::make_shared<grpc_labview::LVMessage>(responseMetadata);
    }

    try
    {
        grpc_labview::ClusterDataCopier::CopyFromCluster(*clientCall->_request.get(), requestCluster);
    }
    catch (grpc_labview::InvalidEnumValueException &e)
    {
        return e.code;
    }

    clientCall->_runFuture = std::async(
        std::launch::async,
        [clientCall]()
        {
            grpc::internal::RpcMethod method(clientCall->_methodName.c_str(), grpc::internal::RpcMethod::NORMAL_RPC);
            clientCall->_status = grpc::internal::BlockingUnaryCall(clientCall->_client->Channel.get(), method, &(clientCall->_context.get()->gRPCClientContext), *clientCall->_request.get(), clientCall->_response.get());
            CheckActiveAndSignalOccurenceForClientCall(clientCall);
            return 0;
        });
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientUnaryCall2WithoutOccurrence(
    grpc_labview::gRPCid *clientId,
    const char *methodName,
    const char *requestMessageName,
    const char *responseMessageName,
    int8_t *requestCluster,
    grpc_labview::gRPCid **callId,
    int32_t timeoutMs,
    grpc_labview::gRPCid *contextId,
    int8_t *responseCluster)
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

    auto clientContext = contextId->CastTo<grpc_labview::ClientContext>();
    if (!clientContext)
    {
        clientContext = std::make_shared<grpc_labview::ClientContext>();
    }
    if (timeoutMs > 0)
    {
        clientContext->set_deadline(timeoutMs);
    }

    auto clientCall = new grpc_labview::ClientCall();
    std::unique_lock<std::mutex> lock(client->clientLock);
    client->ActiveClientCalls[clientCall] = true;
    lock.unlock();
    *callId = grpc_labview::gPointerManager.RegisterPointer(clientCall);
    clientCall->_client = client;
    clientCall->_methodName = methodName;
    clientCall->_context = clientContext;

    auto featureConfig = grpc_labview::FeatureConfig::getInstance();
    if (featureConfig.isFeatureEnabled("EfficientMessageCopy") && responseCluster != nullptr)
    {
        clientCall->_useLVEfficientMessage = true;
    }

    if (clientCall->_useLVEfficientMessage)
    {
        clientCall->_request = std::make_shared<grpc_labview::LVMessageEfficient>(requestMetadata);
        clientCall->_response = std::make_shared<grpc_labview::LVMessageEfficient>(responseMetadata);
        clientCall->_request->SetLVClusterHandle(reinterpret_cast<const char *>(requestCluster));
        clientCall->_response->SetLVClusterHandle(reinterpret_cast<const char *>(responseCluster));
    }
    else
    {
        clientCall->_request = std::make_shared<grpc_labview::LVMessage>(requestMetadata);
        clientCall->_response = std::make_shared<grpc_labview::LVMessage>(responseMetadata);
    }

    try
    {
        grpc_labview::ClusterDataCopier::CopyFromCluster(*clientCall->_request.get(), requestCluster);
    }
    catch (grpc_labview::InvalidEnumValueException &e)
    {
        return e.code;
    }

    clientCall->_runFuture = std::async(
        std::launch::async,
        [clientCall]()
        {
            grpc::internal::RpcMethod method(clientCall->_methodName.c_str(), grpc::internal::RpcMethod::NORMAL_RPC);
            clientCall->_status = grpc::internal::BlockingUnaryCall(clientCall->_client->Channel.get(), method, &(clientCall->_context.get()->gRPCClientContext), *clientCall->_request.get(), clientCall->_response.get());
            return 0;
        });

    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientUnaryCall(
    grpc_labview::gRPCid *clientId,
    grpc_labview::MagicCookie *occurrence,
    const char *methodName,
    const char *requestMessageName,
    const char *responseMessageName,
    int8_t *requestCluster,
    grpc_labview::gRPCid **callId,
    int32_t timeoutMs,
    grpc_labview::gRPCid *contextId)
{
    return ClientUnaryCall2(clientId, occurrence, methodName, requestMessageName, responseMessageName, requestCluster, callId, timeoutMs, contextId, nullptr);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t CompleteClientUnaryCall2(
    grpc_labview::gRPCid *callId,
    int8_t *responseCluster,
    grpc_labview::LStrHandle *errorMessage,
    grpc_labview::AnyCluster *errorDetailsCluster)
{
    auto clientCall = callId->CastTo<grpc_labview::ClientCall>();
    if (!clientCall)
    {
        return -1;
    }

    grpc_labview::gPointerManager.UnregisterPointer(callId);

    int32_t result = 0;
    if (clientCall->_status.ok())
    {
        if (!clientCall->_useLVEfficientMessage)
        {
            try
            {
                grpc_labview::ClusterDataCopier::CopyToCluster(*clientCall->_response.get(), responseCluster);
            }
            catch (grpc_labview::InvalidEnumValueException &e)
            {
                if (errorMessage != nullptr)
                {
                    grpc_labview::SetLVString(errorMessage, e.what());
                }
                return e.code;
            }
        }
    }
    else
    {
        result = -(1000 + clientCall->_status.error_code());
        if (errorMessage != nullptr)
        {
            grpc_labview::SetLVString(errorMessage, clientCall->_status.error_message());
        }
        if (errorDetailsCluster != nullptr)
        {
        }
    }
    std::unique_lock<std::mutex> lock(clientCall->_client->clientLock);
    auto call = clientCall->_client->ActiveClientCalls.find(clientCall.get());
    if (call != clientCall->_client->ActiveClientCalls.end())
    {
        clientCall->_client->ActiveClientCalls.erase(call);
    }
    lock.unlock();
    return result;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t CompleteClientUnaryCall2WithoutOccurrence(
    grpc_labview::gRPCid *callId,
    int8_t *responseCluster,
    grpc_labview::LStrHandle *errorMessage,
    grpc_labview::AnyCluster *errorDetailsCluster)
{
    auto clientCall = callId->CastTo<grpc_labview::ClientCall>();
    if (!clientCall)
    {
        return -1;
    }

    grpc_labview::gPointerManager.UnregisterPointer(callId);

    int32_t result = 0;
    if (clientCall->_status.ok())
    {
        clientCall->_runFuture.wait();
        if (!clientCall->_useLVEfficientMessage)
        {
            try
            {
                grpc_labview::ClusterDataCopier::CopyToCluster(*clientCall->_response.get(), responseCluster);
            }
            catch (grpc_labview::InvalidEnumValueException &e)
            {
                if (errorMessage != nullptr)
                {
                    grpc_labview::SetLVString(errorMessage, e.what());
                }
                return e.code;
            }
        }
    }
    else
    {
        result = -(1000 + clientCall->_status.error_code());
        if (errorMessage != nullptr)
        {
            grpc_labview::SetLVString(errorMessage, clientCall->_status.error_message());
        }
        if (errorDetailsCluster != nullptr)
        {
        }
    }
    std::unique_lock<std::mutex> lock(clientCall->_client->clientLock);
    auto call = clientCall->_client->ActiveClientCalls.find(clientCall.get());
    if (call != clientCall->_client->ActiveClientCalls.end())
    {
        clientCall->_client->ActiveClientCalls.erase(call);
    }
    lock.unlock();
    return result;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t CompleteClientUnaryCall(grpc_labview::gRPCid *callId, int8_t *responseCluster)
{
    return CompleteClientUnaryCall2(callId, responseCluster, nullptr, nullptr);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientBeginClientStreamingCall(
    grpc_labview::gRPCid *clientId,
    const char *methodName,
    const char *requestMessageName,
    const char *responseMessageName,
    grpc_labview::gRPCid **callId,
    int32_t timeoutMs,
    grpc_labview::gRPCid *contextId)
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

    auto clientContext = contextId->CastTo<grpc_labview::ClientContext>();
    if (!clientContext)
    {
        clientContext = std::make_shared<grpc_labview::ClientContext>();
    }
    if (timeoutMs > 0)
    {
        clientContext->set_deadline(timeoutMs);
    }

    auto clientCall = new grpc_labview::ClientStreamingClientCall();
    std::unique_lock<std::mutex> lock(client->clientLock);
    client->ActiveClientCalls[clientCall] = true;
    lock.unlock();
    *callId = grpc_labview::gPointerManager.RegisterPointer(clientCall);
    clientCall->_client = client;
    clientCall->_request = std::make_shared<grpc_labview::LVMessage>(requestMetadata);
    clientCall->_response = std::make_shared<grpc_labview::LVMessage>(responseMetadata);
    clientCall->_context = clientContext;

    grpc::internal::RpcMethod method(methodName, grpc::internal::RpcMethod::CLIENT_STREAMING);
    auto writer = grpc::internal::ClientWriterFactory<grpc_labview::LVMessage>::Create(client->Channel.get(), method, &(clientCall->_context.get()->gRPCClientContext), clientCall->_response.get());
    clientCall->_writer = std::shared_ptr<grpc::ClientWriterInterface<grpc_labview::LVMessage>>(writer);

    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientBeginServerStreamingCall(
    grpc_labview::gRPCid *clientId,
    const char *methodName,
    const char *requestMessageName,
    const char *responseMessageName,
    int8_t *requestCluster,
    grpc_labview::gRPCid **callId,
    int32_t timeoutMs,
    grpc_labview::gRPCid *contextId)
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

    auto clientContext = contextId->CastTo<grpc_labview::ClientContext>();
    if (!clientContext)
    {
        clientContext = std::make_shared<grpc_labview::ClientContext>();
    }
    if (timeoutMs > 0)
    {
        clientContext->set_deadline(timeoutMs);
    }

    auto clientCall = new grpc_labview::ServerStreamingClientCall();
    std::unique_lock<std::mutex> lock(client->clientLock);
    client->ActiveClientCalls[clientCall] = true;
    lock.unlock();
    *callId = grpc_labview::gPointerManager.RegisterPointer(clientCall);
    clientCall->_client = client;
    clientCall->_request = std::make_shared<grpc_labview::LVMessage>(requestMetadata);
    clientCall->_response = std::make_shared<grpc_labview::LVMessage>(responseMetadata);
    clientCall->_context = clientContext;

    try
    {
        grpc_labview::ClusterDataCopier::CopyFromCluster(*clientCall->_request.get(), requestCluster);
    }
    catch (grpc_labview::InvalidEnumValueException &e)
    {
        return e.code;
    }

    grpc::internal::RpcMethod method(methodName, grpc::internal::RpcMethod::SERVER_STREAMING);
    auto reader = grpc::internal::ClientReaderFactory<grpc_labview::LVMessage>::Create<grpc_labview::LVMessage>(client->Channel.get(), method, &(clientCall->_context.get()->gRPCClientContext), *clientCall->_request.get());
    clientCall->_reader = std::shared_ptr<grpc::ClientReader<grpc_labview::LVMessage>>(reader);

    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientBeginBidiStreamingCall(
    grpc_labview::gRPCid *clientId,
    const char *methodName,
    const char *requestMessageName,
    const char *responseMessageName,
    grpc_labview::gRPCid **callId,
    int32_t timeoutMs,
    grpc_labview::gRPCid *contextId)
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

    auto clientContext = contextId->CastTo<grpc_labview::ClientContext>();
    if (!clientContext)
    {
        clientContext = std::make_shared<grpc_labview::ClientContext>();
    }
    if (timeoutMs > 0)
    {
        clientContext->set_deadline(timeoutMs);
    }

    auto clientCall = new grpc_labview::BidiStreamingClientCall();
    std::unique_lock<std::mutex> lock(client->clientLock);
    client->ActiveClientCalls[clientCall] = true;
    lock.unlock();
    *callId = grpc_labview::gPointerManager.RegisterPointer(clientCall);
    clientCall->_client = client;
    clientCall->_request = std::make_shared<grpc_labview::LVMessage>(requestMetadata);
    clientCall->_response = std::make_shared<grpc_labview::LVMessage>(responseMetadata);
    clientCall->_context = clientContext;

    grpc::internal::RpcMethod method(methodName, grpc::internal::RpcMethod::BIDI_STREAMING);
    auto readerWriter = grpc::internal::ClientReaderWriterFactory<grpc_labview::LVMessage, grpc_labview::LVMessage>::Create(client->Channel.get(), method, &(clientCall->_context.get()->gRPCClientContext));
    clientCall->_readerWriter = std::shared_ptr<grpc::ClientReaderWriterInterface<grpc_labview::LVMessage, grpc_labview::LVMessage>>(readerWriter);

    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientBeginReadFromStreamWithoutOccurrence(grpc_labview::gRPCid *callId)
{
    auto reader = callId->CastTo<grpc_labview::StreamReader>();
    auto call = callId->CastTo<grpc_labview::ClientCall>();

    reader->_readFuture = std::async(
        std::launch::async,
        [call, reader]()
        {
            call->_response->Clear();
            auto result = reader->Read(call->_response.get());
            return result;
        });

    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientBeginReadFromStream(grpc_labview::gRPCid *callId, grpc_labview::MagicCookie *occurrencePtr)
{
    auto reader = callId->CastTo<grpc_labview::StreamReader>();
    auto call = callId->CastTo<grpc_labview::ClientCall>();

    auto occurrence = *occurrencePtr;

    if (!reader || !call)
    {
        grpc_labview::SignalOccurrence(occurrence);
        return -1;
    }

    call->_occurrence = occurrence;
    reader->_readFuture = std::async(
        std::launch::async,
        [call, reader]()
        {
            call->_response->Clear();
            auto result = reader->Read(call->_response.get());
            CheckActiveAndSignalOccurenceForClientCall(call.get());
            return result;
        });

    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientCompleteReadFromStream(grpc_labview::gRPCid *callId, int *success, int8_t *responseCluster)
{
    auto reader = callId->CastTo<grpc_labview::StreamReader>();
    auto call = callId->CastTo<grpc_labview::ClientCall>();
    if (!reader || !call)
    {
        return -1;
    }
    reader->_readFuture.wait();
    *success = reader->_readFuture.get();
    if (*success)
    {
        try
        {
            grpc_labview::ClusterDataCopier::CopyToCluster(*call->_response.get(), responseCluster);
        }
        catch (grpc_labview::InvalidEnumValueException &e)
        {
            return e.code;
        }
    }
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientCompleteReadFromStreamWithoutOccurrence(grpc_labview::gRPCid *callId, int *success, int8_t *responseCluster)
{
    return ClientCompleteReadFromStream(callId, success, responseCluster);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientWriteToStream(grpc_labview::gRPCid *callId, int8_t *requestCluster, int *success)
{
    auto writer = callId->CastTo<grpc_labview::StreamWriter>();
    if (!writer)
    {
        return -1;
    }
    auto clientCall = callId->CastTo<grpc_labview::ClientCall>();
    if (!clientCall)
    {
        return -2;
    }
    try
    {
        grpc_labview::ClusterDataCopier::CopyFromCluster(*clientCall->_request.get(), requestCluster);
    }
    catch (grpc_labview::InvalidEnumValueException &e)
    {
        return e.code;
    }
    *success = writer->Write(clientCall->_request.get());
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientWritesComplete(grpc_labview::gRPCid *callId)
{
    auto writer = callId->CastTo<grpc_labview::StreamWriter>();
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
    grpc_labview::gRPCid *callId,
    int8_t *responseCluster,
    grpc_labview::LStrHandle *errorMessage,
    grpc_labview::AnyCluster *errorDetailsCluster)
{
    auto call = callId->CastTo<grpc_labview::ClientCall>();
    if (!call)
    {
        return -1;
    }
    int32_t result = 0;
    if (call->_status.ok())
    {
        try
        {
            grpc_labview::ClusterDataCopier::CopyToCluster(*call->_response.get(), responseCluster);
        }
        catch (grpc_labview::InvalidEnumValueException &e)
        {
            result = e.code;
            if (errorMessage != nullptr)
            {
                grpc_labview::SetLVString(errorMessage, e.what());
            }
        }
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
    std::unique_lock<std::mutex> lock(call->_client->clientLock);
    auto client_call = call->_client->ActiveClientCalls.find(call.get());
    if (client_call != call->_client->ActiveClientCalls.end())
    {
        call->_client->ActiveClientCalls.erase(client_call);
    }
    lock.unlock();
    grpc_labview::gPointerManager.UnregisterPointer(callId);
    return result;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t FinishClientCompleteClientStreamingCallWithoutOccurrence(
    grpc_labview::gRPCid *callId,
    int8_t *responseCluster,
    grpc_labview::LStrHandle *errorMessage,
    grpc_labview::AnyCluster *errorDetailsCluster)
{
    auto call = callId->CastTo<grpc_labview::ClientCall>();
    if (!call)
    {
        return -1;
    }
    int32_t result = 0;
    if (call->_status.ok())
    {
        call->_runFuture.wait();
        try
        {
            grpc_labview::ClusterDataCopier::CopyToCluster(*call->_response.get(), responseCluster);
        }
        catch (grpc_labview::InvalidEnumValueException &e)
        {
            result = e.code;
            if (errorMessage != nullptr)
            {
                grpc_labview::SetLVString(errorMessage, e.what());
            }
        }
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
    std::unique_lock<std::mutex> lock(call->_client->clientLock);
    auto client_call = call->_client->ActiveClientCalls.find(call.get());
    if (client_call != call->_client->ActiveClientCalls.end())
    {
        call->_client->ActiveClientCalls.erase(client_call);
    }
    lock.unlock();
    grpc_labview::gPointerManager.UnregisterPointer(callId);
    return result;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientCompleteClientStreamingCall(grpc_labview::gRPCid *callId, grpc_labview::MagicCookie *occurrencePtr)
{
    auto call = callId->CastTo<grpc_labview::ClientCall>();
    if (!call)
    {
        return -1;
    }
    call->_occurrence = *occurrencePtr;
    call->_runFuture = std::async(
        std::launch::async,
        [call]()
        {
            call->Finish();
            CheckActiveAndSignalOccurenceForClientCall(call.get());
            return 0;
        });
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientCompleteClientStreamingCallWithoutOccurrence(grpc_labview::gRPCid *callId)
{
    auto call = callId->CastTo<grpc_labview::ClientCall>();
    if (!call)
    {
        return -1;
    }
    call->_runFuture = std::async(
        std::launch::async,
        [call]()
        {
            call->Finish();
            return 0;
        });
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientCompleteStreamingCall(
    grpc_labview::gRPCid *callId,
    grpc_labview::LStrHandle *errorMessage,
    grpc_labview::AnyCluster *errorDetailsCluster)
{
    auto call = callId->CastTo<grpc_labview::ClientCall>();
    if (!call)
    {
        return -1;
    }

    // We've already got a shared_ptr for this token, so calling DestroyToken now
    // will just prevent any other API calls from grabbing the pointer.
    grpc_labview::gPointerManager.UnregisterPointer(callId);

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
    std::unique_lock<std::mutex> lock(call->_client->clientLock);
    auto client_call = call->_client->ActiveClientCalls.find(call.get());
    if (client_call != call->_client->ActiveClientCalls.end())
    {
        call->_client->ActiveClientCalls.erase(client_call);
    }
    lock.unlock();
    return result;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientCancelCallContext(
    grpc_labview::gRPCid *contextId)
{
    auto context = contextId->CastTo<grpc_labview::ClientContext>();
    if (!context)
    {
        return -1;
    }

    context->Cancel();
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientCancelCall(
    grpc_labview::gRPCid *callId,
    grpc_labview::LStrHandle *errorMessage,
    grpc_labview::AnyCluster *errorDetailsCluster)
{
    auto call = callId->CastTo<grpc_labview::ClientCall>();
    if (!call)
    {
        return -1;
    }

    // We've already got a shared_ptr for this token, so calling DestroyToken now
    // will just prevent any other API calls from grabbing the pointer.
    grpc_labview::gPointerManager.UnregisterPointer(callId);
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
    std::unique_lock<std::mutex> lock(call->_client->clientLock);
    auto client_call = call->_client->ActiveClientCalls.find(call.get());
    if (client_call != call->_client->ActiveClientCalls.end())
    {
        call->_client->ActiveClientCalls.erase(client_call);
    }
    lock.unlock();
    return result;
}