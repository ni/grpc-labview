//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_server.h>
#include <lv_interop.h>
#include <iostream>
#include <memory>
#include <string>
#include <map>
#include <mutex>
#include <thread>

#ifndef _WIN32
#include <dlfcn.h>
#endif

//---------------------------------------------------------------------
//---------------------------------------------------------------------
using namespace std;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void OccurServerEvent(LVUserEventRef event, EventData* data)
{
    auto error = LVPostLVUserEvent(event, &data);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t LVCreateServer(LVgRPCServerid* id)
{
    InitCallbacks();
    auto server = new LabVIEWQueryServerInstance();
    *id = server;   
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t LVStartServer(char* address, char* serverCertificatePath, char* serverKeyPath, LVgRPCServerid* id)
{   
    LabVIEWQueryServerInstance* server = *(LabVIEWQueryServerInstance**)id;
    return server->Run(address, serverCertificatePath, serverKeyPath);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t LVStopServer(LVgRPCServerid* id)
{
    LabVIEWQueryServerInstance* server = *(LabVIEWQueryServerInstance**)id;
    server->StopServer();
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t RegisterServerEvent(const char* name, LVUserEventRef* item, LVgRPCServerid* id)
{
    LabVIEWQueryServerInstance* server = *(LabVIEWQueryServerInstance**)id;
    server->RegisterEvent(name, *item);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t InvokeGetRequest(LVgRPCid id, LVInvokeRequest* lvRequest)
{
    auto data = *(GenericMethodData**)id;
    auto request = (InvokeRequest*)data->request;
    SetLVString(&lvRequest->command, request->command());
    SetLVString(&lvRequest->parameter, request->parameter());
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t InvokeSetResponse(LVgRPCid id, LVInvokeResponse* lvResponse)
{
    auto data = *(GenericMethodData**)id;
    auto response = (InvokeResponse*)data->response;
    response->set_status(lvResponse->status);
    data->NotifyComplete();
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t QueryGetRequest(LVgRPCid id, LVQueryRequest* lvRequest)
{
    auto data = *(GenericMethodData**)id;
    auto request = (QueryRequest*)data->request;
    SetLVString(&lvRequest->query, request->query());
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t QuerySetResponse(LVgRPCid id, LVQueryResponse* lvResponse)
{
    auto data = *(GenericMethodData**)id;
    auto response = (QueryResponse*)data->response;
    response->set_message(GetLVString(lvResponse->message));
    response->set_status(lvResponse->status);
    data->NotifyComplete();
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t RegisterGetRequest(LVgRPCid id, LVRegistrationRequest* request)
{
    RegistrationRequestData* data = *(RegistrationRequestData**)id;
    SetLVString(&request->eventName, data->request->eventname());
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t NotifyServerEvent(LVgRPCid id, LVServerEvent* event)
{
    RegistrationRequestData* data = *(RegistrationRequestData**)id;
    queryserver::ServerEvent e;
    e.set_eventdata(GetLVString(event->eventData));
    e.set_serverid(event->serverId);
    e.set_status(event->status);
    data->eventWriter->Write(e);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t CloseServerEvent(LVgRPCid id)
{
    RegistrationRequestData* data = *(RegistrationRequestData**)id;
    data->NotifyComplete();
    return 0;
}
