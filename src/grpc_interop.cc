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
void OccurServerEvent(LVUserEventRef event, LVgRPCid* data)
{
    auto error = LVPostLVUserEvent(event, &data);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void OccurServerEvent(LVUserEventRef event, LVgRPCid* data, std::string eventMethodName)
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
    metadata->typeUrl = name;
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
    metadata->typeUrl = GetLVString(lvMetadata->typeUrl);
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
LIBRARY_EXPORT int32_t LVCreateServer(LVgRPCid** id)
{
    InitCallbacks();
    auto server = new LabVIEWgRPCServer();
    *id = server;   
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t LVStartServer(char* address, char* serverCertificatePath, char* serverKeyPath, LVgRPCid** id)
{   
    auto server = (*id)->CastTo<LabVIEWgRPCServer>();
    return server->Run(address, serverCertificatePath, serverKeyPath);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t LVStopServer(LVgRPCid** id)
{
    auto server = (*id)->CastTo<LabVIEWgRPCServer>();
    server->StopServer();
    delete server;
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t RegisterMessageMetadata(LVgRPCid** id, LVMessageMetadata* lvMetadata)
{    
    auto server = (*id)->CastTo<MessageElementMetadataOwner>();
    auto metadata = CreateMessageMetadata(server, lvMetadata);
    server->RegisterMetadata(metadata);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t RegisterMessageMetadata2(LVgRPCid** id, LVMessageMetadata2* lvMetadata)
{    
    auto server = (*id)->CastTo<MessageElementMetadataOwner>();
    auto metadata = CreateMessageMetadata2(server, lvMetadata);
    server->RegisterMetadata(metadata);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t CompleteMetadataRegistration(LVgRPCid** id)
{        
    auto server = (*id)->CastTo<MessageElementMetadataOwner>();
    server->FinalizeMetadata();
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t RegisterServerEvent(LVgRPCid** id, const char* name, LVUserEventRef* item, const char* requestMessageName, const char* responseMessageName)
{    
    auto server = (*id)->CastTo<LabVIEWgRPCServer>();

    server->RegisterEvent(name, *item, requestMessageName, responseMessageName);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t RegisterGenericMethodServerEvent(LVgRPCid** id, LVUserEventRef* item)
{    
    auto server = (*id)->CastTo<LabVIEWgRPCServer>();

    server->RegisterGenericMethodEvent(*item);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t GetRequestData(LVgRPCid** id, int8_t* lvRequest)
{
    auto data = (*id)->CastTo<GenericMethodData>();
    data->_call->ReadNext();
    ClusterDataCopier::CopyToCluster(*data->_request, lvRequest);
    data->_call->ReadComplete();
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t SetResponseData(LVgRPCid** id, int8_t* lvRequest)
{
    auto data = (*id)->CastTo<GenericMethodData>();
    ClusterDataCopier::CopyFromCluster(*data->_response, lvRequest);
    if (!data->_call->Write())
    {
        return -1;
    }
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t CloseServerEvent(LVgRPCid** id)
{
    auto data = (*id)->CastTo<GenericMethodData>();
    data->NotifyComplete();
    data->_call->Finish();    
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t IsCancelled(LVgRPCid** id)
{
    auto data = (*id)->CastTo<GenericMethodData>();
    return data->_call->IsCancelled();
}
