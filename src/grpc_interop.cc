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
std::shared_ptr<MessageMetadata> CreateMessageMetadata(IMessageElementMetadataOwner* metadataOwner, LVMessageMetadata* lvMetadata)
{
    std::shared_ptr<MessageMetadata> metadata(new MessageMetadata());

    auto name = GetLVString(lvMetadata->messageName);
    metadata->messageName = name;
    if (lvMetadata->elements != nullptr)
    {
        LVMesageElementMetadata* lvElement = (LVMesageElementMetadata*)(*lvMetadata->elements)->str;
        for (int x = 0; x < (*lvMetadata->elements)->cnt; ++x, ++lvElement)
        {
            auto element = std::make_shared<MessageElementMetadata>(metadataOwner);
            element->embeddedMessageName = GetLVString(lvElement->embeddedMessageName);
            element->protobufIndex = lvElement->protobufIndex;
            element->clusterOffset = lvElement->clusterOffset;
            element->type = (LVMessageMetadataType)lvElement->valueType;
            element->isRepeated = lvElement->isRepeated;
            metadata->elements.insert({element->protobufIndex, element});
        }
    }
    return metadata;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void CopyStringToCluster(const shared_ptr<MessageElementMetadata> metadata, int8_t* start, const shared_ptr<LVMessageValue>& value)
{
    if (metadata->isRepeated)
    {        
        auto repeatedString = static_cast<const LVRepeatedStringMessageValue&>(*value);
        LVNumericArrayResize(0x01, 1, start, repeatedString._value.size() * sizeof(LStrHandle));
        auto array = *(LV1DArrayHandle*)start;
        (*array)->cnt = repeatedString._value.size();
        int x = 0;
        for (auto str: repeatedString._value)
        {
            LStrHandle* lvString = (LStrHandle*)((*array)->str + (sizeof(LStrHandle) * x));
            *lvString = nullptr;
            SetLVString(lvString, str);
            x += 1;
        }
    }
    else
    {
        SetLVString((LStrHandle*)start, ((LVStringMessageValue*)value.get())->_value);
    }
}

struct TestCluster
{
    LStrHandle rootStr;
    LV1DArrayHandle arr;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void CopyToCluster(const LVMessage& message, int8_t* cluster)
{
    TestCluster* tt = (TestCluster*)cluster;

    for (auto val : message._metadata)
    {
        auto start = cluster + val.second->clusterOffset;
        shared_ptr<LVMessageValue> value;
        for (auto v : message._values)
        {
            if (v.second->_protobufId == val.second->protobufIndex)
            {
                value = v.second;
                break;
            }
        }
        if (value != nullptr)
        {
            switch (val.second->type)
            {
                case LVMessageMetadataType::StringValue:
                    CopyStringToCluster(val.second, start, value);
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
                    CopyToCluster(*((LVNestedMessageMessageValue*)value.get())->_value, start);
                    break;
            }
        }
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void CopyStringFromCluster(const shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message)
{    
    if (metadata->isRepeated)
    {
        auto array = *(LV1DArrayHandle*)start;
        if (array && *array)
        {
            auto repeatedStringValue = std::make_shared<LVRepeatedStringMessageValue>(metadata->protobufIndex);
            message._values.emplace(metadata->protobufIndex, repeatedStringValue);
            auto lvStr = (LStrHandle*)(*array)->str;
            for (int x=0; x < (*array)->cnt; ++x)
            {
                auto str = GetLVString(*lvStr);
                repeatedStringValue->_value.Add(str);
                lvStr += 1;
            }
        }
    }
    else
    {
        auto str = GetLVString(*(LStrHandle*)start);
        auto stringValue = std::make_shared<LVStringMessageValue>(metadata->protobufIndex, str);
        message._values.emplace(metadata->protobufIndex, stringValue);
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void CopyFromCluster(LVMessage& message, int8_t* cluster)
{
    for (auto val : message._metadata)
    {
        auto start = cluster + val.second->clusterOffset;
        switch (val.second->type)
        {
            case LVMessageMetadataType::StringValue:
                CopyStringFromCluster(val.second, start, message);
                break;
            case LVMessageMetadataType::BoolValue:
                {
                    auto value = std::make_shared<LVBooleanMessageValue>(val.second->protobufIndex, *(bool*)start);
                    message._values.emplace(val.first, value);
                }
                break;
            case LVMessageMetadataType::DoubleValue:
                {
                    auto value = std::make_shared<LVDoubleMessageValue>(val.second->protobufIndex, *(double*)start);
                    message._values.emplace(val.first, value);
                }
                break;
            case LVMessageMetadataType::FloatValue:
                {
                    auto value = std::make_shared<LVFloatMessageValue>(val.second->protobufIndex, *(float*)start);
                    message._values.emplace(val.first, value);
                }
                break;
            case LVMessageMetadataType::Int32Value:
                {
                    auto value = std::make_shared<LVInt32MessageValue>(val.second->protobufIndex, *(int*)start);
                    message._values.emplace(val.first, value);
                }
                break;
            case LVMessageMetadataType::MessageValue:
                {
                    auto metadata = val.second->_owner->FindMetadata(val.second->embeddedMessageName);
                    auto nested = std::make_shared<LVMessage>(metadata->elements);
                    CopyFromCluster(*nested, start);
                    auto value = std::make_shared<LVNestedMessageMessageValue>(val.second->protobufIndex, nested);
                    message._values.emplace(val.first, value);
                }
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
    auto metadata = CreateMessageMetadata(server, lvMetadata);
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
