//---------------------------------------------------------------------
// LabVIEW implementation of a gRPC Server
//---------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <metadata_owner.h>
#include <grpcpp/grpcpp.h>
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
        std::future<int> _runFuture;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    struct ClientCall : public gRPCid
    {
    public:
        MagicCookie occurrence;
        grpc::ClientContext context;
        std::shared_ptr<LVMessage> request;
        std::shared_ptr<LVMessage> response;
        grpc::Status status;
    };

}
