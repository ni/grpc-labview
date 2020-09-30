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

std::shared_ptr<MessageMetadata> CreateMessageMetadata(LVMessageMetadata* lvMetadata)
{
    std::shared_ptr<MessageMetadata> metadata(new MessageMetadata());

    auto name = GetLVString(lvMetadata->messageName);
    metadata->messageName = name;
    if (lvMetadata->elements != nullptr)
    {
        LVMesageElementMetadata* lvElement = (LVMesageElementMetadata*)(*lvMetadata->elements)->str;
        for (int x = 0; x < (*lvMetadata->elements)->cnt; ++x, ++lvElement)
        {
            MessageElementMetadata element;
            element.embeddedMessageName = GetLVString(lvElement->embeddedMessageName);
            element.protobufIndex = lvElement->protobufIndex;
            element.clusterOffset = lvElement->clusterOffset;
            element.type = (LVMessageMetadataType)lvElement->valueType;
            element.isRepeated = lvElement->isRepeated;
            metadata->elements.insert(std::pair<int, MessageElementMetadata>(element.protobufIndex, element));
        }
    }
    return metadata;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t RegisterServerEvent(const char* name, LVUserEventRef* item, LVMessageMetadata* lvRequestMetadata, LVMessageMetadata* lvResponseMetadata, LVgRPCServerid* id)
{
    LabVIEWQueryServerInstance* server = *(LabVIEWQueryServerInstance**)id;

    auto requestMetadata = CreateMessageMetadata(lvRequestMetadata);
    auto responseMetadata = CreateMessageMetadata(lvResponseMetadata);

    server->RegisterEvent(name, *item, requestMetadata, responseMetadata);
    return 0;
}

void CopyToCluster(const LVMessage& message, int8_t* cluster)
{
    for (auto val : message._metadata)
    {
        auto start = cluster + val.second.clusterOffset;
        LVMessageValue* value = nullptr;
        for (auto v : message._values)
        {
            if (v->protobufId == val.second.protobufIndex)
            {
                value = v;
                break;
            }
        }
        if (value != nullptr)
        {
            switch (val.second.type)
            {
                case LVMessageMetadataType::StringValue:
                {
                    SetLVString((LStrHandle*)start, ((LVStringMessageValue*)value)->value);
                }
                break;
            }
        }
    }
}

void CopyFromCluster(LVMessage& message, int8_t* cluster)
{
    for (auto val : message._metadata)
    {
        auto start = cluster + val.second.clusterOffset;
        switch (val.second.type)
        {
            case LVMessageMetadataType::StringValue:
            {
                auto stringValue = new LVStringMessageValue();
                stringValue->value = GetLVString(*(LStrHandle*)start);
                message._values.push_back(stringValue);
            }
            break;
        }
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t GetRequestData(LVgRPCid id, int8_t* lvRequest)
{
    auto data = *(GenericMethodData**)id;
    CopyToCluster(*data->request, lvRequest);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t SetResponseData(LVgRPCid id, int8_t* lvRequest)
{
    auto data = *(GenericMethodData**)id;
    CopyFromCluster(*data->response, lvRequest);

    // TODO: If this is a streaming call then we do not notify here.
    data->NotifyComplete();
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
