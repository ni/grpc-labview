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
void CopyStringToCluster(const shared_ptr<MessageElementMetadata> metadata, int8_t* start, const shared_ptr<LVMessageValue>& value)
{
    if (metadata->isRepeated)
    {        
        auto repeatedString = static_cast<const LVRepeatedStringMessageValue&>(*value);
        LVNumericArrayResize(0x08, 1, start, repeatedString._value.size());
        auto array = *(LV1DArrayHandle*)start;
        (*array)->cnt = repeatedString._value.size();
        int x = 0;
        auto lvString = (*array)->bytes<LStrHandle>();
        for (auto str: repeatedString._value)
        {
            *lvString = nullptr;
            SetLVString(lvString, str);
            lvString += 1;
        }
    }
    else
    {
        SetLVString((LStrHandle*)start, ((LVStringMessageValue*)value.get())->_value);
    }
}

struct lVCluster
{
};

void CopyToCluster(const LVMessage& message, int8_t* cluster);

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void CopyMessageToCluster(const shared_ptr<MessageElementMetadata> metadata, int8_t* start, const shared_ptr<LVMessageValue>& value)
{
    if (metadata->isRepeated)
    {
        auto repeatedNested = static_cast<const LVRepeatedNestedMessageMessageValue&>(*value);
        LVNumericArrayResize(0x08, 1, start, repeatedNested._value.size());
        auto array = *(LV1DArrayHandle*)start;
        (*array)->cnt = repeatedNested._value.size();
        int x = 0;
        auto lvCluster = (*array)->bytes<lVCluster*>();
        for (auto str : repeatedNested._value)
        {
            *lvCluster = nullptr;
            CopyToCluster(*str, (int8_t*)lvCluster);
            lvCluster += 1;
        }
    }
    else
    {
        CopyToCluster(*((LVNestedMessageMessageValue*)value.get())->_value, start);
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void CopyInt32ToCluster(const shared_ptr<MessageElementMetadata> metadata, int8_t* start, const shared_ptr<LVMessageValue>& value)
{
    if (metadata->isRepeated)
    {        
        auto repeatedInt32 = static_pointer_cast<LVRepeatedInt32MessageValue>(value);
        LVNumericArrayResize(0x03, 1, start, repeatedInt32->_value.size());
        auto array = *(LV1DArrayHandle*)start;
        (*array)->cnt = repeatedInt32->_value.size();
        auto byteCount = repeatedInt32->_value.size() * sizeof(int32_t);
        memcpy((*array)->bytes<int32_t>(), repeatedInt32->_value.data(), byteCount);
    }
    else
    {
        *(int*)start = ((LVInt32MessageValue*)value.get())->_value;
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void CopyBoolToCluster(const shared_ptr<MessageElementMetadata> metadata, int8_t* start, const shared_ptr<LVMessageValue>& value)
{
    if (metadata->isRepeated)
    {        
        auto repeatedBoolean = static_pointer_cast<LVRepeatedBooleanMessageValue>(value);
        LVNumericArrayResize(0x01, 1, start, repeatedBoolean->_value.size());
        auto array = *(LV1DArrayHandle*)start;
        (*array)->cnt = repeatedBoolean->_value.size();
        auto byteCount = repeatedBoolean->_value.size() * sizeof(bool);
        memcpy((*array)->bytes<bool>(), repeatedBoolean->_value.data(), byteCount);
    }
    else
    {
        *(bool*)start = ((LVBooleanMessageValue*)value.get())->_value;
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void CopyDoubleToCluster(const shared_ptr<MessageElementMetadata> metadata, int8_t* start, const shared_ptr<LVMessageValue>& value)
{
    if (metadata->isRepeated)
    {
        auto repeatedDouble = static_pointer_cast<LVRepeatedDoubleMessageValue>(value);
        auto array = *(LV1DArrayHandle*)start;
        LVNumericArrayResize(0x0A, 1, start, repeatedDouble->_value.size());
        array = *(LV1DArrayHandle*)start;
        (*array)->cnt = repeatedDouble->_value.size();
        auto byteCount = repeatedDouble->_value.size() * sizeof(double);
        memcpy((*array)->bytes<double>(), repeatedDouble->_value.data(), byteCount);
    }
    else
    {
        *(double*)start = ((LVDoubleMessageValue*)value.get())->_value;
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void CopyFloatToCluster(const shared_ptr<MessageElementMetadata> metadata, int8_t* start, const shared_ptr<LVMessageValue>& value)
{
    if (metadata->isRepeated)
    {
        auto repeatedFloat = static_pointer_cast<LVRepeatedFloatMessageValue>(value);
        LVNumericArrayResize(0x03, 1, start, repeatedFloat->_value.size());
        auto array = *(LV1DArrayHandle*)start;
        (*array)->cnt = repeatedFloat->_value.size();
        auto byteCount = repeatedFloat->_value.size() * sizeof(float);
        memcpy((*array)->bytes<float>(), repeatedFloat->_value.data(), byteCount);
    }
    else
    {
        *(float*)start = ((LVFloatMessageValue*)value.get())->_value;
    }
}

struct TestI
{
    bool boolValue;
};

struct Test
{
    bool boolValue;
    TestI inside;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void CopyToCluster(const LVMessage& message, int8_t* cluster)
{
    auto t = (Test*)cluster;
    auto offset = offsetof(Test, inside);
    auto size = sizeof(t);

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
                    CopyBoolToCluster(val.second, start, value);
                    break;
                case LVMessageMetadataType::DoubleValue:
                    CopyDoubleToCluster(val.second, start, value);
                    break;
                case LVMessageMetadataType::FloatValue:
                    CopyFloatToCluster(val.second, start, value);
                    break;
                case LVMessageMetadataType::Int32Value:
                    CopyInt32ToCluster(val.second, start, value);
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
            auto lvStr = (*array)->bytes<LStrHandle>();
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
void CopyBoolFromCluster(const shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message)
{    
    if (metadata->isRepeated)
    {
        auto array = *(LV1DArrayHandle*)start;
        if (array && *array)
        {
            auto count = (*array)->cnt;
            auto repeatedValue = std::make_shared<LVRepeatedBooleanMessageValue>(metadata->protobufIndex);
            message._values.emplace(metadata->protobufIndex, repeatedValue);
            auto data = (*array)->bytes<bool>();
            repeatedValue->_value.Reserve(count);
            auto dest = repeatedValue->_value.AddNAlreadyReserved(count);
            memcpy(dest, data, count * sizeof(bool));
        }
    }
    else
    {
        auto value = std::make_shared<LVBooleanMessageValue>(metadata->protobufIndex, *(bool*)start);
        message._values.emplace(metadata->protobufIndex, value);
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void CopyInt32FromCluster(const shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message)
{    
    if (metadata->isRepeated)
    {
        auto array = *(LV1DArrayHandle*)start;
        if (array && *array)
        {
            auto count = (*array)->cnt;
            auto repeatedValue = std::make_shared<LVRepeatedInt32MessageValue>(metadata->protobufIndex);
            message._values.emplace(metadata->protobufIndex, repeatedValue);
            auto data = (*array)->bytes<int32_t>();
            repeatedValue->_value.Reserve(count);
            auto dest = repeatedValue->_value.AddNAlreadyReserved(count);
            memcpy(dest, data, count * sizeof(int32_t));
        }
    }
    else
    {
        auto value = std::make_shared<LVInt32MessageValue>(metadata->protobufIndex, *(int*)start);
        message._values.emplace(metadata->protobufIndex, value);
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void CopyDoubleFromCluster(const shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message)
{    
    if (metadata->isRepeated)
    {
        auto array = *(LV1DArrayHandle*)start;
        if (array && *array)
        {
            auto count = (*array)->cnt;
            auto repeatedValue = std::make_shared<LVRepeatedDoubleMessageValue>(metadata->protobufIndex);
            message._values.emplace(metadata->protobufIndex, repeatedValue);
            auto data = (*array)->bytes<double>();
            repeatedValue->_value.Reserve(count);
            auto dest = repeatedValue->_value.AddNAlreadyReserved(count);
            memcpy(dest, data, count * sizeof(double));
        }
    }
    else
    {
        auto value = std::make_shared<LVDoubleMessageValue>(metadata->protobufIndex, *(double*)start);
        message._values.emplace(metadata->protobufIndex, value);
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
void CopyFloatFromCluster(const shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message)
{    
    if (metadata->isRepeated)
    {
        auto array = *(LV1DArrayHandle*)start;
        if (array && *array)
        {
            auto count = (*array)->cnt;
            auto repeatedValue = std::make_shared<LVRepeatedFloatMessageValue>(metadata->protobufIndex);
            message._values.emplace(metadata->protobufIndex, repeatedValue);
            auto data = (*array)->bytes<float>();
            repeatedValue->_value.Reserve(count);
            auto dest = repeatedValue->_value.AddNAlreadyReserved(count);
            memcpy(dest, data, count * sizeof(float));
        }
    }
    else
    {
        auto value = std::make_shared<LVFloatMessageValue>(metadata->protobufIndex, *(float*)start);
        message._values.emplace(metadata->protobufIndex, value);
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
                CopyBoolFromCluster(val.second, start, message);
                break;
            case LVMessageMetadataType::DoubleValue:
                CopyDoubleFromCluster(val.second, start, message);
                break;
            case LVMessageMetadataType::FloatValue:
                CopyFloatFromCluster(val.second, start, message);
                break;
            case LVMessageMetadataType::Int32Value:
                CopyInt32FromCluster(val.second, start, message);
                break;
            case LVMessageMetadataType::MessageValue:
                {
                    auto metadata = val.second->_owner->FindMetadata(val.second->embeddedMessageName);
                    auto nested = std::make_shared<LVMessage>(metadata->_mappedElements);
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
