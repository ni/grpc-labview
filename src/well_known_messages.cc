#include <well_known_messages.h>

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
                auto owner = _instance->_owner.get();
                owner->RegisterMetadata(Double2DArray::GetInstance().GetMetadata(owner));
                owner->RegisterMetadata(String2DArray::GetInstance().GetMetadata(owner));
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
        template<typename TRepeatedType>
        std::shared_ptr<MessageMetadata> Base2DArray<TRepeatedType>::GetMetadata(IMessageElementMetadataOwner* metadataOwner)
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

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        template<typename TRepeatedType>
        void Base2DArray<TRepeatedType>::CopyFromMessageToCluster(const grpc_labview::MessageElementMetadata& metadata, const std::shared_ptr<const LVMessageValue>& value, int8_t* start)
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

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        template<typename TRepeatedType>
        void Base2DArray<TRepeatedType>::CopyFromClusterToMessage(const std::shared_ptr<const MessageElementMetadata> metadata, int8_t* start, LVMessage& message)
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
        void Double2DArray::CopyArrayFromMessageToCluster(int rows, int columns, const std::shared_ptr<const LVMessageValue>& dataFieldValue, int8_t* start)
        {
            auto elementCount = rows * columns;
            NumericArrayResize(GetTypeCodeForSize(sizeof(double)), 2, start, elementCount);
            auto array = *(LV2DArrayHandle*)start;
            (*array)->dimensionSizes[0] = rows;
            (*array)->dimensionSizes[1] = columns;
            // Protect against a malformed message where the amount of data sent doesn't match the dimension sizes.
            // LV will automatically pad/handle writing less data than was allocated to the array so we just need
            // to make sure we don't write more data than was allocated to the array.
            auto dataValue = std::static_pointer_cast<const LVRepeatedMessageValue<double>>(dataFieldValue);
            auto lvByteCount = elementCount * sizeof(double);
            auto protobufByteCount = dataValue->_value.size() * sizeof(double);
            memcpy((*array)->bytes<double>(), dataValue->_value.data(), std::min(lvByteCount, protobufByteCount));
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        void Double2DArray::CopyArrayFromClusterToMessage(int totalElements, LV2DArrayHandle array, const std::shared_ptr<LVMessageValue>& dataFieldValue)
        {
            auto arrayData = (*array)->bytes<double>();
            auto dataValue = std::static_pointer_cast<LVRepeatedMessageValue<double>>(dataFieldValue);
            dataValue->_value.Reserve(totalElements);
            auto dest = dataValue->_value.AddNAlreadyReserved(totalElements);
            memcpy(dest, arrayData, totalElements * sizeof(double));
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        const std::string& String2DArray::GetMessageName()
        {
            static const std::string messageName = "ni_protobuf_types_String2DArray";
            return messageName;
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        const std::string& String2DArray::GetMessageUrl()
        {
            static const std::string messageUrl = "ni.protobuf.types.String2DArray";
            return messageUrl;
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        void String2DArray::CopyArrayFromMessageToCluster(int rows, int columns, const std::shared_ptr<const LVMessageValue>& dataFieldValue, int8_t* start)
        {
            auto elementCount = rows * columns;
            NumericArrayResize(GetTypeCodeForSize(sizeof(char*)), 2, start, elementCount);
            auto array = *(LV2DArrayHandle*)start;
            (*array)->dimensionSizes[0] = rows;
            (*array)->dimensionSizes[1] = columns;

            // Protect against a malformed message where the amount of data sent doesn't match the dimension sizes.
            // LV will automatically pad/handle writing less data than was allocated to the array so we just need
            // to make sure we don't write more data than was allocated to the array.
            auto dataValue = std::static_pointer_cast<const LVRepeatedMessageValue<std::string>>(dataFieldValue);
            auto stringElements = dataValue->_value.size();
            auto stringsToCopy = std::min(elementCount, stringElements);
            auto lvString = (*array)->bytes<LStrHandle>();
            for (int i = 0; i < stringsToCopy; i++)
            {
                *lvString = nullptr;
                SetLVString(lvString, dataValue->_value[i]);
                lvString += 1;
            }
        }

        void String2DArray::CopyArrayFromClusterToMessage(int totalElements, LV2DArrayHandle array, const std::shared_ptr<LVMessageValue>& dataFieldValue)
        {
            auto lvStr = (*array)->bytes<LStrHandle>();
            auto dataValue = std::static_pointer_cast<LVRepeatedMessageValue<std::string>>(dataFieldValue);
            for (int i = 0; i < totalElements; i++)
            {
                auto str = GetLVString(*lvStr);
                dataValue->_value.Add(str);
                lvStr += 1;
            }
        }
}
}