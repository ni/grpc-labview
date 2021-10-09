//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_client.h>
#include <lv_interop.h>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t CreateClient(const char* address, const char* serverCertificatePath, const char* serverKeyPath, LVgRPCid** clientId)
{
    InitCallbacks();

    auto client = new LabVIEWgRPCClient(address, serverCertificatePath, serverKeyPath);
    *clientId = client;
    return 0;
}


//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientUnaryCall(LVgRPCid* clientId, const char* methodName, const char* requestMessageName, const char* responseMessageName, int8_t* request, int8_t* response)
{    
    return 0;    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t ClientBeginClientStreamingCall(LVgRPCid* clientId, const char* methodName, const char* requestMessageName, const char* responseMessageName)
{
    return 0;    
}

LIBRARY_EXPORT int32_t ClientBeginServerStreamingCall(LVgRPCid* clientId, const char* methodName, const char* requestMessageName, const char* responseMessageName)
{    
    return 0;    
}

LIBRARY_EXPORT int32_t ClientBeginBidiStreamingCall(LVgRPCid* clientId, const char* methodName, const char* requestMessageName, const char* responseMessageName)
{
    return 0;    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LabVIEWgRPCClient::LabVIEWgRPCClient(const char* address, const char* serverCertificatePath, const char* serverKeyPath)
{    
}
