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

            virtual void CopyFromMessageToCluster(const grpc_labview::MessageElementMetadata& metadata, const std::shared_ptr<const LVMessageValue>& value, int8_t* start)
            {
                // LV doesn't support arrays of arrays and we don't currently support automatically wrapping the 2D array
                // in a cluster like we do with a repeated bytes field.
                assert(!metadata.isRepeated);

                auto& my2DArrayMessage = ((LVNestedMessageMessageValue*)value.get())->_value;

                int rows = 0;
                auto it = my2DArrayMessage->_values.find(_rowsIndex);
                if (it != my2DArrayMessage->_values.end())
                {
                    rows = std::static_pointer_cast<LVVariableMessageValue<int>>(it->second)->_value;
                }

                int columns = 0;
                it = my2DArrayMessage->_values.find(_columnsIndex);
                if (it != my2DArrayMessage->_values.end())
                {
                    columns = std::static_pointer_cast<LVVariableMessageValue<int>>(it->second)->_value;
                }

                std::shared_ptr<LVRepeatedMessageValue<TRepeatedType>> dataValue = nullptr;
                it = my2DArrayMessage->_values.find(_dataIndex);
                if (it != my2DArrayMessage->_values.end())
                {
                    dataValue = std::static_pointer_cast<LVRepeatedMessageValue<TRepeatedType>>(it->second);
                }

                auto elementCount = rows * columns;
                if (elementCount > 0 && dataValue != nullptr)
                {
                    CopyArrayFromMessageToCluster(rows, columns, dataValue, start);
                }
            }

            virtual void CopyFromClusterToMessage(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message)
            {
                // LV doesn't support arrays of arrays and we don't currently support automatically wrapping the 2D array
                // in a cluster like we do with a repeated bytes field.
                assert(!metadata->isRepeated);

                auto array = *(LV2DArrayHandle*)start;
                if (array == nullptr || *array == nullptr)
                {
                    return;
                }

                int rows = (*array)->dimensionSizes[0];
                int columns = (*array)->dimensionSizes[1];
                if (rows == 0 || columns == 0)
                {
                    return;
                }

                // Convert from native 2D array in LV to equivalent of protobuf message on the wire
                auto arrayMetadata = metadata->_owner->FindMetadata(metadata->embeddedMessageName);
                auto arrayMessage = std::make_shared<LVMessage>(arrayMetadata);

                // Add field values to message
                auto rowsValue = std::make_shared<LVVariableMessageValue<int>>(_rowsIndex, rows);
                arrayMessage->_values.emplace(_rowsIndex, rowsValue);
                auto columnsValue = std::make_shared<LVVariableMessageValue<int>>(_columnsIndex, columns);
                arrayMessage->_values.emplace(_columnsIndex, columnsValue);
                auto dataValue = std::make_shared<LVRepeatedMessageValue<TRepeatedType>>(_dataIndex);
                arrayMessage->_values.emplace(_dataIndex, dataValue);

                CopyArrayFromClusterToMessage(rows * columns, array, dataValue);

                auto messageValue = std::make_shared<LVNestedMessageMessageValue>(metadata->protobufIndex, arrayMessage);
                message._values.emplace(metadata->protobufIndex, messageValue);
            }

            virtual std::shared_ptr<MessageMetadata> GetMetadata(IMessageElementMetadataOwner* metadataOwner)
            {
                auto messageMetadata = std::make_shared<MessageMetadata>();
                messageMetadata->messageName = GetMessageName();
                messageMetadata->typeUrl = GetMessageUrl();

                auto rowsMetadata = std::make_shared<MessageElementMetadata>(LVMessageMetadataType::Int32Value, false, _rowsIndex);
                rowsMetadata->fieldName = "rows";
                rowsMetadata->_owner = metadataOwner;
                auto columnsMetadata = std::make_shared<MessageElementMetadata>(LVMessageMetadataType::Int32Value, false, _columnsIndex);
                columnsMetadata->fieldName = "columns";
                columnsMetadata->_owner = metadataOwner;
                auto dataMetadata = std::make_shared<MessageElementMetadata>(_elementMetadataType, true, _dataIndex);
                dataMetadata->fieldName = "data";
                dataMetadata->_owner = metadataOwner;

                messageMetadata->_elements.push_back(rowsMetadata);
                messageMetadata->_elements.push_back(columnsMetadata);
                messageMetadata->_elements.push_back(dataMetadata);
                messageMetadata->_mappedElements.emplace(rowsMetadata->protobufIndex, rowsMetadata);
                messageMetadata->_mappedElements.emplace(columnsMetadata->protobufIndex, columnsMetadata);
                messageMetadata->_mappedElements.emplace(dataMetadata->protobufIndex, dataMetadata);

                return messageMetadata;
            }

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