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
    static void CopyStringToCluster(const shared_ptr<MessageElementMetadata> metadata, int8_t* start, const shared_ptr<LVMessageValue>& value);
    static void CopyMessageToCluster(const shared_ptr<MessageElementMetadata> metadata, int8_t* start, const shared_ptr<LVMessageValue>& value);
    static void CopyInt32ToCluster(const shared_ptr<MessageElementMetadata> metadata, int8_t* start, const shared_ptr<LVMessageValue>& value);
    static void CopyBoolToCluster(const shared_ptr<MessageElementMetadata> metadata, int8_t* start, const shared_ptr<LVMessageValue>& value);
    static void CopyDoubleToCluster(const shared_ptr<MessageElementMetadata> metadata, int8_t* start, const shared_ptr<LVMessageValue>& value);
    static void CopyFloatToCluster(const shared_ptr<MessageElementMetadata> metadata, int8_t* start, const shared_ptr<LVMessageValue>& value);

    static void CopyStringFromCluster(const shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
    static void CopyMessageFromCluster(const shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
    static void CopyBoolFromCluster(const shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
    static void CopyInt32FromCluster(const shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
    static void CopyDoubleFromCluster(const shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
    static void CopyFloatFromCluster(const shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
};
