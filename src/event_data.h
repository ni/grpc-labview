//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpcpp/impl/codegen/server_context.h>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
using grpc::ServerContext;

namespace grpc_labview 
{
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class EventData : public gRPCid
    {
    public:
        EventData(ServerContext* context);

    private:
        bool _completed;
        std::mutex lockMutex;
        std::condition_variable lock;

    public:
        ServerContext* context;

    public:
        void WaitForComplete();
        void NotifyComplete();
    };
}
