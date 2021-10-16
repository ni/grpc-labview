//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_client.h>
#include <lv_interop.h>

namespace grpc_labview
{
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    LabVIEWgRPCClient::LabVIEWgRPCClient(const char* address, const char* serverCertificatePath, const char* serverKeyPath)
    {    
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t CreateClient(const char* address, const char* serverCertificatePath, const char* serverKeyPath, grpc_labview::gRPCid** clientId)
{
    grpc_labview::InitCallbacks();

    auto client = new grpc_labview::LabVIEWgRPCClient(address, serverCertificatePath, serverKeyPath);
    *clientId = client;
    return 0;
}


//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientUnaryCall(grpc_labview::gRPCid* clientId, const char* methodName, const char* requestMessageName, const char* responseMessageName, int8_t* request, int8_t* response)
{    
    return 0;    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientBeginClientStreamingCall(grpc_labview::gRPCid* clientId, const char* methodName, const char* requestMessageName, const char* responseMessageName)
{
    return 0;    
}

LIBRARY_EXPORT int32_t ClientBeginServerStreamingCall(grpc_labview::gRPCid* clientId, const char* methodName, const char* requestMessageName, const char* responseMessageName)
{    
    return 0;    
}

LIBRARY_EXPORT int32_t ClientBeginBidiStreamingCall(grpc_labview::gRPCid* clientId, const char* methodName, const char* requestMessageName, const char* responseMessageName)
{
    return 0;    
}
