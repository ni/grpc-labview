//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <cluster_copier.h>
#include <lv_interop.h>
#include <google/protobuf/any.pb.h>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t PackToAny(LVgRPCid id, const char* messageType, int8_t* cluster, LVgRPCid* anyResult)
{
    auto any = new google::protobuf::Any();

    auto metadataOwner = (IMessageElementMetadataOwner*)id;
    auto metadata = metadataOwner->FindMetadata(messageType);
    if (metadata == nullptr)
    {
        return -2;
    }

    LVMessage message(metadata);
    ClusterDataCopier::CopyFromCluster(message, cluster);
    std::string buffer;
    if (message.SerializeToString(&buffer))
    {
        any->set_value(buffer);
        any->set_type_url(messageType);
        *anyResult = any;
        return 0;
    }
    return -2;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t FreeAny(LVgRPCid anyId, int8_t* lvRequest)
{
    auto any = (google::protobuf::Any*)anyId;
    delete any;
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t UnpackFromAny(LVgRPCid id, LVgRPCid anyId, const char* messageType, int8_t* cluster)
{
    auto any = (google::protobuf::Any*)anyId;
    auto metadataOwner = (IMessageElementMetadataOwner*)id;
    auto metadata = metadataOwner->FindMetadata(messageType);

    if (metadata == nullptr)
    {
        return -2;
    }  
    LVMessage message(metadata);
    if (message.ParseFromString(any->value()))
    {
        ClusterDataCopier::CopyToCluster(message, cluster);
        return 0;
    }
    return -2;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t TryUnpackFromAny(LVgRPCid id, LVgRPCid anyId, const char* messageType, int8_t* cluster)
{    
    auto any = (google::protobuf::Any*)anyId;
    auto metadataOwner = (IMessageElementMetadataOwner*)id;
    auto metadata = metadataOwner->FindMetadata(messageType);

    if (metadata == nullptr)
    {
        return -2;
    }
    if (any->type_url() != messageType)
    {
        return -1;
    }
    LVMessage message(metadata);
    if (message.ParseFromString(any->value()))
    {
        ClusterDataCopier::CopyToCluster(message, cluster);
        return 0;
    }
    return -2;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t IsAnyOfType(LVgRPCid anyId, const char* messageType)
{    
    auto any = (google::protobuf::Any*)anyId;
    if (any->type_url() != messageType)
    {
        return -1;
    }
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t AnyBuilderBegin(LVgRPCid* builderId)
{   
    auto metadata = std::make_shared<MessageMetadata>();
    auto rootMessage = new LVMessage(metadata);
    *builderId = rootMessage;
    return 0; 
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t AnyBuilderAddValue(LVgRPCid anyId, LVMessageMetadataType valueType, bool isRepeated, int protobufIndex, int8_t* value)
{
    auto message = (LVMessage*)anyId;
    ClusterDataCopier::AnyBuilderAddValue(*message, valueType, isRepeated, protobufIndex, value);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t AnyBuilderBeginNestedMessage(LVgRPCid builderId, int protobufIndex, LVgRPCid* nestedId)
{   
    auto message = (LVMessage*)builderId;
    auto metadata = std::make_shared<MessageMetadata>();

    auto nested = std::make_shared<LVMessage>(metadata);
    auto value = std::make_shared<LVNestedMessageMessageValue>(protobufIndex, nested);
    message->_values.emplace(protobufIndex, value);
    *nestedId = nested.get();
    return 0; 
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t AnyBuilderBeginRepeatedNestedMessage(LVgRPCid builderId, int protobufIndex, LVgRPCid* nestedId)
{   
    auto message = (LVMessage*)builderId;
    auto metadata = std::make_shared<MessageMetadata>();

    auto value = std::make_shared<LVRepeatedNestedMessageMessageValue>(protobufIndex);
    message->_values.emplace(protobufIndex, value);
    *nestedId = value.get();
    return 0; 
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t AnyBuilderBeginRepeatedNestedMessageElement(LVgRPCid builderId, int protobufIndex, LVgRPCid* nestedId)
{   
    auto message = (LVRepeatedNestedMessageMessageValue*)builderId;
    auto metadata = std::make_shared<MessageMetadata>();

    auto nested = std::make_shared<LVMessage>(metadata);
    message->_value.emplace_back(nested);
    *nestedId = nested.get();
    return 0; 
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t AnyBuilderBuild(LVgRPCid builderId, const char* typeUrl, LVgRPCid* anyId)
{   
    auto message = (LVMessage*)builderId;
    auto any = new google::protobuf::Any();

    std::string buffer;
    if (message->SerializeToString(&buffer))
    {
        any->set_value(buffer);
        any->set_type_url(typeUrl);
        *anyId = any;
        delete message;
        return 0;
    }
    delete message;
    return -2;
}
