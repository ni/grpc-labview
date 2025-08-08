//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <cluster_copier.h>
#include <lv_interop.h>
#include <google/protobuf/any.pb.h>
#include <serialization_session.h>
#include <lv_message.h>
#include <string_utils.h>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t CreateSerializationSession(grpc_labview::gRPCid** sessionId)
{
    try {
        grpc_labview::InitCallbacks();
        auto session = new grpc_labview::LabVIEWSerializationSession();
        grpc_labview::gPointerManager.RegisterPointer(session);
        *sessionId = session;
        return 0;
    } catch (const std::exception&) {
        return grpc_labview::TranslateException();
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t FreeSerializationSession(grpc_labview::gRPCid* sessionId)
{
    try {
        auto session = sessionId->CastTo<grpc_labview::LabVIEWSerializationSession>();
        grpc_labview::gPointerManager.UnregisterPointer(sessionId);
        return 0;
    } catch (const std::exception&) {
        return grpc_labview::TranslateException();
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t PackToBuffer(grpc_labview::gRPCid* id, const char* messageType, int8_t* cluster, grpc_labview::LV1DArrayHandle* lvBuffer)
{
    try {
        auto metadataOwner = id->CastTo<grpc_labview::IMessageElementMetadataOwner>();
        if (!metadataOwner)
        {
            return -1;
        }

        if (!grpc_labview::VerifyAsciiString(messageType))
        {
            return -2;
        }

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
            NumericArrayResize(0x01, 1, lvBuffer, buffer.length());
            (**lvBuffer)->cnt = buffer.length();
            uint8_t* elements = (**lvBuffer)->bytes<uint8_t>();
            memcpy(elements, buffer.c_str(), buffer.length());
            return 0;
        }
        return -2;
    } catch (const std::exception&) {
        return grpc_labview::TranslateException();
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t PackToAny(grpc_labview::gRPCid* id, const char* messageType, int8_t* cluster, grpc_labview::AnyCluster* anyCluster)
{
    try {
        auto metadataOwner = id->CastTo<grpc_labview::IMessageElementMetadataOwner>();
        if (!metadataOwner)
        {
            return -1;
        }

        if (!grpc_labview::VerifyAsciiString(messageType))
        {
            return -2;
        }

        auto metadata = metadataOwner->FindMetadata(messageType);
        if (metadata == nullptr)
        {
            return -2;
        }

        SetLVAsciiString(&anyCluster->TypeUrl, "/" + metadata->typeUrl);
        return PackToBuffer(id, messageType, cluster, &anyCluster->Bytes);
    } catch (const std::exception&) {
        return grpc_labview::TranslateException();
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t UnpackFromBuffer(grpc_labview::gRPCid* id, grpc_labview::LV1DArrayHandle lvBuffer, const char* messageType, int8_t* cluster)
{
    try {
        auto metadataOwner = id->CastTo<grpc_labview::IMessageElementMetadataOwner>();
        if (!metadataOwner)
        {
            return -1;
        }

        if (!grpc_labview::VerifyAsciiString(messageType))
        {
            return -2;
        }

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
    } catch (const std::exception&) {
        return grpc_labview::TranslateException();
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t UnpackFromAny(grpc_labview::gRPCid* id, grpc_labview::AnyCluster* anyCluster, const char* messageType, int8_t* cluster)
{
    try {
        return UnpackFromBuffer(id, anyCluster->Bytes, messageType, cluster);
    } catch (const std::exception&) {
        return grpc_labview::TranslateException();
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t TryUnpackFromAny(grpc_labview::gRPCid* id, grpc_labview::AnyCluster* anyCluster, const char* messageType, int8_t* cluster)
{
    try {
        auto metadataOwner = id->CastTo<grpc_labview::IMessageElementMetadataOwner>();
        if (!metadataOwner)
        {
            return -1;
        }

        if (!grpc_labview::VerifyAsciiString(messageType))
        {
            return -2;
        }

        auto metadata = metadataOwner->FindMetadata(messageType);
        if (metadata == nullptr)
        {
            return -2;
        }

        if (grpc_labview::GetLVAsciiString(anyCluster->TypeUrl) != messageType)
        {
            return -1;
        }
        return UnpackFromBuffer(id, anyCluster->Bytes, messageType, cluster);
    } catch (const std::exception&) {
        return grpc_labview::TranslateException();
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t IsAnyOfType(grpc_labview::gRPCid* id, grpc_labview::AnyCluster* anyCluster, const char* messageType)
{
    try {
        auto metadataOwner = id->CastTo<grpc_labview::IMessageElementMetadataOwner>();
        if (!metadataOwner)
        {
            return -1;
        }

        if (!grpc_labview::VerifyAsciiString(messageType))
        {
            return -2;
        }

        auto metadata = metadataOwner->FindMetadata(messageType);
        if (metadata == nullptr)
        {
            return -2;
        }

        if (grpc_labview::GetLVAsciiString(anyCluster->TypeUrl) != metadata->typeUrl)
        {
            return -1;
        }
        return 0;
    } catch (const std::exception&) {
        return grpc_labview::TranslateException();
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t AnyBuilderBegin(grpc_labview::gRPCid** builderId)
{
    try {
        grpc_labview::InitCallbacks();

        auto metadata = std::make_shared<grpc_labview::MessageMetadata>();
        auto rootMessage = new grpc_labview::LVMessage(metadata);
        grpc_labview::gPointerManager.RegisterPointer(rootMessage);
        *builderId = rootMessage;
        return 0;
    } catch (const std::exception&) {
        return grpc_labview::TranslateException();
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t AnyBuilderAddValue(grpc_labview::gRPCid* anyId, grpc_labview::LVMessageMetadataType valueType, int isRepeated, int protobufIndex, int8_t* value)
{
    try {
        auto message = anyId->CastTo<grpc_labview::LVMessage>();
        if (!message)
        {
            return -1;
        }

        grpc_labview::ClusterDataCopier::AnyBuilderAddValue(*message, valueType, isRepeated, protobufIndex, value);
        return 0;
    } catch (const std::exception&) {
        return grpc_labview::TranslateException();
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t AnyBuilderBeginNestedMessage(grpc_labview::gRPCid* builderId, int protobufIndex, grpc_labview::gRPCid** nestedId)
{
    try {
        auto message = builderId->CastTo<grpc_labview::LVMessage>();
        if (!message)
        {
            return -1;
        }

        auto metadata = std::make_shared<grpc_labview::MessageMetadata>();
        auto nested = std::make_shared<grpc_labview::LVMessage>(metadata);
        auto value = std::make_shared<grpc_labview::LVNestedMessageMessageValue>(protobufIndex, nested);
        message->_values.emplace(protobufIndex, value);
        *nestedId = nested.get();
        return 0;
    } catch (const std::exception&) {
        return grpc_labview::TranslateException();
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t AnyBuilderBeginRepeatedNestedMessage(grpc_labview::gRPCid* builderId, int protobufIndex, grpc_labview::gRPCid** nestedId)
{
    try {
        auto message = builderId->CastTo<grpc_labview::LVMessage>();
        if (!message)
        {
            return -1;
        }

        auto metadata = std::make_shared<grpc_labview::MessageMetadata>();
        auto value = std::make_shared<grpc_labview::LVRepeatedNestedMessageMessageValue>(protobufIndex);
        message->_values.emplace(protobufIndex, value);
        *nestedId = value.get();
        return 0;
    } catch (const std::exception&) {
        return grpc_labview::TranslateException();
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t AnyBuilderBeginRepeatedNestedMessageElement(grpc_labview::gRPCid* builderId, grpc_labview::gRPCid** nestedId)
{
    try {
        auto message = builderId->CastTo<grpc_labview::LVRepeatedNestedMessageMessageValue>();
        if (!message)
        {
            return -1;
        }

        auto metadata = std::make_shared<grpc_labview::MessageMetadata>();

        auto nested = std::make_shared<grpc_labview::LVMessage>(metadata);
        message->_value.emplace_back(nested);
        *nestedId = nested.get();
        return 0;
    } catch (const std::exception&) {
        return grpc_labview::TranslateException();
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t AnyBuilderBuildToBuffer(grpc_labview::gRPCid* builderId, const char* typeUrl, grpc_labview::LV1DArrayHandle* lvBuffer)
{
    try {
        auto message = builderId->CastTo<grpc_labview::LVMessage>();
        if (!message)
        {
            return -1;
        }

        // The typeUrl parameter is unused.

        grpc_labview::gPointerManager.UnregisterPointer(builderId);
        std::string buffer;
        if (message->SerializeToString(&buffer))
        {
            grpc_labview::NumericArrayResize(0x01, 1, lvBuffer, buffer.length());
            (**lvBuffer)->cnt = buffer.length();
            uint8_t* elements = (**lvBuffer)->bytes<uint8_t>();
            memcpy(elements, buffer.c_str(), buffer.length());
            return 0;
        }
        return -2;
    } catch (const std::exception&) {
        return grpc_labview::TranslateException();
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t AnyBuilderBuild(grpc_labview::gRPCid* builderId, const char* typeUrl, grpc_labview::AnyCluster* anyCluster)
{
    try {
        auto message = builderId->CastTo<grpc_labview::LVMessage>();
        if (!message)
        {
            return -1;
        }

        // The typeUrl parameter is unused.

        grpc_labview::SetLVAsciiString(&anyCluster->TypeUrl, message->_metadata->typeUrl);
        return AnyBuilderBuildToBuffer(builderId, typeUrl, &anyCluster->Bytes);
    } catch (const std::exception&) {
        return grpc_labview::TranslateException();
    }
}
