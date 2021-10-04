//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_server.h>
#include <cluster_copier.h>
#include <lv_interop.h>
#include <iostream>
#include <memory>
#include <string>
#include <map>
#include <mutex>
#include <thread>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void OccurServerEvent(LVUserEventRef event, EventData* data)
{
    auto error = LVPostLVUserEvent(event, &data);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void OccurServerEvent(LVUserEventRef event, EventData* data, std::string eventMethodName)
{
    LStr* lvMethodName = (LStr*)malloc(sizeof(int32_t) + eventMethodName.length() + 1);
    lvMethodName->cnt = eventMethodName.length();
    memcpy(lvMethodName->str, eventMethodName.c_str(), eventMethodName.length());

    GeneralMethodEventData eventData;
    eventData.methodData = data;
    eventData.methodName = &lvMethodName;
    auto error = LVPostLVUserEvent(event, &eventData);

    free(lvMethodName);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
std::shared_ptr<MessageMetadata> CreateMessageMetadata(IMessageElementMetadataOwner* metadataOwner, LVMessageMetadata* lvMetadata)
{
    std::shared_ptr<MessageMetadata> metadata(new MessageMetadata());

    auto name = GetLVString(lvMetadata->messageName);
    metadata->messageName = name;
    int clusterOffset = 0;
    if (lvMetadata->elements != nullptr)
    {
        auto lvElement = (*lvMetadata->elements)->bytes<LVMesageElementMetadata>();
        for (int x = 0; x < (*lvMetadata->elements)->cnt; ++x, ++lvElement)
        {
            auto element = std::make_shared<MessageElementMetadata>(metadataOwner);
            element->embeddedMessageName = GetLVString(lvElement->embeddedMessageName);
            element->protobufIndex = lvElement->protobufIndex;
            element->type = (LVMessageMetadataType)lvElement->valueType;
            element->isRepeated = lvElement->isRepeated;
            metadata->_elements.push_back(element);
            metadata->_mappedElements.emplace(element->protobufIndex, element);

        }
    }
    return metadata;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
std::shared_ptr<MessageMetadata> CreateMessageMetadata2(IMessageElementMetadataOwner* metadataOwner, LVMessageMetadata2* lvMetadata)
{
    std::shared_ptr<MessageMetadata> metadata(new MessageMetadata());

    auto name = GetLVString(lvMetadata->messageName);
    metadata->messageName = name;
    int clusterOffset = 0;
    if (lvMetadata->elements != nullptr)
    {
        auto lvElement = (*lvMetadata->elements)->bytes<LVMesageElementMetadata>();
        for (int x = 0; x < (*lvMetadata->elements)->cnt; ++x, ++lvElement)
        {
            auto element = std::make_shared<MessageElementMetadata>(metadataOwner);
            element->embeddedMessageName = GetLVString(lvElement->embeddedMessageName);
            element->protobufIndex = lvElement->protobufIndex;
            element->type = (LVMessageMetadataType)lvElement->valueType;
            element->isRepeated = lvElement->isRepeated;
            metadata->_elements.push_back(element);
            metadata->_mappedElements.emplace(element->protobufIndex, element);

        }
    }
    return metadata;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t LVCreateServer(LVgRPCServerid* id)
{
    InitCallbacks();
    auto server = new LabVIEWgRPCServer();
    *id = server;   
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t LVStartServer(char* address, char* serverCertificatePath, char* serverKeyPath, LVgRPCServerid* id)
{   
    auto server = *(LabVIEWgRPCServer**)id;
    return server->Run(address, serverCertificatePath, serverKeyPath);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t LVStopServer(LVgRPCServerid* id)
{
    auto server = *(LabVIEWgRPCServer**)id;
    server->StopServer();
    delete server;
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t RegisterMessageMetadata(LVgRPCServerid* id, LVMessageMetadata* lvMetadata)
{    
    auto server = *(MessageElementMetadataOwner**)id;
    auto metadata = CreateMessageMetadata(server, lvMetadata);
    server->RegisterMetadata(metadata);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t RegisterMessageMetadata2(LVgRPCServerid* id, LVMessageMetadata2* lvMetadata)
{    
    auto server = *(MessageElementMetadataOwner**)id;
    auto metadata = CreateMessageMetadata2(server, lvMetadata);
    server->RegisterMetadata(metadata);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t CompleteMetadataRegistration(LVgRPCServerid* id)
{    
    auto server = *(MessageElementMetadataOwner**)id;
    server->FinalizeMetadata();
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t RegisterServerEvent(LVgRPCServerid* id, const char* name, LVUserEventRef* item, const char* requestMessageName, const char* responseMessageName)
{    
    auto server = *(LabVIEWgRPCServer**)id;

    server->RegisterEvent(name, *item, requestMessageName, responseMessageName);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t RegisterGenericMethodServerEvent(LVgRPCServerid* id, LVUserEventRef* item)
{    
    auto server = *(LabVIEWgRPCServer**)id;

    server->RegisterGenericMethodEvent(*item);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t GetRequestData(LVgRPCid id, int8_t* lvRequest)
{
    auto data = *(GenericMethodData**)id;
    data->_call->ReadNext();
    ClusterDataCopier::CopyToCluster(*data->_request, lvRequest);
    data->_call->ReadComplete();
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t SetResponseData(LVgRPCid id, int8_t* lvRequest)
{
    auto data = *(GenericMethodData**)id;
    ClusterDataCopier::CopyFromCluster(*data->_response, lvRequest);
    if (!data->_call->Write())
    {
        return -1;
    }
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t CloseServerEvent(LVgRPCid id)
{
    GenericMethodData* data = *(GenericMethodData**)id;
    data->NotifyComplete();
    data->_call->Finish();    
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t IsCancelled(LVgRPCid id)
{
    GenericMethodData* data = *(GenericMethodData**)id;
    return data->_call->IsCancelled();
}
