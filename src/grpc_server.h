//---------------------------------------------------------------------
// LabVIEW implementation of a gRPC Server
//---------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#ifdef __WIN32__
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

//---------------------------------------------------------------------
// IMPORTANT: lv_serialization_traits.h MUST be included BEFORE grpc headers
// This registers our custom SerializationTraits<LVMessage> specialization
//---------------------------------------------------------------------
#include <lv_serialization_traits.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/impl/codegen/async_generic_service.h>
#include <grpcpp/impl/codegen/completion_queue.h>
#include <grpcpp/impl/codegen/message_allocator.h>
#include <grpcpp/impl/codegen/method_handler.h>
#include <grpcpp/impl/codegen/proto_utils.h>
#include <grpcpp/impl/codegen/server_callback.h>
#include <grpcpp/impl/codegen/server_callback_handlers.h>
#include <grpcpp/impl/codegen/server_context.h>
#include <lv_interop.h>
#include <condition_variable>
#include <future>
#include <map>
#include <event_data.h>
#include <metadata_owner.h>
#include <lv_semaphore.h>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
using grpc::Server;
using grpc::ServerBuilder;
using grpc::Status;

namespace grpc_labview
{
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class LabVIEWgRPCServer;
    class LVMessage;
    class CallData;
    class CallDoneNotificationTag;
    class MessageElementMetadata;
    struct MessageMetadata;

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class ServerStartEventData : public EventData
    {
    public:
        ServerStartEventData();

    public:
        int serverStartStatus;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    struct LVEventData
    {
        LVUserEventRef event;
        std::string requestMetadataName;
        std::string responseMetadataName;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class LabVIEWgRPCServer : public MessageElementMetadataOwner, public gRPCid, public std::enable_shared_from_this<LabVIEWgRPCServer>
    {
    public:
        LabVIEWgRPCServer();
        int Start(std::string address, std::string serverCertificatePath, std::string serverKeyPath);
        void Stop();
        int GetListeningPort();
        void RegisterEvent(std::string eventName, LVUserEventRef reference, std::string requestMessageName, std::string responseMessageName);
        void RegisterGenericMethodEvent(LVUserEventRef item);
        int SendEvent(std::string name, gRPCid* data);

        bool FindEventData(const std::string& name, LVEventData& data);

    private:
        std::mutex _eventMutex;
        std::mutex _lifeCycleMutex;
        std::unique_ptr<Server> _server;
        std::unique_ptr<grpc::ServerCompletionQueue> _cq;
        std::map<std::string, LVEventData> _registeredServerMethods;
        LVUserEventRef _genericMethodEvent;
        std::unique_ptr<grpc::AsyncGenericService> _rpcService;
        std::unique_ptr<std::thread> _runThread;
        CallDoneNotificationTag* _undeliveredCallDoneNotificationTag = nullptr;
        int _listeningPort;

    private:
        void RunServer(std::string address, std::string serverCertificatePath, std::string serverKeyPath, ServerStartEventData* serverStarted);
        void HandleRpcs();
        void RetainUndeliveredCallDoneNotificationTag(CallDoneNotificationTag* tag);
        void ReleaseUndeliveredCallDoneNotificationTag(CallDoneNotificationTag* tag);
        bool FindEventDataLocked(const std::string& name, LVEventData& data);

        friend class CallData;
        friend class CallDoneNotificationTag;

    private:
        static void StaticRunServer(LabVIEWgRPCServer* server, std::string address, std::string serverCertificatePath, std::string serverKeyPath, ServerStartEventData* serverStarted);
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class CallDataBase
    {
    public:
        virtual void Proceed(bool ok) = 0;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class CallData : public CallDataBase, public gRPCid, public std::enable_shared_from_this<CallData>
    {
    public:
        static std::shared_ptr<CallData> Create(std::shared_ptr<LabVIEWgRPCServer>server, grpc::AsyncGenericService* service, grpc::ServerCompletionQueue* cq);

        void Proceed(bool ok) override;
        bool Write(int8_t* cluster);
        void FinishFromLabVIEW();
        void FinishOperationCompleted(bool ok);
        void OnCallDone();
        bool IsCancelled();
        bool ReadNext(int8_t* cluster);
        void SetCallStatusError(std::string errorMessage);
        void SetCallStatusError(grpc::StatusCode statusCode, std::string errorMessage);
        grpc::StatusCode GetCallStatusCode();

    private:
        CallData(std::shared_ptr<LabVIEWgRPCServer> server, grpc::AsyncGenericService* service, grpc::ServerCompletionQueue* cq);
        void FinishWithStatusLocked(grpc::Status status);

    private:
        std::weak_ptr<LabVIEWgRPCServer> _server;
        grpc::AsyncGenericService* _service;
        grpc::ServerCompletionQueue* _cq;
        grpc::GenericServerContext _ctx;
        grpc::GenericServerAsyncReaderWriter _stream;
        grpc::ByteBuffer _rb;
        grpc::Status _callStatus;
        CallDoneNotificationTag* _callDoneNotificationTag;
        bool _cancelled;
        bool _finishedFromLabVIEW;

        std::shared_ptr<LVMessage> _request;
        std::shared_ptr<LVMessage> _response;

        // Lock order: read/outbound operation mutex first, then state mutex.
        // Never wait for completion while holding _stateMutex.
        std::mutex _readMutex;
        std::mutex _outboundOperationMutex;
        std::mutex _stateMutex; // Protects state transitions

        enum class CallState
        {
            WaitingForConnection,
            Connected,
            Finishing,
            Finished
        };
        CallState _callState;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class CallDoneNotificationTag : public CallDataBase
    {
    public:
        CallDoneNotificationTag(std::shared_ptr<CallData> callData, std::shared_ptr<LabVIEWgRPCServer> server);
        void SetCallAccepted();
        void Proceed(bool ok) override;

    private:
        std::weak_ptr<CallData> _callData;
        std::shared_ptr<CallData> _acceptedCallData;
        std::weak_ptr<LabVIEWgRPCServer> _server;
    };

    //---------------------------------------------------------------------
    // Completion queue tag that keeps CallData alive
    //---------------------------------------------------------------------
    class CompletionQueueTag : public CallDataBase
    {
    public:
        static CompletionQueueTag* ForRequestCall(std::shared_ptr<CallData> callData)
        {
            return new CompletionQueueTag(callData, true);
        }

        static CompletionQueueTag* ForFinish(std::shared_ptr<CallData> callData)
        {
            return new CompletionQueueTag(callData, false);
        }

        void Proceed(bool ok) override
        {
            if (_isRequest)
            {
                _callData->Proceed(ok);
            }
            else
            {
                _callData->FinishOperationCompleted(ok);
            }
            delete this;
        }

    private:
        CompletionQueueTag(std::shared_ptr<CallData> callData, bool isRequest)
            : _callData(callData), _isRequest(isRequest) {}

        std::shared_ptr<CallData> _callData;
        bool _isRequest;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class ReadNextTag : public CallDataBase
    {
    public:
        ReadNextTag(std::shared_ptr<CallData> callData);
        void Proceed(bool ok) override;
        bool Wait();

    private:
        Semaphore _readCompleteSemaphore;
        std::shared_ptr<CallData> _callData;
        bool _success;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class WriteNextTag : public CallDataBase
    {
    public:
        WriteNextTag(std::shared_ptr<CallData> callData);
        void Proceed(bool ok) override;
        bool Wait();

    private:
        Semaphore _writeCompleteSemaphore;
        std::shared_ptr<CallData> _callData;
        bool _success;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    struct LVRegistrationRequest
    {
        LStrHandle eventName;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    struct LVServerEvent
    {
        LStrHandle eventData;
        int32_t serverId;
        int32_t status;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    #ifdef _PS_4
    #pragma pack (push, 1)
    #endif
    struct GeneralMethodEventData
    {
        LStrHandle methodName;
        gRPCid* methodData;
    };
    #ifdef _PS_4
    #pragma pack (pop)
    #endif

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    int OccurServerEvent(LVUserEventRef event, gRPCid* data);
    int OccurServerEvent(LVUserEventRef event, gRPCid* data, std::string eventMethodName);
    std::string read_keycert(const std::string &filename);

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class ProtoDescriptorString {
        // Static members
        static ProtoDescriptorString* m_instance;
        static std::mutex m_mutex;

        // Non static members
        std::string m_descriptor;
        int m_refcount = 0; // Not a normal refcount. Its counts the number of time we set the descriptor string.

        // Default private constructor to prevent instantiation
        ProtoDescriptorString() = default;

        // Delete copy constructor and assignment operator
        ProtoDescriptorString(const ProtoDescriptorString&) = delete;
        ProtoDescriptorString& operator=(const ProtoDescriptorString&) = delete;
    public:
        // Return the static class instance
        static ProtoDescriptorString* getInstance();

        // Set the descriptor string
        void setDescriptor(std::string);

        // Get the descriptor string
        std::string getDescriptor();

        // Delete the instance based on the refcount
        void deleteInstance();
    };
}
