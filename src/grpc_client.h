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
        
    public:
        MagicCookie occurrence;
        grpc::ClientContext context;
        std::shared_ptr<LVMessage> request;
        std::shared_ptr<LVMessage> response;
        grpc::Status status;
        std::future<int> _runFuture;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class StreamWriter
    {
    public:
        virtual bool Write(LVMessage* message) = 0;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class StreamReader
    {
    public:
        std::future<int> _readFuture;

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

    public:
        std::shared_ptr<grpc_impl::ClientReaderInterface<grpc_labview::LVMessage>> _reader;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class ClientStreamingClientCall : public ClientCall, public StreamWriter
    {        
    public:
        ~ClientStreamingClientCall();

        bool Write(LVMessage* message) override;

        std::shared_ptr<grpc_impl::ClientWriterInterface<grpc_labview::LVMessage>> _writer;

    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class BidiStreamingClientCall : public ClientCall, public StreamReader, public StreamWriter
    {       
    public:
        ~BidiStreamingClientCall();

        bool Read(LVMessage* message) override;
        bool Write(LVMessage* message) override;

        std::shared_ptr<grpc_impl::ClientReaderWriterInterface<grpc_labview::LVMessage, grpc_labview::LVMessage>> _readerWriter;
    };
}
