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
void CopyToCluster(const LVMessage& message, int8_t* cluster)
{
    for (auto val : message._metadata)
    {
        auto start = cluster + val.second.clusterOffset;
        shared_ptr<LVMessageValue> value;
        for (auto v : message._values)
        {
            if (v->_protobufId == val.second.protobufIndex)
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
                    SetLVString((LStrHandle*)start, ((LVStringMessageValue*)value.get())->_value);
                    break;
                case LVMessageMetadataType::BoolValue:
                    *(bool*)start = ((LVBooleanMessageValue*)value.get())->_value;
                    break;
                case LVMessageMetadataType::DoubleValue:
                    *(double*)start = ((LVDoubleMessageValue*)value.get())->_value;
                    break;
                case LVMessageMetadataType::FloatValue:
                    *(float*)start = ((LVFloatMessageValue*)value.get())->_value;
                    break;
                case LVMessageMetadataType::Int32Value:
                    *(int*)start = ((LVInt32MessageValue*)value.get())->_value;
                    break;
                case LVMessageMetadataType::MessageValue:
                    break;
            }
        }
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void CopyFromCluster(LVMessage& message, int8_t* cluster)
{
    for (auto val : message._metadata)
    {
        auto start = cluster + val.second.clusterOffset;
        switch (val.second.type)
        {
            case LVMessageMetadataType::StringValue:
            {
                auto str = GetLVString(*(LStrHandle*)start);
                auto stringValue = std::make_shared<LVStringMessageValue>(val.second.protobufIndex, str);
                message._values.push_back(stringValue);
            }
            case LVMessageMetadataType::BoolValue:
            {
                auto value = std::make_shared<LVBooleanMessageValue>(val.second.protobufIndex, *(bool*)start);
                message._values.push_back(value);
            }
            case LVMessageMetadataType::DoubleValue:
            {
                auto value = std::make_shared<LVDoubleMessageValue>(val.second.protobufIndex, *(double*)start);
                message._values.push_back(value);
            }
            case LVMessageMetadataType::FloatValue:
            {
                auto value = std::make_shared<LVFloatMessageValue>(val.second.protobufIndex, *(float*)start);
                message._values.push_back(value);
            }
            case LVMessageMetadataType::Int32Value:
            {
                auto value = std::make_shared<LVInt32MessageValue>(val.second.protobufIndex, *(int*)start);
                message._values.push_back(value);
            }
            case LVMessageMetadataType::MessageValue:
            break;
        }
    }
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
    auto server = *(LabVIEWgRPCServer**)id;
    auto metadata = CreateMessageMetadata(lvMetadata);
    server->RegisterMetadata(metadata);
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
LIBRARY_EXPORT int32_t GetRequestData(LVgRPCid id, int8_t* lvRequest)
{
    auto data = *(GenericMethodData**)id;
    CopyToCluster(*data->_request, lvRequest);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t SetResponseData(LVgRPCid id, int8_t* lvRequest)
{
    auto data = *(GenericMethodData**)id;
    CopyFromCluster(*data->_response, lvRequest);
    data->_call->Write();
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
