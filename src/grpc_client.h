//---------------------------------------------------------------------
// LabVIEW implementation of a gRPC Server
//---------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <metadata_owner.h>
#include <grpcpp/grpcpp.h>
#include <lv_message.h>
#include <grpcpp/impl/codegen/sync_stream_impl.h>
#include <future>

namespace grpc_labview 
{
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class LVMessage;

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class LabVIEWgRPCClient : public MessageElementMetadataOwner, public gRPCid
    {
    public:
        LabVIEWgRPCClient();
        void Connect(const char* address, const std::string& certificatePath);

    public:
        std::shared_ptr<grpc::Channel> Channel;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class ClientCall : public gRPCid
    {
    public:
        virtual ~ClientCall();
        virtual void Finish();
        
    public:
        grpc_labview::LabVIEWgRPCClient* _client;
        std::string _methodName;
        MagicCookie _occurrence;
        grpc::ClientContext _context;
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
        ~ServerStreamingClientCall() override;
        bool Read(LVMessage* message) override;
        void Finish() override;
    public:
        std::shared_ptr<grpc_impl::ClientReaderInterface<grpc_labview::LVMessage>> _reader;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class ClientStreamingClientCall : public ClientCall, public StreamWriter
    {        
    public:
        ~ClientStreamingClientCall();
        void Finish() override;
        bool Write(LVMessage* message) override;
        void WritesComplete() override;

    public:
        std::shared_ptr<grpc_impl::ClientWriterInterface<grpc_labview::LVMessage>> _writer;
    private:
        bool _writesComplete;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class BidiStreamingClientCall : public ClientCall, public StreamReader, public StreamWriter
    {       
    public:
        ~BidiStreamingClientCall();
        void Finish() override;
        void WritesComplete() override;
        bool Read(LVMessage* message) override;
        bool Write(LVMessage* message) override;

    public:
        std::shared_ptr<grpc_impl::ClientReaderWriterInterface<grpc_labview::LVMessage, grpc_labview::LVMessage>> _readerWriter;
    private:
        bool _writesComplete;
    };
}
