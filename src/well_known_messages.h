//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <lv_message.h>
#include <message_metadata.h>
#include <message_value.h>
#include <metadata_owner.h>
#include <well_known_types.h>

namespace grpc_labview
{
    namespace wellknown
    {
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
        //---------------------------------------------------------------------
        class I2DArray
        {
        public:
            virtual const std::string& GetMessageName() = 0;
            virtual const std::string& GetMessageUrl() = 0;

            virtual void CopyFromMessageToCluster(const grpc_labview::MessageElementMetadata& metadata, const std::shared_ptr<const LVMessageValue>& value, int8_t* start) = 0;
            virtual void CopyFromClusterToMessage(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message) = 0;
            virtual std::shared_ptr<MessageMetadata> GetMetadata(IMessageElementMetadataOwner* metadataOwner) = 0;
            virtual ~I2DArray() = default;

        protected:
            static const int _rowsIndex = 1;
            static const int _columnsIndex = 2;
            static const int _dataIndex = 3;
        };

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        template <typename TRepeatedType>
        class Base2DArray : public I2DArray
        {
        public:
            Base2DArray(LVMessageMetadataType elementMetadataType) : _elementMetadataType(elementMetadataType) {}

            virtual void CopyFromMessageToCluster(const grpc_labview::MessageElementMetadata& metadata, const std::shared_ptr<const LVMessageValue>& value, int8_t* start) override;
            virtual void CopyFromClusterToMessage(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message) override;
            virtual std::shared_ptr<MessageMetadata> GetMetadata(IMessageElementMetadataOwner* metadataOwner) override;
            virtual ~Base2DArray() = default;

        protected:
            virtual void CopyArrayFromMessageToCluster(int rows, int columns, const std::shared_ptr<const LVMessageValue>& dataFieldValue, int8_t* start) = 0;
            virtual void CopyArrayFromClusterToMessage(int totalElements, LV2DArrayHandle array, const std::shared_ptr<LVMessageValue>& dataFieldValue) = 0;

        private:
            LVMessageMetadataType _elementMetadataType;
        };

        //---------------------------------------------------------------------
        // https://github.com/ni/ni-apis/blob/main/ni/protobuf/types/array.proto
        //---------------------------------------------------------------------
        class Double2DArray : public Base2DArray<double>
        {
        public:
            Double2DArray() : Base2DArray(LVMessageMetadataType::DoubleValue) {}
            static Double2DArray& GetInstance()
            {
                static Double2DArray instance;
                return instance;
            }

            const std::string& GetMessageName() override;
            const std::string& GetMessageUrl() override;

        protected:
            void CopyArrayFromMessageToCluster(int rows, int columns, const std::shared_ptr<const LVMessageValue>& dataFieldValue, int8_t* start) override;
            void CopyArrayFromClusterToMessage(int totalElements, LV2DArrayHandle array, const std::shared_ptr<LVMessageValue>& dataFieldValue) override;
        };

        //---------------------------------------------------------------------
        // https://github.com/ni/ni-apis/blob/main/ni/protobuf/types/array.proto
        //---------------------------------------------------------------------
        class String2DArray : public Base2DArray<std::string>
        {
        public:
            String2DArray() : Base2DArray(LVMessageMetadataType::StringValue) {}
            static String2DArray& GetInstance()
            {
                static String2DArray instance;
                return instance;
            }

            const std::string& GetMessageName() override;
            const std::string& GetMessageUrl() override;

        protected:
            void CopyArrayFromMessageToCluster(int rows, int columns, const std::shared_ptr<const LVMessageValue>& dataFieldValue, int8_t* start) override;
            void CopyArrayFromClusterToMessage(int totalElements, LV2DArrayHandle array, const std::shared_ptr<LVMessageValue>& dataFieldValue) override;
        };
    }
}