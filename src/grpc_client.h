//---------------------------------------------------------------------
// LabVIEW implementation of a gRPC Server
//---------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <metadata_owner.h>
#include <grpcpp/grpcpp.h>
#include <lv_message.h>
#include <grpcpp/impl/codegen/sync_stream.h>
#include <future>
#include <list>

namespace grpc_labview 
{
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class LVMessage;
    class ClientCall;

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class LabVIEWgRPCClient : public MessageElementMetadataOwner, public gRPCid
    {
    public:
        LabVIEWgRPCClient();
        void Connect(const char* address, const std::string& certificatePath);

    public:
        std::shared_ptr<grpc::Channel> Channel;
        std::list<ClientCall*> ActiveClientCalls;
        std::mutex clientLock;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class ClientContext : public gRPCid
    {
    public:
        void Cancel();
        grpc::ClientContext gRPCClientContext;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class ClientCall : public gRPCid
    {
    public:
        ClientCall(int32_t timeoutMs);
        virtual ~ClientCall();
        virtual void Finish();
        void Cancel();
        
    public:
        std::shared_ptr<grpc_labview::LabVIEWgRPCClient> _client;
        std::string _methodName;
        MagicCookie _occurrence;
        std::shared_ptr<ClientContext> _context;
        std::shared_ptr<LVMessage> _request;
        std::shared_ptr<LVMessage> _response;
        grpc::Status _status;
        std::future<int> _runFuture;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class StreamWriter
    {
    public:
        virtual bool Write(LVMessage* message) = 0;
        virtual void WritesComplete() = 0;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class StreamReader
    {
    public:
        std::future<bool> _readFuture;

    public:
        virtual bool Read(LVMessage* message) = 0;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class ServerStreamingClientCall : public ClientCall, public StreamReader
    {        
    public:
        ServerStreamingClientCall(int32_t timeoutMs) : ClientCall(timeoutMs) {}
        ~ServerStreamingClientCall() override;
        bool Read(LVMessage* message) override;
        void Finish() override;
    public:
        std::shared_ptr<grpc::ClientReaderInterface<grpc_labview::LVMessage>> _reader;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class ClientStreamingClientCall : public ClientCall, public StreamWriter
    {        
    public:
        ClientStreamingClientCall(int32_t timeoutMs) : ClientCall(timeoutMs) { _writesComplete = false; }
        ~ClientStreamingClientCall();
        void Finish() override;
        bool Write(LVMessage* message) override;
        void WritesComplete() override;

    public:
        std::shared_ptr<grpc::ClientWriterInterface<grpc_labview::LVMessage>> _writer;
    private:
        bool _writesComplete;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class BidiStreamingClientCall : public ClientCall, public StreamReader, public StreamWriter
    {       
    public:
        BidiStreamingClientCall(int32_t timeoutMs) : ClientCall(timeoutMs) { _writesComplete = false; }
        ~BidiStreamingClientCall();
        void Finish() override;
        void WritesComplete() override;
        bool Read(LVMessage* message) override;
        bool Write(LVMessage* message) override;

    public:
        std::shared_ptr<grpc::ClientReaderWriterInterface<grpc_labview::LVMessage, grpc_labview::LVMessage>> _readerWriter;
    private:
        bool _writesComplete;
    };
}
