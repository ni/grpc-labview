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

namespace grpc_labview
{
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void OccurServerEvent(LVUserEventRef event, gRPCid* data)
    {
        auto error = PostUserEvent(event, &data);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void OccurServerEvent(LVUserEventRef event, gRPCid* data, std::string eventMethodName)
    {
        LStr* lvMethodName = (LStr*)malloc(sizeof(int32_t) + eventMethodName.length() + 1);
        lvMethodName->cnt = eventMethodName.length();
        memcpy(lvMethodName->str, eventMethodName.c_str(), eventMethodName.length());

        GeneralMethodEventData eventData;
        eventData.methodData = data;
        eventData.methodName = &lvMethodName;
        auto error = PostUserEvent(event, &eventData);

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
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t LVCreateServer(grpc_labview::gRPCid** id)
{
    grpc_labview::InitCallbacks();
    auto server = new grpc_labview::LabVIEWgRPCServer();
    *id = server;   
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t LVStartServer(char* address, char* serverCertificatePath, char* serverKeyPath, grpc_labview::gRPCid** id)
{   
    auto server = (*id)->CastTo<grpc_labview::LabVIEWgRPCServer>();
    return server->Run(address, serverCertificatePath, serverKeyPath);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t LVGetServerListeningPort(grpc_labview::gRPCid** id, int* listeningPort)
{   
    auto server = (*id)->CastTo<grpc_labview::LabVIEWgRPCServer>();
    *listeningPort = server->ListeningPort();
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t LVStopServer(grpc_labview::gRPCid** id)
{
    auto server = (*id)->CastTo<grpc_labview::LabVIEWgRPCServer>();
    server->StopServer();
    delete server;
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t RegisterMessageMetadata(grpc_labview::gRPCid** id, grpc_labview::LVMessageMetadata* lvMetadata)
{    
    auto server = (*id)->CastTo<grpc_labview::MessageElementMetadataOwner>();
    auto metadata = CreateMessageMetadata(server, lvMetadata);
    server->RegisterMetadata(metadata);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t RegisterMessageMetadata2(grpc_labview::gRPCid** id, grpc_labview::LVMessageMetadata2* lvMetadata)
{    
    auto server = (*id)->CastTo<grpc_labview::MessageElementMetadataOwner>();
    auto metadata = CreateMessageMetadata2(server, lvMetadata);
    server->RegisterMetadata(metadata);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t CompleteMetadataRegistration(grpc_labview::gRPCid** id)
{        
    auto server = (*id)->CastTo<grpc_labview::MessageElementMetadataOwner>();
    server->FinalizeMetadata();
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t RegisterServerEvent(grpc_labview::gRPCid** id, const char* name, grpc_labview::LVUserEventRef* item, const char* requestMessageName, const char* responseMessageName)
{    
    auto server = (*id)->CastTo<grpc_labview::LabVIEWgRPCServer>();

    server->RegisterEvent(name, *item, requestMessageName, responseMessageName);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t RegisterGenericMethodServerEvent(grpc_labview::gRPCid** id, grpc_labview::LVUserEventRef* item)
{    
    auto server = (*id)->CastTo<grpc_labview::LabVIEWgRPCServer>();

    server->RegisterGenericMethodEvent(*item);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t GetRequestData(grpc_labview::gRPCid** id, int8_t* lvRequest)
{
    auto data = (*id)->CastTo<grpc_labview::GenericMethodData>();
    data->_call->ReadNext();
    grpc_labview::ClusterDataCopier::CopyToCluster(*data->_request, lvRequest);
    data->_call->ReadComplete();
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t SetResponseData(grpc_labview::gRPCid** id, int8_t* lvRequest)
{
    auto data = (*id)->CastTo<grpc_labview::GenericMethodData>();
    grpc_labview::ClusterDataCopier::CopyFromCluster(*data->_response, lvRequest);
    if (!data->_call->Write())
    {
        return -1;
    }
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t CloseServerEvent(grpc_labview::gRPCid** id)
{
    auto data = (*id)->CastTo<grpc_labview::GenericMethodData>();
    data->NotifyComplete();
    data->_call->Finish();    
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t IsCancelled(grpc_labview::gRPCid** id)
{
    auto data = (*id)->CastTo<grpc_labview::GenericMethodData>();
    return data->_call->IsCancelled();
}
