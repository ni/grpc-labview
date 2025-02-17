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
            NumericArrayResize(GetTypeCodeForSize(sizeof(LStrHandle)), 2, start, elementCount);
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
            dataValue->_value.Reserve(totalElements);
            for (int i = 0; i < totalElements; i++)
            {
                auto str = GetLVString(*lvStr);
                dataValue->_value.AddAlreadyReserved(std::move(str));
                lvStr += 1;
            }
        }
}
}