//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <cluster_copier.h>
#include <lv_message.h>

namespace grpc_labview {

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    // Reads a message from any client and writes that message data to a cluster on the LabVIEW side.
    // This method takes the address of the cluster as a parameter and writes to it using the offsets present
    // in the metadata. Each field of the cluster gets written that way, by fetching the offset from the metadata.
    // [Inputs]
    // LVMessage: Rrepresentation of the proto 'message' in LabVIEW
    //  ->  message._values: contains the deserialized values for this message
    // cluster: Pointer to the cluster created by LabVIEW
    void ClusterDataCopier::CopyToCluster(const LVMessage& message, int8_t* cluster)
    {
        std::map<std::string, int> oneof_containerToSelectedIndexMap;
        for (auto val : message._metadata->_mappedElements)
        {
            auto fieldMetadata = val.second;
            auto start = cluster + fieldMetadata->clusterOffset;
            std::shared_ptr<LVMessageValue> value;
            for (auto v : message._values)
            {
                if (v.second->_protobufId == fieldMetadata->protobufIndex)
                {
                    value = v.second;
                    break;
                }
            }
            if (value != nullptr)
            {
                if (fieldMetadata->isInOneof)
                {
                    // set the map of the selected index for the "oneofContainer" to this protobuf Index
                    assert(oneof_containerToSelectedIndexMap.find(fieldMetadata->oneofContainerName) == oneof_containerToSelectedIndexMap.end());
                    oneof_containerToSelectedIndexMap.insert(std::pair<std::string, int>(fieldMetadata->oneofContainerName, fieldMetadata->protobufIndex));
                }
                switch (fieldMetadata->type)
                {
                case LVMessageMetadataType::StringValue:
                    CopyStringToCluster(fieldMetadata, start, value);
                    break;
                case LVMessageMetadataType::BytesValue:
                    CopyBytesToCluster(fieldMetadata, start, value);
                    break;
                case LVMessageMetadataType::BoolValue:
                    CopyBoolToCluster(fieldMetadata, start, value);
                    break;
                case LVMessageMetadataType::DoubleValue:
                    CopyDoubleToCluster(fieldMetadata, start, value);
                    break;
                case LVMessageMetadataType::FloatValue:
                    CopyFloatToCluster(fieldMetadata, start, value);
                    break;
                case LVMessageMetadataType::Int32Value:
                    CopyInt32ToCluster(fieldMetadata, start, value);
                    break;
                case LVMessageMetadataType::MessageValue:
                    CopyMessageToCluster(fieldMetadata, start, value);
                    break;
                case LVMessageMetadataType::Int64Value:
                    CopyInt64ToCluster(fieldMetadata, start, value);
                    break;
                case LVMessageMetadataType::UInt32Value:
                    CopyUInt32ToCluster(fieldMetadata, start, value);
                    break;
                case LVMessageMetadataType::UInt64Value:
                    CopyUInt64ToCluster(fieldMetadata, start, value);
                    break;
                case LVMessageMetadataType::EnumValue:
                    CopyEnumToCluster(fieldMetadata, start, value);
                    break;
                case LVMessageMetadataType::SInt32Value:
                    CopySInt32ToCluster(fieldMetadata, start, value);
                    break;
                case LVMessageMetadataType:: SInt64Value:
                    CopySInt64ToCluster(fieldMetadata, start, value);
                    break;
                case LVMessageMetadataType::Fixed32Value:
                    CopyFixed32ToCluster(fieldMetadata, start, value);
                    break;
                case LVMessageMetadataType::Fixed64Value:
                     CopyFixed64ToCluster(fieldMetadata, start, value);
                     break;
                case LVMessageMetadataType::SFixed32Value:
                    CopySFixed32ToCluster(fieldMetadata, start, value);
                    break;
                case LVMessageMetadataType::SFixed64Value:
                    CopySFixed64ToCluster(fieldMetadata, start, value);
                    break;
                }
            }
        }

        // second pass to fill the oneof selected_index. We can do this in one pass when we push the selected_field to the end of the oneof cluster!        
        // TODO: Skip the entire loop if the message has no oneof. It's a bool in the metadata.
        for (auto val : message._metadata->_mappedElements)
        {
            auto fieldMetadata = val.second;            
            if (fieldMetadata->isInOneof&& fieldMetadata->protobufIndex < 0)
            {   
                // This field is the selected_index field of a oneof
                if (oneof_containerToSelectedIndexMap.find(fieldMetadata->oneofContainerName) != oneof_containerToSelectedIndexMap.end())
                {
                    auto start = cluster + fieldMetadata->clusterOffset;
                    *(int*)start = oneof_containerToSelectedIndexMap[fieldMetadata->oneofContainerName];
                }
            }
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ClusterDataCopier::CopyFromCluster(LVMessage& message, int8_t* cluster)
    {
        message._values.clear();
        std::map<std::string, int> oneof_containerToSelectedIndexMap; // Needed to serialize only the field related to the selected_index
        for (auto val : message._metadata->_mappedElements)
        {            
            auto fieldMetadata = val.second;
            if (fieldMetadata->isInOneof)
            {
                if (fieldMetadata->protobufIndex < 0)
                {
                    // set the map of the selected index for the "oneofContainer" to this protobuf Index
                    assert(oneof_containerToSelectedIndexMap.find(fieldMetadata->oneofContainerName) == oneof_containerToSelectedIndexMap.end());
                    auto selected_index = *(int*)(cluster + fieldMetadata->clusterOffset);
                    oneof_containerToSelectedIndexMap.insert(std::pair<std::string, int>(fieldMetadata->oneofContainerName, selected_index));
                }
            }
        }
        for (auto val : message._metadata->_mappedElements)
        {
            auto fieldMetadata = val.second;
            if (fieldMetadata->isInOneof)
            {
                if (fieldMetadata->protobufIndex >= 0)
                {
                    auto it = oneof_containerToSelectedIndexMap.find(fieldMetadata->oneofContainerName);
                    assert (it != oneof_containerToSelectedIndexMap.end());
                    auto selected_index = it->second;
                    if (selected_index != fieldMetadata->protobufIndex)
                    {
                        // This field is not the selected_index field of a oneof. Do not serialize it.
                        continue;
                    }
                }
            }
            auto start = cluster + fieldMetadata->clusterOffset;
            switch (fieldMetadata->type)
            {
            case LVMessageMetadataType::StringValue:
                CopyStringFromCluster(fieldMetadata, start, message);
                break;
            case LVMessageMetadataType::BytesValue:
                CopyBytesFromCluster(fieldMetadata, start, message);
                break;
            case LVMessageMetadataType::BoolValue:
                CopyBoolFromCluster(fieldMetadata, start, message);
                break;
            case LVMessageMetadataType::DoubleValue:
                CopyDoubleFromCluster(fieldMetadata, start, message);
                break;
            case LVMessageMetadataType::FloatValue:
                CopyFloatFromCluster(fieldMetadata, start, message);
                break;
            case LVMessageMetadataType::Int32Value:
                CopyInt32FromCluster(fieldMetadata, start, message);
                break;
            case LVMessageMetadataType::MessageValue:
                CopyMessageFromCluster(fieldMetadata, start, message);
                break;
            case LVMessageMetadataType::Int64Value:
                CopyInt64FromCluster(fieldMetadata, start, message);
                break;
            case LVMessageMetadataType::UInt32Value:
                CopyUInt32FromCluster(fieldMetadata, start, message);
                break;
            case LVMessageMetadataType::UInt64Value:
                CopyUInt64FromCluster(fieldMetadata, start, message);
                break;
            case LVMessageMetadataType::EnumValue:
                CopyEnumFromCluster(fieldMetadata, start, message);
                break;
            case LVMessageMetadataType::SInt32Value:
                CopySInt32FromCluster(fieldMetadata, start, message);
                break;
            case LVMessageMetadataType::SInt64Value:
                CopySInt64FromCluster(fieldMetadata, start, message);
                break;
            case LVMessageMetadataType::Fixed32Value:
                CopyFixed32FromCluster(fieldMetadata, start, message);
            case LVMessageMetadataType::Fixed64Value:
                CopyFixed64FromCluster(fieldMetadata, start, message);
                break;
            case LVMessageMetadataType::SFixed32Value:
                CopySFixed32FromCluster(fieldMetadata, start, message);
                break;
            case LVMessageMetadataType::SFixed64Value:
                CopySFixed64FromCluster(fieldMetadata, start, message);
                break;
            }
        }
    }


    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool ClusterDataCopier::AnyBuilderAddValue(LVMessage& message, LVMessageMetadataType valueType, bool isRepeated, int protobufIndex, int8_t* value)
    {
        auto metadata = std::make_shared<MessageElementMetadata>(nullptr);
        metadata->clusterOffset = 0;
        metadata->embeddedMessageName = std::string();
        metadata->isRepeated = isRepeated;
        metadata->protobufIndex = protobufIndex;
        metadata->type = valueType;

        switch (valueType)
        {
        case LVMessageMetadataType::StringValue:
            CopyStringFromCluster(metadata, value, message);
            break;
        case LVMessageMetadataType::BytesValue:
            CopyBytesFromCluster(metadata, value, message);
            break;
        case LVMessageMetadataType::BoolValue:
            CopyBoolFromCluster(metadata, value, message);
            break;
        case LVMessageMetadataType::DoubleValue:
            CopyDoubleFromCluster(metadata, value, message);
            break;
        case LVMessageMetadataType::FloatValue:
            CopyFloatFromCluster(metadata, value, message);
            break;
        case LVMessageMetadataType::Int32Value:
            CopyInt32FromCluster(metadata, value, message);
            break;
        case LVMessageMetadataType::MessageValue:
            return false;
            break;
        case LVMessageMetadataType::Int64Value:
            CopyInt64FromCluster(metadata, value, message);
            break;
        case LVMessageMetadataType::UInt32Value:
            CopyUInt32FromCluster(metadata, value, message);
            break;
        case LVMessageMetadataType::UInt64Value:
            CopyUInt64FromCluster(metadata, value, message);
            break;
        case LVMessageMetadataType::EnumValue:
            CopyEnumFromCluster(metadata, value, message);
            break;
        case LVMessageMetadataType::SInt32Value:
            CopySInt32FromCluster(metadata, value, message);
            break;
        case LVMessageMetadataType::SInt64Value:
            CopySInt64FromCluster(metadata, value, message);
            break;
        case LVMessageMetadataType::Fixed32Value:
            CopyFixed32FromCluster(metadata, value, message);
            break;
        case LVMessageMetadataType::Fixed64Value:
            CopyFixed64FromCluster(metadata, value, message);
            break;
        case LVMessageMetadataType::SFixed32Value:
            CopySFixed32FromCluster(metadata, value, message);
            break;
        case LVMessageMetadataType::SFixed64Value:
            CopySFixed64FromCluster(metadata, value, message);
            break;
        default:
            return false;
            break;
        }
        return true;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ClusterDataCopier::CopyStringToCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<LVMessageValue>& value)
    {
        if (metadata->isRepeated)
        {
            auto repeatedString = static_cast<const LVRepeatedStringMessageValue&>(*value);
            if (repeatedString._value.size() != 0)
            {
                NumericArrayResize(0x08, 1, start, repeatedString._value.size());
                auto array = *(LV1DArrayHandle*)start;
                (*array)->cnt = repeatedString._value.size();
                int x = 0;
                auto lvString = (*array)->bytes<LStrHandle>();
                for (auto str : repeatedString._value)
                {
                    *lvString = nullptr;
                    SetLVString(lvString, str);
                    lvString += 1;
                }
            }
        }
        else
        {
            SetLVString((LStrHandle*)start, ((LVStringMessageValue*)value.get())->_value);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ClusterDataCopier::CopyBytesToCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<LVMessageValue>& value)
    {
        CopyStringToCluster(metadata, start, value);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    struct LVCluster
    {
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ClusterDataCopier::CopyMessageToCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<LVMessageValue>& value)
    {
        if (metadata->isRepeated)
        {
            auto repeatedNested = std::static_pointer_cast<LVRepeatedNestedMessageMessageValue>(value);
            if (repeatedNested->_value.size() != 0)
            {
                auto nestedMetadata = repeatedNested->_value.front()->_metadata;
                auto clusterSize = nestedMetadata->clusterSize;

                NumericArrayResize(0x08, 1, start, repeatedNested->_value.size() * clusterSize);
                auto array = *(LV1DArrayHandle*)start;
                (*array)->cnt = repeatedNested->_value.size();
                int x = 0;
                for (auto str : repeatedNested->_value)
                {
                    auto lvCluster = (LVCluster**)(*array)->bytes(x * clusterSize, nestedMetadata->alignmentRequirement);
                    *lvCluster = nullptr;
                    CopyToCluster(*str, (int8_t*)lvCluster);
                    x += 1;
                }
            }
        }
        else
        {
            CopyToCluster(*((LVNestedMessageMessageValue*)value.get())->_value, start);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ClusterDataCopier::CopyInt32ToCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<LVMessageValue>& value)
    {
        if (metadata->isRepeated)
        {
            auto repeatedInt32 = std::static_pointer_cast<LVRepeatedMessageValue<int>>(value);
            if (repeatedInt32->_value.size() != 0)
            {
                NumericArrayResize(0x03, 1, start, repeatedInt32->_value.size());
                auto array = *(LV1DArrayHandle*)start;
                (*array)->cnt = repeatedInt32->_value.size();
                auto byteCount = repeatedInt32->_value.size() * sizeof(int32_t);
                memcpy((*array)->bytes<int32_t>(), repeatedInt32->_value.data(), byteCount);
            }
        }
        else
        {
            *(int*)start = ((LVVariableMessageValue<int>*)value.get())->_value;
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ClusterDataCopier::CopyUInt32ToCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<LVMessageValue>& value)
    {
        if (metadata->isRepeated)
        {
            auto repeatedUInt32 = std::static_pointer_cast<LVRepeatedMessageValue<uint32_t>>(value);
            if (repeatedUInt32->_value.size() != 0)
            {
                NumericArrayResize(0x03, 1, start, repeatedUInt32->_value.size());
                auto array = *(LV1DArrayHandle*)start;
                (*array)->cnt = repeatedUInt32->_value.size();
                auto byteCount = repeatedUInt32->_value.size() * sizeof(uint32_t);
                memcpy((*array)->bytes<uint32_t>(), repeatedUInt32->_value.data(), byteCount);
            }
        }
        else
        {
            *(int*)start = ((LVVariableMessageValue<uint32_t>*)value.get())->_value;
        }
    }

    void ClusterDataCopier::CopyEnumToCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<LVMessageValue>& value)
    {
        std::shared_ptr<EnumMetadata> enumMetadata = metadata->_owner->FindEnumMetadata(metadata->embeddedMessageName);

        if (metadata->isRepeated)
        {
            auto repeatedEnum = std::static_pointer_cast<LVRepeatedEnumMessageValue>(value);
            int count = repeatedEnum->_value.size();
            // Map the repeatedEnum from protobuf to LV enum values.
            int32_t* mappedArray = (int32_t*)malloc(count * sizeof(int32_t));

            for (size_t i = 0; i < count; i++)
            {
                auto protoValue = (repeatedEnum->_value.data())[i];
                mappedArray[i] = enumMetadata->GetLVEnumValueFromProtoValue(protoValue);
            }

            if (count != 0)
            {
                NumericArrayResize(0x03, 1, start, count);
                auto array = *(LV1DArrayHandle*)start;
                (*array)->cnt = count;
                auto byteCount = count * sizeof(int32_t);
                memcpy((*array)->bytes<int32_t>(), mappedArray, byteCount);
            }
            
            free(mappedArray);
        }
        else
        {
            auto enumValueFromPrtobuf = ((LVEnumMessageValue*)value.get())->_value;
            *(int*)start = enumMetadata->GetLVEnumValueFromProtoValue(enumValueFromPrtobuf);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ClusterDataCopier::CopyInt64ToCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<LVMessageValue>& value)
    {
        if (metadata->isRepeated)
        {
            auto repeatedInt64 = std::static_pointer_cast<LVRepeatedMessageValue<int64_t>>(value);
            if (repeatedInt64->_value.size() != 0)
            {
                NumericArrayResize(0x04, 1, start, repeatedInt64->_value.size());
                auto array = *(LV1DArrayHandle*)start;
                (*array)->cnt = repeatedInt64->_value.size();
                auto byteCount = repeatedInt64->_value.size() * sizeof(int64_t);
                memcpy((*array)->bytes<int64_t>(), repeatedInt64->_value.data(), byteCount);
            }
        }
        else
        {
            *(int64_t*)start = ((LVVariableMessageValue<int64_t>*)value.get())->_value;
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ClusterDataCopier::CopyUInt64ToCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<LVMessageValue>& value)
    {
        if (metadata->isRepeated)
        {
            auto repeatedUInt64 = std::static_pointer_cast<LVRepeatedMessageValue<uint64_t>>(value);
            if (repeatedUInt64->_value.size() != 0)
            {
                NumericArrayResize(0x08, 1, start, repeatedUInt64->_value.size());
                auto array = *(LV1DArrayHandle*)start;
                (*array)->cnt = repeatedUInt64->_value.size();
                auto byteCount = repeatedUInt64->_value.size() * sizeof(uint64_t);
                memcpy((*array)->bytes<uint64_t>(), repeatedUInt64->_value.data(), byteCount);
            }
        }
        else
        {
            *(uint64_t*)start = ((LVVariableMessageValue<uint64_t>*)value.get())->_value;
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ClusterDataCopier::CopyBoolToCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<LVMessageValue>& value)
    {
        if (metadata->isRepeated)
        {
            auto repeatedBoolean = std::static_pointer_cast<LVRepeatedMessageValue<bool>>(value);
            if (repeatedBoolean->_value.size() != 0)
            {
                NumericArrayResize(0x01, 1, start, repeatedBoolean->_value.size());
                auto array = *(LV1DArrayHandle*)start;
                (*array)->cnt = repeatedBoolean->_value.size();
                auto byteCount = repeatedBoolean->_value.size() * sizeof(bool);
                memcpy((*array)->bytes<bool>(), repeatedBoolean->_value.data(), byteCount);
            }
        }
        else
        {
            *(bool*)start = ((LVVariableMessageValue<bool>*)value.get())->_value;
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ClusterDataCopier::CopyDoubleToCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<LVMessageValue>& value)
    {
        if (metadata->isRepeated)
        {
            auto repeatedDouble = std::static_pointer_cast<LVRepeatedMessageValue<double>>(value);
            if (repeatedDouble->_value.size() != 0)
            {
                auto array = *(LV1DArrayHandle*)start;
                NumericArrayResize(0x0A, 1, start, repeatedDouble->_value.size());
                array = *(LV1DArrayHandle*)start;
                (*array)->cnt = repeatedDouble->_value.size();
                auto byteCount = repeatedDouble->_value.size() * sizeof(double);
                memcpy((*array)->bytes<double>(), repeatedDouble->_value.data(), byteCount);
            }
        }
        else
        {
            *(double*)start = ((LVVariableMessageValue<double>*)value.get())->_value;
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ClusterDataCopier::CopyFloatToCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<LVMessageValue>& value)
    {
        if (metadata->isRepeated)
        {
            auto repeatedFloat = std::static_pointer_cast<LVRepeatedMessageValue<float>>(value);
            if (repeatedFloat->_value.size() != 0)
            {
                NumericArrayResize(0x03, 1, start, repeatedFloat->_value.size());
                auto array = *(LV1DArrayHandle*)start;
                (*array)->cnt = repeatedFloat->_value.size();
                auto byteCount = repeatedFloat->_value.size() * sizeof(float);
                memcpy((*array)->bytes<float>(), repeatedFloat->_value.data(), byteCount);
            }
        }
        else
        {
            *(float*)start = ((LVVariableMessageValue<float>*)value.get())->_value;
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ClusterDataCopier::CopySInt32ToCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<LVMessageValue>& value)
    {
        if (metadata->isRepeated)
        {
            auto repeatedSInt32 = std::static_pointer_cast<LVRepeatedSInt32MessageValue>(value);
            if (repeatedSInt32->_value.size() != 0)
            {
                NumericArrayResize(0x03, 1, start, repeatedSInt32->_value.size());
                auto array = *(LV1DArrayHandle*)start;
                (*array)->cnt = repeatedSInt32->_value.size();
                auto byteCount = repeatedSInt32->_value.size() * sizeof(int32_t);
                memcpy((*array)->bytes<int32_t>(), repeatedSInt32->_value.data(), byteCount);
            }
        }
        else
        {
            *(int32_t*)start = ((LVSInt32MessageValue*)value.get())->_value;
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ClusterDataCopier::CopySInt64ToCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<LVMessageValue>& value)
    {
        if (metadata->isRepeated)
        {
            auto repeatedSInt64 = std::static_pointer_cast<LVRepeatedSInt64MessageValue>(value);
            if (repeatedSInt64->_value.size() != 0)
            {
                NumericArrayResize(0x04, 1, start, repeatedSInt64->_value.size());
                auto array = *(LV1DArrayHandle*)start;
                (*array)->cnt = repeatedSInt64->_value.size();
                auto byteCount = repeatedSInt64->_value.size() * sizeof(int64_t);
                memcpy((*array)->bytes<int64_t>(), repeatedSInt64->_value.data(), byteCount);
            }
        }
        else
        {
            *(int64_t*)start = ((LVSInt64MessageValue*)value.get())->_value;
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ClusterDataCopier::CopyFixed32ToCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<LVMessageValue>& value)
    {
        if (metadata->isRepeated)
        {
            auto repeated = std::static_pointer_cast<LVRepeatedFixed32MessageValue>(value);
            if (repeated->_value.size() != 0)
            {
                NumericArrayResize(0x03, 1, start, repeated->_value.size());
                auto array = *(LV1DArrayHandle*)start;
                (*array)->cnt = repeated->_value.size();
                auto byteCount = repeated->_value.size() * sizeof(uint32_t);
                memcpy((*array)->bytes<uint32_t>(), repeated->_value.data(), byteCount);
            }
        }
        else
        {
            *(uint32_t*)start = ((LVFixed32MessageValue*)value.get())->_value;
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ClusterDataCopier::CopySFixed32ToCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<LVMessageValue>& value)
    {
        if (metadata->isRepeated)
        {
            auto repeated = std::static_pointer_cast<LVRepeatedSFixed32MessageValue>(value);
            if (repeated->_value.size() != 0)
            {
                NumericArrayResize(0x03, 1, start, repeated->_value.size());
                auto array = *(LV1DArrayHandle*)start;
                (*array)->cnt = repeated->_value.size();
                auto byteCount = repeated->_value.size() * sizeof(int32_t);
                memcpy((*array)->bytes<int32_t>(), repeated->_value.data(), byteCount);
            }
        }
        else
        {
            *(int32_t*)start = ((LVSFixed32MessageValue*)value.get())->_value;
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ClusterDataCopier::CopyFixed64ToCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<LVMessageValue>& value)
    {
        if (metadata->isRepeated)
        {
            auto repeated = std::static_pointer_cast<LVRepeatedFixed64MessageValue>(value);
            if (repeated->_value.size() != 0)
            {
                NumericArrayResize(0x04, 1, start, repeated->_value.size());
                auto array = *(LV1DArrayHandle*)start;
                (*array)->cnt = repeated->_value.size();
                auto byteCount = repeated->_value.size() * sizeof(uint64_t);
                memcpy((*array)->bytes<uint64_t>(), repeated->_value.data(), byteCount);
            }
        }
        else
        {
            *(uint64_t*)start = ((LVFixed64MessageValue*)value.get())->_value;
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ClusterDataCopier::CopySFixed64ToCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, const std::shared_ptr<LVMessageValue>& value)
    {
        if (metadata->isRepeated)
        {
            auto repeated = std::static_pointer_cast<LVRepeatedSFixed64MessageValue>(value);
            if (repeated->_value.size() != 0)
            {
                NumericArrayResize(0x04, 1, start, repeated->_value.size());
                auto array = *(LV1DArrayHandle*)start;
                (*array)->cnt = repeated->_value.size();
                auto byteCount = repeated->_value.size() * sizeof(int64_t);
                memcpy((*array)->bytes<int64_t>(), repeated->_value.data(), byteCount);
            }
        }
        else
        {
            *(int64_t*)start = ((LVSFixed64MessageValue*)value.get())->_value;
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ClusterDataCopier::CopyStringFromCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message)
    {
        if (metadata->isRepeated)
        {
            auto array = *(LV1DArrayHandle*)start;
            if (array && *array && ((*array)->cnt != 0))
            {
                auto repeatedStringValue = std::make_shared<LVRepeatedStringMessageValue>(metadata->protobufIndex);
                message._values.emplace(metadata->protobufIndex, repeatedStringValue);
                auto lvStr = (*array)->bytes<LStrHandle>();
                for (int x = 0; x < (*array)->cnt; ++x)
                {
                    auto str = GetLVString(*lvStr);
                    // repeatedStringValue->_value.Add(&str);
                    repeatedStringValue->_value.Add()->assign(str);
                    lvStr += 1;
                }
            }
        }
        else
        {
            auto str = GetLVString(*(LStrHandle*)start);
            auto stringValue = std::make_shared<LVStringMessageValue>(metadata->protobufIndex, str);
            message._values.emplace(metadata->protobufIndex, stringValue);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ClusterDataCopier::CopyBytesFromCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message)
    {
        CopyStringFromCluster(metadata, start, message);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ClusterDataCopier::CopyBoolFromCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message)
    {
        if (metadata->isRepeated)
        {
            auto array = *(LV1DArrayHandle*)start;
            if (array && *array && ((*array)->cnt != 0))
            {
                auto count = (*array)->cnt;
                auto repeatedValue = std::make_shared<LVRepeatedMessageValue<bool>>(metadata->protobufIndex);
                message._values.emplace(metadata->protobufIndex, repeatedValue);
                auto data = (*array)->bytes<bool>();
                repeatedValue->_value.Reserve(count);
                auto dest = repeatedValue->_value.AddNAlreadyReserved(count);
                memcpy(dest, data, count * sizeof(bool));
            }
        }
        else
        {
            auto value = std::make_shared<LVVariableMessageValue<bool>>(metadata->protobufIndex, *(bool*)start);
            message._values.emplace(metadata->protobufIndex, value);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ClusterDataCopier::CopyInt32FromCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message)
    {
        if (metadata->isRepeated)
        {
            auto array = *(LV1DArrayHandle*)start;
            if (array && *array && ((*array)->cnt != 0))
            {
                auto count = (*array)->cnt;
                auto repeatedValue = std::make_shared<LVRepeatedMessageValue<int>>(metadata->protobufIndex);
                message._values.emplace(metadata->protobufIndex, repeatedValue);
                auto data = (*array)->bytes<int32_t>();
                repeatedValue->_value.Reserve(count);
                auto dest = repeatedValue->_value.AddNAlreadyReserved(count);
                memcpy(dest, data, count * sizeof(int32_t));
            }
        }
        else
        {
            auto value = std::make_shared<LVVariableMessageValue<int>>(metadata->protobufIndex, *(int*)start);
            message._values.emplace(metadata->protobufIndex, value);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ClusterDataCopier::CopyUInt32FromCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message)
    {
        if (metadata->isRepeated)
        {
            auto array = *(LV1DArrayHandle*)start;
            if (array && *array && ((*array)->cnt != 0))
            {
                auto count = (*array)->cnt;
                auto repeatedValue = std::make_shared<LVRepeatedMessageValue<uint32_t>>(metadata->protobufIndex);
                message._values.emplace(metadata->protobufIndex, repeatedValue);
                auto data = (*array)->bytes<uint32_t>();
                repeatedValue->_value.Reserve(count);
                auto dest = repeatedValue->_value.AddNAlreadyReserved(count);
                memcpy(dest, data, count * sizeof(uint32_t));
            }
        }
        else
        {
            auto value = std::make_shared<LVVariableMessageValue<uint32_t>>(metadata->protobufIndex, *(uint32_t*)start);
            message._values.emplace(metadata->protobufIndex, value);
        }
    }

    void ClusterDataCopier::CopyEnumFromCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message)
    {
        std::shared_ptr<EnumMetadata> enumMetadata = metadata->_owner->FindEnumMetadata(metadata->embeddedMessageName);

        if (metadata->isRepeated)
        {
            auto array = *(LV1DArrayHandle*)start;
            if (array && *array && ((*array)->cnt != 0))
            {
                auto count = (*array)->cnt;
                auto repeatedValue = std::make_shared<LVRepeatedEnumMessageValue>(metadata->protobufIndex);
                message._values.emplace(metadata->protobufIndex, repeatedValue);
                auto data = (*array)->bytes<int32_t>();

                // "data" has the array of enums sent from the LV side. Iterate this array, map each element to the equivalent proto
                // value and copy the new array into the destination.
                int32_t* mappedArray = (int32_t*)malloc(count * sizeof(int32_t));

                for (size_t i = 0; i < count; i++)
                {
                    auto enumValueFromLV = data[i];
                    // Find the equivalent proto value for enumValueFromLV
                    mappedArray[i] = enumMetadata->GetProtoValueFromLVEnumValue(enumValueFromLV);
                }

                repeatedValue->_value.Reserve(count);
                auto dest = repeatedValue->_value.AddNAlreadyReserved(count);
                memcpy(dest, mappedArray, count * sizeof(int32_t));
                
                free(mappedArray);
            }
        }
        else
        {
            auto enumValueFromLV = *(int32_t*)start;
            int protoValue = enumMetadata->GetProtoValueFromLVEnumValue(enumValueFromLV);
            auto value = std::make_shared<LVEnumMessageValue>(metadata->protobufIndex, protoValue);
            message._values.emplace(metadata->protobufIndex, value);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ClusterDataCopier::CopyInt64FromCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message)
    {
        if (metadata->isRepeated)
        {
            auto array = *(LV1DArrayHandle*)start;
            if (array && *array && ((*array)->cnt != 0))
            {
                auto count = (*array)->cnt;
                auto repeatedValue = std::make_shared<LVRepeatedMessageValue<int64_t>>(metadata->protobufIndex);
                message._values.emplace(metadata->protobufIndex, repeatedValue);
                auto data = (*array)->bytes<int64_t>();
                repeatedValue->_value.Reserve(count);
                auto dest = repeatedValue->_value.AddNAlreadyReserved(count);
                memcpy(dest, data, count * sizeof(int64_t));
            }
        }
        else
        {
            auto value = std::make_shared<LVVariableMessageValue<int64_t>>(metadata->protobufIndex, *(int64_t*)start);
            message._values.emplace(metadata->protobufIndex, value);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ClusterDataCopier::CopyUInt64FromCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message)
    {
        if (metadata->isRepeated)
        {
            auto array = *(LV1DArrayHandle*)start;
            if (array && *array && ((*array)->cnt != 0))
            {
                auto count = (*array)->cnt;
                auto repeatedValue = std::make_shared<LVRepeatedMessageValue<uint64_t>>(metadata->protobufIndex);
                message._values.emplace(metadata->protobufIndex, repeatedValue);
                auto data = (*array)->bytes<uint64_t>();
                repeatedValue->_value.Reserve(count);
                auto dest = repeatedValue->_value.AddNAlreadyReserved(count);
                memcpy(dest, data, count * sizeof(uint64_t));
            }
        }
        else
        {
            auto value = std::make_shared<LVVariableMessageValue<uint64_t>>(metadata->protobufIndex, *(uint64_t*)start);
            message._values.emplace(metadata->protobufIndex, value);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ClusterDataCopier::CopyDoubleFromCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message)
    {
        if (metadata->isRepeated)
        {
            auto array = *(LV1DArrayHandle*)start;
            if (array && *array && ((*array)->cnt != 0))
            {
                auto count = (*array)->cnt;
                auto repeatedValue = std::make_shared<LVRepeatedMessageValue<double>>(metadata->protobufIndex);
                message._values.emplace(metadata->protobufIndex, repeatedValue);
                auto data = (*array)->bytes<double>();
                repeatedValue->_value.Reserve(count);
                auto dest = repeatedValue->_value.AddNAlreadyReserved(count);
                memcpy(dest, data, count * sizeof(double));
            }
        }
        else
        {
            auto value = std::make_shared<LVVariableMessageValue<double>>(metadata->protobufIndex, *(double*)start);
            message._values.emplace(metadata->protobufIndex, value);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ClusterDataCopier::CopyFloatFromCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message)
    {
        if (metadata->isRepeated)
        {
            auto array = *(LV1DArrayHandle*)start;
            if (array && *array && ((*array)->cnt != 0))
            {
                auto count = (*array)->cnt;
                auto repeatedValue = std::make_shared<LVRepeatedMessageValue<float>>(metadata->protobufIndex);
                message._values.emplace(metadata->protobufIndex, repeatedValue);
                auto data = (*array)->bytes<float>();
                repeatedValue->_value.Reserve(count);
                auto dest = repeatedValue->_value.AddNAlreadyReserved(count);
                memcpy(dest, data, count * sizeof(float));
            }
        }
        else
        {
            auto value = std::make_shared<LVVariableMessageValue<float>>(metadata->protobufIndex, *(float*)start);
            message._values.emplace(metadata->protobufIndex, value);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ClusterDataCopier::CopyMessageFromCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message)
    {
        auto nestedMetadata = metadata->_owner->FindMetadata(metadata->embeddedMessageName);

        if (metadata->isRepeated)
        {
            auto array = *(LV1DArrayHandle*)start;
            if (array && *array && ((*array)->cnt != 0))
            {
                auto count = (*array)->cnt;
                if (count != 0)
                {
                    auto repeatedValue = std::make_shared<LVRepeatedNestedMessageMessageValue>(metadata->protobufIndex);
                    message._values.emplace(metadata->protobufIndex, repeatedValue);

                    for (int x = 0; x < count; ++x)
                    {
                        auto data = (LVCluster*)(*array)->bytes(nestedMetadata->clusterSize * x, nestedMetadata->alignmentRequirement);
                        auto nested = std::make_shared<LVMessage>(nestedMetadata);
                        repeatedValue->_value.push_back(nested);
                        CopyFromCluster(*nested, (int8_t*)data);
                    }
                }
            }
        }
        else
        {
            auto nested = std::make_shared<LVMessage>(nestedMetadata);
            CopyFromCluster(*nested, start);
            auto value = std::make_shared<LVNestedMessageMessageValue>(metadata->protobufIndex, nested);
            message._values.emplace(metadata->protobufIndex, value);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ClusterDataCopier::CopySInt32FromCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message)
    {
        if (metadata->isRepeated)
        {
            auto array = *(LV1DArrayHandle*)start;
            if (array && *array && ((*array)->cnt != 0))
            {
                auto count = (*array)->cnt;
                auto repeatedValue = std::make_shared<LVRepeatedSInt32MessageValue>(metadata->protobufIndex);
                message._values.emplace(metadata->protobufIndex, repeatedValue);
                auto data = (*array)->bytes<int32_t>();
                repeatedValue->_value.Reserve(count);
                auto dest = repeatedValue->_value.AddNAlreadyReserved(count);
                memcpy(dest, data, count * sizeof(int32_t));
            }
        }
        else
        {
            auto value = std::make_shared<LVSInt32MessageValue>(metadata->protobufIndex, *(int*)start);
            message._values.emplace(metadata->protobufIndex, value);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ClusterDataCopier::CopySInt64FromCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message)
    {
        if (metadata->isRepeated)
        {
            auto array = *(LV1DArrayHandle*)start;
            if (array && *array && ((*array)->cnt != 0))
            {
                auto count = (*array)->cnt;
                auto repeatedValue = std::make_shared<LVRepeatedSInt64MessageValue>(metadata->protobufIndex);
                message._values.emplace(metadata->protobufIndex, repeatedValue);
                auto data = (*array)->bytes<int64_t>();
                repeatedValue->_value.Reserve(count);
                auto dest = repeatedValue->_value.AddNAlreadyReserved(count);
                memcpy(dest, data, count * sizeof(int64_t));
            }
        }
        else
        {
            auto value = std::make_shared<LVSInt64MessageValue>(metadata->protobufIndex, *(int64_t*)start);
            message._values.emplace(metadata->protobufIndex, value);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ClusterDataCopier::CopyFixed32FromCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message)
    {
        if (metadata->isRepeated)
        {
            auto array = *(LV1DArrayHandle*)start;
            if (array && *array && ((*array)->cnt != 0))
            {
                auto count = (*array)->cnt;
                auto repeatedValue = std::make_shared<LVRepeatedFixed32MessageValue>(metadata->protobufIndex);
                message._values.emplace(metadata->protobufIndex, repeatedValue);
                auto data = (*array)->bytes<uint32_t>();
                repeatedValue->_value.Reserve(count);
                auto dest = repeatedValue->_value.AddNAlreadyReserved(count);
                memcpy(dest, data, count * sizeof(uint32_t));
            }
        }
        else
        {
            auto value = std::make_shared<LVFixed32MessageValue>(metadata->protobufIndex, *(uint32_t*)start);
            message._values.emplace(metadata->protobufIndex, value);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ClusterDataCopier::CopyFixed64FromCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message)
    {
        if (metadata->isRepeated)
        {
            auto array = *(LV1DArrayHandle*)start;
            if (array && *array && ((*array)->cnt != 0))
            {
                auto count = (*array)->cnt;
                auto repeatedValue = std::make_shared<LVRepeatedFixed64MessageValue>(metadata->protobufIndex);
                message._values.emplace(metadata->protobufIndex, repeatedValue);
                auto data = (*array)->bytes<uint64_t>();
                repeatedValue->_value.Reserve(count);
                auto dest = repeatedValue->_value.AddNAlreadyReserved(count);
                memcpy(dest, data, count * sizeof(uint64_t));
            }
        }
        else
        {
            auto value = std::make_shared<LVFixed64MessageValue>(metadata->protobufIndex, *(uint64_t*)start);
            message._values.emplace(metadata->protobufIndex, value);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ClusterDataCopier::CopySFixed32FromCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message)
    {
        if (metadata->isRepeated)
        {
            auto array = *(LV1DArrayHandle*)start;
            if (array && *array && ((*array)->cnt != 0))
            {
                auto count = (*array)->cnt;
                auto repeatedValue = std::make_shared<LVRepeatedSFixed32MessageValue>(metadata->protobufIndex);
                message._values.emplace(metadata->protobufIndex, repeatedValue);
                auto data = (*array)->bytes<int32_t>();
                repeatedValue->_value.Reserve(count);
                auto dest = repeatedValue->_value.AddNAlreadyReserved(count);
                memcpy(dest, data, count * sizeof(int32_t));
            }
        }
        else
        {
            auto value = std::make_shared<LVSFixed32MessageValue>(metadata->protobufIndex, *(int32_t*)start);
            message._values.emplace(metadata->protobufIndex, value);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void ClusterDataCopier::CopySFixed64FromCluster(const std::shared_ptr<MessageElementMetadata> metadata, int8_t* start, LVMessage& message)
    {
        if (metadata->isRepeated)
        {
            auto array = *(LV1DArrayHandle*)start;
            if (array && *array && ((*array)->cnt != 0))
            {
                auto count = (*array)->cnt;
                auto repeatedValue = std::make_shared<LVRepeatedSFixed64MessageValue>(metadata->protobufIndex);
                message._values.emplace(metadata->protobufIndex, repeatedValue);
                auto data = (*array)->bytes<int64_t>();
                repeatedValue->_value.Reserve(count);
                auto dest = repeatedValue->_value.AddNAlreadyReserved(count);
                memcpy(dest, data, count * sizeof(int64_t));
            }
        }
        else
        {
            auto value = std::make_shared<LVSFixed64MessageValue>(metadata->protobufIndex, *(int64_t*)start);
            message._values.emplace(metadata->protobufIndex, value);
        }
    }
}