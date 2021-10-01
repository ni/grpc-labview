//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_server.h>
#include <lv_interop.h>
#include <memory>
#include <string>
#include <map>
#include <mutex>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class ClusterDataCopier
{
public:
    static void CopyToCluster(const LVMessage& message, int8_t* cluster);
    static void CopyFromCluster(LVMessage& message, int8_t* cluster);

private:
    static void CopyStringToCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<LVMessageValue>& value);
    static void CopyMessageToCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<LVMessageValue>& value);
    static void CopyInt32ToCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<LVMessageValue>& value);
    static void CopyUInt32ToCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<LVMessageValue>& value);
    static void CopyInt64ToCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<LVMessageValue>& value);
    static void CopyUInt64ToCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<LVMessageValue>& value);
    static void CopyEnumToCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<LVMessageValue>& value);
    static void CopyBoolToCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<LVMessageValue>& value);
    static void CopyDoubleToCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<LVMessageValue>& value);
    static void CopyFloatToCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<LVMessageValue>& value);

    static void CopyStringFromCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
    static void CopyMessageFromCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
    static void CopyBoolFromCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
    static void CopyInt32FromCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
    static void CopyUInt32FromCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
    static void CopyInt64FromCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
    static void CopyUInt64FromCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
    static void CopyEnumFromCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
    static void CopyDoubleFromCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
    static void CopyFloatFromCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
};
