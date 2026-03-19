//---------------------------------------------------------------------
// LabVIEW implementation of a gRPC Server
//---------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#ifdef __WIN32__
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

//---------------------------------------------------------------------
//---------------------------------------------------------------------
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
#include <semaphore.h>

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
    class CallFinishedTag;
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
        int Run(std::string address, std::string serverCertificatePath, std::string serverKeyPath);
        int ListeningPort();
        void StopServer();
        void RegisterEvent(std::string eventName, LVUserEventRef reference, std::string requestMessageName, std::string responseMessageName);
        void RegisterGenericMethodEvent(LVUserEventRef item);
        void SendEvent(std::string name, gRPCid* data);

        bool FindEventData(std::string name, LVEventData& data);
        bool HasGenericMethodEvent();
        bool HasRegisteredServerMethod(std::string methodName);

    private:
        std::mutex _mutex;
        std::unique_ptr<Server> _server;
        std::unique_ptr<grpc::ServerCompletionQueue> _cq;
        std::map<std::string, LVEventData> _registeredServerMethods;
        LVUserEventRef _genericMethodEvent;
        std::unique_ptr<grpc::AsyncGenericService> _rpcService;
        std::unique_ptr<std::thread> _runThread;
        int _listeningPort;

    private:
        void RunServer(std::string address, std::string serverCertificatePath, std::string serverKeyPath, ServerStartEventData* serverStarted);
        void HandleRpcs();

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
        void FinishFromCompletionQueue();
        bool IsCancelled();
        bool IsActive();
        bool ReadNext(int8_t* cluster);
        void SetCallStatusError(std::string errorMessage);
        void SetCallStatusError(grpc::StatusCode statusCode, std::string errorMessage);
        grpc::StatusCode GetCallStatusCode();

    private:
        CallData(std::shared_ptr<LabVIEWgRPCServer> server, grpc::AsyncGenericService* service, grpc::ServerCompletionQueue* cq);

        std::shared_ptr<LabVIEWgRPCServer> _server;
        grpc::AsyncGenericService* _service;
        grpc::ServerCompletionQueue* _cq;
        grpc::GenericServerContext _ctx;
        grpc::GenericServerAsyncReaderWriter _stream;
        grpc::ByteBuffer _rb;
        grpc::Status _callStatus;
        CallFinishedTag* _callFinishedTag;

        std::shared_ptr<LVMessage> _request;
        std::shared_ptr<LVMessage> _response;
        std::mutex _readMutex;
        std::mutex _writeMutex;
        std::mutex _stateMutex; // Protects state transitions

        enum class CallStatus
        {
            WaitingForConnection,
            Connected,
            Finishing,
            Finished
        };
        CallStatus _status;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class CallFinishedTag : public CallDataBase
    {
    public:
        CallFinishedTag(std::shared_ptr<CallData> callData);
        void Proceed(bool ok) override;

    private:
        std::shared_ptr<CallData> _callData;
    };

    //---------------------------------------------------------------------
    // Completion queue tag that keeps CallData alive
    //---------------------------------------------------------------------
    class CompletionQueueTag : public CallDataBase
    {
    public:
        CompletionQueueTag(std::shared_ptr<CallData> callData)
            : _callData(callData) {
        }

        void Proceed(bool ok) override
        {
            _callData->Proceed(ok);
            delete this;
        }

    private:
        std::shared_ptr<CallData> _callData;
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
    void OccurServerEvent(LVUserEventRef event, gRPCid* data);
    void OccurServerEvent(LVUserEventRef event, gRPCid* data, std::string eventMethodName);
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
