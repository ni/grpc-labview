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
    class GenericMethodData : public EventData, public IMessageElementMetadataOwner
    {
    public:
        GenericMethodData(CallData* call, ServerContext* context, std::shared_ptr<LVMessage> request, std::shared_ptr<LVMessage> response);
        virtual std::shared_ptr<MessageMetadata> FindMetadata(const std::string& name) override;
        std::shared_ptr<EnumMetadata> FindEnumMetadata(const std::string& name) {
            return nullptr;
        };

    public:
        CallData* _call;
        std::shared_ptr<LVMessage> _request;
        std::shared_ptr<LVMessage> _response;
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
    class LabVIEWgRPCServer : public MessageElementMetadataOwner, public gRPCid
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
        bool _shutdown;
        int _listeningPort;

    private:
        void RunServer(std::string address, std::string serverCertificatePath, std::string serverKeyPath, ServerStartEventData* serverStarted);
        void HandleRpcs(grpc::ServerCompletionQueue *cq);

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
    class CallFinishedData : CallDataBase
    {
    public:
        CallFinishedData(CallData* callData);
        void Proceed(bool ok) override;

    private:
        CallData* _call;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class CallData : public CallDataBase, public IMessageElementMetadataOwner
    {
    public:
        CallData(LabVIEWgRPCServer* server, grpc::AsyncGenericService* service, grpc::ServerCompletionQueue* cq);
        std::shared_ptr<MessageMetadata> FindMetadata(const std::string& name) override;
        std::shared_ptr<EnumMetadata> FindEnumMetadata(const std::string& name) {
            return nullptr;
        }
        void Proceed(bool ok) override;
        bool Write();
        void Finish();
        bool IsCancelled();
        bool IsActive();
        bool ReadNext();
        void ReadComplete();
        void SetCallStatusError(std::string errorMessage);
        void SetCallStatusError(grpc::StatusCode statusCode, std::string errorMessage);

    private:
        LabVIEWgRPCServer* _server;
        grpc::AsyncGenericService* _service;
        grpc::ServerCompletionQueue* _cq;
        grpc::GenericServerContext _ctx;
        grpc::GenericServerAsyncReaderWriter _stream;
        grpc::ByteBuffer _rb;
        grpc::Status _callStatus;

        Semaphore _writeSemaphore;
        std::shared_ptr<GenericMethodData> _methodData;
        std::shared_ptr<LVMessage> _request;
        std::shared_ptr<LVMessage> _response;

        bool _requestDataReady;

        enum class CallStatus
        {
            Create,
            Read,
            Writing,
            Process,
            PendingFinish,
            Finish
        };
        CallStatus _status;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class ReadNextTag : CallDataBase
    {
    public:
        ReadNextTag(CallData* callData);
        void Proceed(bool ok) override;
        bool Wait();

    private:
        Semaphore _readCompleteSemaphore;
        CallData* _callData;
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
