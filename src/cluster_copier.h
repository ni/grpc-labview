//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_server.h>
#include <lv_interop.h>
#include <memory>
#include <string>
#include <map>
#include <mutex>
#include <message_metadata.h>
#include <enum_metadata.h>
#include <message_value.h>
#include <exceptions.h>

namespace grpc_labview
{
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class ClusterDataCopier
    {
    public:
        static void CopyToCluster(const LVMessage& message, int8_t* cluster);
        static void CopyFromCluster(LVMessage& message, int8_t* cluster);
        static bool AnyBuilderAddValue(LVMessage& message, LVMessageMetadataType valueType, bool isRepeated, int protobufIndex, int8_t* value);

    private:
        static void CopyStringToCluster(std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, std::shared_ptr<const LVMessageValue>& value);
        static void CopyBytesToCluster(std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, std::shared_ptr<const LVMessageValue>& value);
        static void CopyMessageToCluster(std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, std::shared_ptr<const LVMessageValue>& value);
        static void CopyInt32ToCluster(std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, std::shared_ptr<const LVMessageValue>& value);
        static void CopyUInt32ToCluster(std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, std::shared_ptr<const LVMessageValue>& value);
        static void CopyInt64ToCluster(std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, std::shared_ptr<const LVMessageValue>& value);
        static void CopyUInt64ToCluster(std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, std::shared_ptr<const LVMessageValue>& value);
        static void CopyEnumToCluster(std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, std::shared_ptr<const LVMessageValue>& value);
        static void CopyBoolToCluster(std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, std::shared_ptr<const LVMessageValue>& value);
        static void CopyDoubleToCluster(std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, std::shared_ptr<const LVMessageValue>& value);
        static void CopyFloatToCluster(std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, std::shared_ptr<const LVMessageValue>& value);
        static void CopySInt32ToCluster(std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, std::shared_ptr<const LVMessageValue>& value);
        static void CopySInt64ToCluster(std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, std::shared_ptr<const LVMessageValue>& value);
        static void CopyFixed32ToCluster(std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, std::shared_ptr<const LVMessageValue>& value);
        static void CopyFixed64ToCluster(std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, std::shared_ptr<const LVMessageValue>& value);
        static void CopySFixed32ToCluster(std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, std::shared_ptr<const LVMessageValue>& value);
        static void CopySFixed64ToCluster(std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, std::shared_ptr<const LVMessageValue>& value);

        static void CopyStringFromCluster(std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
        static void CopyBytesFromCluster(std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
        static void CopyMessageFromCluster(std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
        static void CopyBoolFromCluster(std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
        static void CopyInt32FromCluster(std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
        static void CopyUInt32FromCluster(std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
        static void CopyInt64FromCluster(std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
        static void CopyUInt64FromCluster(std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
        static void CopyEnumFromCluster(std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
        static void CopyDoubleFromCluster(std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
        static void CopyFloatFromCluster(std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
        static void CopySInt32FromCluster(std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
        static void CopySInt64FromCluster(std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
        static void CopyFixed32FromCluster(std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
        static void CopyFixed64FromCluster(std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
        static void CopySFixed32FromCluster(std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
        static void CopySFixed64FromCluster(std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
    };
}