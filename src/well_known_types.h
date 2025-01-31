//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace grpc_labview
{
    class MessageElementMetadata;
    struct MessageMetadata;
    class LVMessage;
    class LVMessageValue;
    class IMessageElementMetadataOwner;
    class MessageElementMetadataOwner;

    namespace wellknown
    {
        //---------------------------------------------------------------------
        // Well known message types that are supported natively without having to
        // explicitly generate code from a proto file.
        //---------------------------------------------------------------------
        enum class Types
        {
            None,
            Double2DArray
        };

        //---------------------------------------------------------------------
        // Class for accessing metadata for well known types
        //---------------------------------------------------------------------
        class MetadataOwner
        {
        public:
            static MetadataOwner& GetInstance();
            std::shared_ptr<MessageMetadata> FindMetadata(const std::string& name);

        private:
            static MetadataOwner* _instance;
            static std::mutex _mutex;
            std::unique_ptr<MessageElementMetadataOwner> _owner;

            MetadataOwner();
        };

        //---------------------------------------------------------------------
        // https://github.com/ni/ni-apis/blob/main/ni/protobuf/types/array.proto
        //---------------------------------------------------------------------
        class Double2DArray
        {
        public:
            static const std::string& GetMessageName();
            static const std::string& GetMessageUrl();

            static void CopyToCluster(const grpc_labview::MessageElementMetadata& metadata, int8_t* start, const std::shared_ptr<const LVMessageValue>& value);
            static void CopyFromCluster(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message);
            static std::shared_ptr<MessageMetadata> GetMetadata(IMessageElementMetadataOwner* metadataOwner);

        private:
            static const int _rowsIndex = 1;
            static const int _columnsIndex = 2;
            static const int _dataIndex = 3;
        };
    }
}