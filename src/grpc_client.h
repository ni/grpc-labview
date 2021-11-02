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
        MagicCookie occurrence;
        grpc::ClientContext context;
        std::shared_ptr<LVMessage> request;
        std::shared_ptr<LVMessage> response;
        grpc::Status status;
        std::future<int> _runFuture;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class ServerStreamingClientCall : public ClientCall
    {        
    public:
        std::shared_ptr<grpc_impl::ClientReaderInterface<grpc_labview::LVMessage>> _reader;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class ClientStreamingClientCall : public ClientCall
    {        
    public:
        std::shared_ptr<grpc_impl::ClientWriterInterface<grpc_labview::LVMessage>> _writer;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class BidiStreamingClientCall : public ClientCall
    {       
    public:
        std::shared_ptr<grpc_impl::ClientReaderWriterInterface<grpc_labview::LVMessage, grpc_labview::LVMessage>> _readerWriter;
    };
}
