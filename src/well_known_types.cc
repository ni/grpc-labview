#include <well_known_types.h>
#include <lv_message.h>
#include <message_metadata.h>
#include <metadata_owner.h>

namespace grpc_labview
{
    namespace wellknown
    {
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        MetadataOwner* MetadataOwner::_instance = nullptr;
        std::mutex MetadataOwner::_mutex;

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        MetadataOwner& MetadataOwner::GetInstance()
        {
            std::unique_lock<std::mutex> lock(_mutex);
            if (_instance == nullptr)
            {
                _instance = new MetadataOwner();
                auto& owner = _instance->_owner;
                owner->RegisterMetadata(Double2DArray::GetMetadata(owner.get()));
                owner->FinalizeMetadata();
            }
            return *_instance;
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        MetadataOwner::MetadataOwner()
        {
            _owner = std::make_unique<MessageElementMetadataOwner>();
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        std::shared_ptr<MessageMetadata> MetadataOwner::FindMetadata(const std::string& name)
        {
            return _owner->FindLocalMetadata(name);
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        const std::string& Double2DArray::GetMessageName()
        {
            static const std::string messageName = "ni_protobuf_types_Double2DArray";
            return messageName;
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        const std::string& Double2DArray::GetMessageUrl()
        {
            static const std::string messageUrl = "ni.protobuf.types.Double2DArray";
            return messageUrl;
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        void Double2DArray::CopyFromCluster(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message)
        {
            // LV doesn't support arrays of arrays. CopyMessageFromCluster should be used instead to copy/serialize based
            // proto/cluster data type rather than native LV array data type.
            assert(!metadata->isRepeated);
            assert(metadata->wellKnownType == wellknown::Types::Double2DArray);

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
            auto nestedMetadata = metadata->_owner->FindMetadata(metadata->embeddedMessageName);
            auto nested = std::make_shared<LVMessage>(nestedMetadata);

            // Add field values to message
            auto rowsValue = std::make_shared<LVVariableMessageValue<int>>(_rowsIndex, rows);
            nested->_values.emplace(_rowsIndex, rowsValue);
            auto columnsValue = std::make_shared<LVVariableMessageValue<int>>(_columnsIndex, columns);
            nested->_values.emplace(_columnsIndex, columnsValue);
            auto dataValue = std::make_shared<LVRepeatedMessageValue<double>>(_dataIndex);
            nested->_values.emplace(_dataIndex, dataValue);

            // Copy data from LV array to message
            auto arrayData = (*array)->bytes<double>();
            auto totalElements = rows * columns;
            dataValue->_value.Reserve(totalElements);
            auto dest = dataValue->_value.AddNAlreadyReserved(totalElements);
            memcpy(dest, arrayData, totalElements * sizeof(double));

            auto messageValue = std::make_shared<LVNestedMessageMessageValue>(metadata->protobufIndex, nested);
            message._values.emplace(metadata->protobufIndex, messageValue);
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        void Double2DArray::CopyToCluster(const MessageElementMetadata& metadata, int8_t* start, const std::shared_ptr<const LVMessageValue>& value)
        {
            // LV doesn't support arrays of arrays. CopyMessageFromCluster should be used instead to copy/serialize based
            // proto/cluster data type rather than native LV array data type.
            assert(!metadata.isRepeated);
            assert(metadata.wellKnownType == wellknown::Types::Double2DArray);

            auto& double2DArrayMessage = ((LVNestedMessageMessageValue*)value.get())->_value;

            int rows = 0;
            auto it = double2DArrayMessage->_values.find(_rowsIndex);
            if (it != double2DArrayMessage->_values.end())
            {
                rows = std::static_pointer_cast<LVVariableMessageValue<int>>(it->second)->_value;
            }

            int columns = 0;
            it = double2DArrayMessage->_values.find(_columnsIndex);
            if (it != double2DArrayMessage->_values.end())
            {
                columns = std::static_pointer_cast<LVVariableMessageValue<int>>(it->second)->_value;
            }

            std::shared_ptr<LVRepeatedMessageValue<double>> dataValue = nullptr;
            it = double2DArrayMessage->_values.find(_dataIndex);
            if (it != double2DArrayMessage->_values.end())
            {
                dataValue = std::static_pointer_cast<LVRepeatedMessageValue<double>>(it->second);
            }

            auto elementCount = rows * columns;
            if (elementCount > 0)
            {
                NumericArrayResize(GetTypeCodeForSize(sizeof(double)), 2, start, elementCount);
                auto array = *(LV2DArrayHandle*)start;
                (*array)->dimensionSizes[0] = rows;
                (*array)->dimensionSizes[1] = columns;
                // Protect against a malformed message where the amount of data sent doesn't match the dimension sizes.
                // LV will automatically pad/handle writing less data than was allocated to the array so we just need
                // to make sure we don't write more data than was allocated to the array.
                auto lvByteCount = elementCount * sizeof(double);
                auto protobufByteCount = dataValue->_value.size() * sizeof(double);
                memcpy((*array)->bytes<double>(), dataValue->_value.data(), std::min(lvByteCount, protobufByteCount));
            }
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        std::shared_ptr<MessageMetadata> Double2DArray::GetMetadata(IMessageElementMetadataOwner* metadataOwner)
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
            auto dataMetadata = std::make_shared<MessageElementMetadata>(LVMessageMetadataType::DoubleValue, true, _dataIndex);
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
    }
}