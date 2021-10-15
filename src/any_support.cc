//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <cluster_copier.h>
#include <lv_interop.h>
#include <google/protobuf/any.pb.h>
#include <serialization_session.h>
#include <lv_message.h>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t CreateSerializationSession(grpc_labview::LVgRPCid** sessionId)
{
    grpc_labview::InitCallbacks();
    auto session = new grpc_labview::LabVIEWSerializationSession();
    *sessionId = session;
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t FreeSerializationSession(grpc_labview::LVgRPCid* sessionId)
{
    auto session = sessionId->CastTo<grpc_labview::LabVIEWSerializationSession>();
    delete session;
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t PackToBuffer(grpc_labview::LVgRPCid* id, const char* messageType, int8_t* cluster, grpc_labview::LV1DArrayHandle* lvBuffer)
{
    auto metadataOwner = id->CastTo<grpc_labview::IMessageElementMetadataOwner>();
    auto metadata = metadataOwner->FindMetadata(messageType);
    if (metadata == nullptr)
    {
        return -2;
    }

    grpc_labview::LVMessage message(metadata);
    grpc_labview::ClusterDataCopier::CopyFromCluster(message, cluster);
    std::string buffer;
    if (message.SerializeToString(&buffer))
    {
        LVNumericArrayResize(0x01, 1, lvBuffer, buffer.length());
        (**lvBuffer)->cnt = buffer.length();
        uint8_t* elements = (**lvBuffer)->bytes<uint8_t>();
        memcpy(elements, buffer.c_str(), buffer.length());
        return 0;
    }
    return -2;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t PackToAny(grpc_labview::LVgRPCid* id, const char* messageType, int8_t* cluster, grpc_labview::LVAny* lvAny)
{
    auto metadataOwner = id->CastTo<grpc_labview::IMessageElementMetadataOwner>();
    auto metadata = metadataOwner->FindMetadata(messageType);
    if (metadata == nullptr)
    {
        return -2;
    }

    SetLVString(&lvAny->TypeUrl, "/" + metadata->typeUrl);
    return PackToBuffer(id, messageType, cluster, &lvAny->Bytes);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t UnpackFromBuffer(grpc_labview::LVgRPCid* id, grpc_labview::LV1DArrayHandle lvBuffer, const char* messageType, int8_t* cluster)
{
    auto metadataOwner = id->CastTo<grpc_labview::IMessageElementMetadataOwner>();
    auto metadata = metadataOwner->FindMetadata(messageType);

    if (metadata == nullptr)
    {
        return -2;
    }  
    grpc_labview::LVMessage message(metadata);
    char* elements = (*lvBuffer)->bytes<char>();
    std::string buffer(elements, (*lvBuffer)->cnt);
    if (message.ParseFromString(buffer))
    {
        grpc_labview::ClusterDataCopier::CopyToCluster(message, cluster);
        return 0;
    }
    return -2;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t UnpackFromAny(grpc_labview::LVgRPCid* id, grpc_labview::LVAny* lvAny, const char* messageType, int8_t* cluster)
{
    return UnpackFromBuffer(id, lvAny->Bytes, messageType, cluster);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t TryUnpackFromAny(grpc_labview::LVgRPCid* id, grpc_labview::LVAny* lvAny, const char* messageType, int8_t* cluster)
{    
    auto metadataOwner = id->CastTo<grpc_labview::IMessageElementMetadataOwner>();
    auto metadata = metadataOwner->FindMetadata(messageType);

    if (metadata == nullptr)
    {
        return -2;
    }
    if (grpc_labview::GetLVString(lvAny->TypeUrl) != messageType)
    {
        return -1;
    }
    return UnpackFromBuffer(id, lvAny->Bytes, messageType, cluster);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t IsAnyOfType(grpc_labview::LVgRPCid* id, grpc_labview::LVAny* lvAny, const char* messageType)
{    
    auto metadataOwner = id->CastTo<grpc_labview::IMessageElementMetadataOwner>();
    auto metadata = metadataOwner->FindMetadata(messageType);

    if (grpc_labview::GetLVString(lvAny->TypeUrl) != metadata->typeUrl)
    {
        return -1;
    }
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t AnyBuilderBegin(grpc_labview::LVgRPCid** builderId)
{   
    grpc_labview::InitCallbacks();

    auto metadata = std::make_shared<grpc_labview::MessageMetadata>();
    auto rootMessage = new grpc_labview::LVMessage(metadata);
    *builderId = rootMessage;
    return 0; 
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t AnyBuilderAddValue(grpc_labview::LVgRPCid* anyId, grpc_labview::LVMessageMetadataType valueType, int isRepeated, int protobufIndex, int8_t* value)
{
    auto message = anyId->CastTo<grpc_labview::LVMessage>();
    grpc_labview::ClusterDataCopier::AnyBuilderAddValue(*message, valueType, isRepeated, protobufIndex, value);
    return 0;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t AnyBuilderBeginNestedMessage(grpc_labview::LVgRPCid* builderId, int protobufIndex, grpc_labview::LVgRPCid** nestedId)
{   
    auto message = builderId->CastTo<grpc_labview::LVMessage>();
    auto metadata = std::make_shared<grpc_labview::MessageMetadata>();

    auto nested = std::make_shared<grpc_labview::LVMessage>(metadata);
    auto value = std::make_shared<grpc_labview::LVNestedMessageMessageValue>(protobufIndex, nested);
    message->_values.emplace(protobufIndex, value);
    *nestedId = nested.get();
    return 0; 
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t AnyBuilderBeginRepeatedNestedMessage(grpc_labview::LVgRPCid* builderId, int protobufIndex, grpc_labview::LVgRPCid** nestedId)
{   
    auto message = builderId->CastTo<grpc_labview::LVMessage>();
    auto metadata = std::make_shared<grpc_labview::MessageMetadata>();

    auto value = std::make_shared<grpc_labview::LVRepeatedNestedMessageMessageValue>(protobufIndex);
    message->_values.emplace(protobufIndex, value);
    *nestedId = value.get();
    return 0; 
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t AnyBuilderBeginRepeatedNestedMessageElement(grpc_labview::LVgRPCid* builderId, grpc_labview::LVgRPCid** nestedId)
{   
    auto message = builderId->CastTo<grpc_labview::LVRepeatedNestedMessageMessageValue>();
    auto metadata = std::make_shared<grpc_labview::MessageMetadata>();

    auto nested = std::make_shared<grpc_labview::LVMessage>(metadata);
    message->_value.emplace_back(nested);
    *nestedId = nested.get();
    return 0; 
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t AnyBuilderBuildToBuffer(grpc_labview::LVgRPCid* builderId, const char* typeUrl, grpc_labview::LV1DArrayHandle* lvBuffer)
{   
    auto message = builderId->CastTo<grpc_labview::LVMessage>();
    std::string buffer;
    if (message->SerializeToString(&buffer))
    {
        grpc_labview::LVNumericArrayResize(0x01, 1, lvBuffer, buffer.length());
        (**lvBuffer)->cnt = buffer.length();
        uint8_t* elements = (**lvBuffer)->bytes<uint8_t>();
        memcpy(elements, buffer.c_str(), buffer.length());
        delete message;
        return 0;
    }
    delete message;
    return -2;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t AnyBuilderBuild(grpc_labview::LVgRPCid* builderId, const char* typeUrl, grpc_labview::LVAny* lvAny)
{   
    auto message = builderId->CastTo<grpc_labview::LVMessage>();
    grpc_labview::SetLVString(&lvAny->TypeUrl, message->_metadata->typeUrl);
    return AnyBuilderBuildToBuffer(builderId, typeUrl, &lvAny->Bytes);
}
