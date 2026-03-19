//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

#include <cstdint>

namespace grpc_labview
{
    class gRPCid;

    // LV cleanup proc used to unregister any CallData objects registered with the pointer manager if the user did
    // not explicitly close and clean them up.
    int32_t CloseServerEventCleanupProc(gRPCid* serverId);
}