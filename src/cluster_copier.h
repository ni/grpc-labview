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
        static void CopyStringToCluster(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<const LVMessageValue>& value);
        static void CopyBytesToCluster(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<const LVMessageValue>& value);
        static void CopyMessageToCluster(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<const LVMessageValue>& value);
        static void CopyInt32ToCluster(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<const LVMessageValue>& value);
        static void CopyUInt32ToCluster(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<const LVMessageValue>& value);
        static void CopyInt64ToCluster(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<const LVMessageValue>& value);
        static void CopyUInt64ToCluster(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<const LVMessageValue>& value);
        static void CopyEnumToCluster(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<const LVMessageValue>& value);
        static void CopyBoolToCluster(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<const LVMessageValue>& value);
        static void CopyDoubleToCluster(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<const LVMessageValue>& value);
        static void CopyFloatToCluster(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<const LVMessageValue>& value);
        static void CopySInt32ToCluster(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<const LVMessageValue>& value);
        static void CopySInt64ToCluster(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<const LVMessageValue>& value);
        static void CopyFixed32ToCluster(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<const LVMessageValue>& value);
        static void CopyFixed64ToCluster(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<const LVMessageValue>& value);
        static void CopySFixed32ToCluster(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<const LVMessageValue>& value);
        static void CopySFixed64ToCluster(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<const LVMessageValue>& value);

        static void CopyStringFromCluster(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
        static void CopyBytesFromCluster(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
        static void CopyMessageFromCluster(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
        static void CopyBoolFromCluster(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
        static void CopyInt32FromCluster(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
        static void CopyUInt32FromCluster(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
        static void CopyInt64FromCluster(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
        static void CopyUInt64FromCluster(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
        static void CopyEnumFromCluster(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
        static void CopyDoubleFromCluster(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
        static void CopyFloatFromCluster(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
        static void CopySInt32FromCluster(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
        static void CopySInt64FromCluster(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
        static void CopyFixed32FromCluster(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
        static void CopyFixed64FromCluster(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
        static void CopySFixed32FromCluster(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
        static void CopySFixed64FromCluster(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
    };
}