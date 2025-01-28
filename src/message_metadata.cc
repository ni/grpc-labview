//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include "message_metadata.h"
#include "lv_interop.h"
#include "well_known_types.h"

namespace grpc_labview
{
    //---------------------------------------------------------------------
    // Functions for calculating whether an element is considered a well known type.
    //---------------------------------------------------------------------
    static wellknown::Types IsWellKnownDouble2DArray(const MessageElementMetadata& metadata)
    {
        if (!metadata.isRepeated)
        {
            return wellknown::Types::Double2DArray;
        }
        return wellknown::Types::None;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    std::map<const std::string, wellknown::Types(*)(const MessageElementMetadata&)> MessageElementMetadata::_wellKnownTypeFunctionMap =
    {
        { wellknown::Double2DArray::GetMessageName(), IsWellKnownDouble2DArray}
    };

    //---------------------------------------------------------------------
	//---------------------------------------------------------------------
    MessageElementMetadata::MessageElementMetadata(LVMessageMetadataType valueType, bool isRepeated, int protobufIndex)
    {
        _owner = nullptr;
        clusterOffset = 0;
        this->isRepeated = isRepeated;
        this->protobufIndex = protobufIndex;
        type = valueType;
        isInOneof = false;
        wellKnownType = wellknown::Types::None;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
	MessageElementMetadata::MessageElementMetadata(IMessageElementMetadataOwner* owner, LVMesageElementMetadata* lvElement, int metadataVersion)
    {
        _owner = owner;
        clusterOffset = 0;
        embeddedMessageName = GetLVString(lvElement->embeddedMessageName);
        isRepeated = lvElement->isRepeated;
        protobufIndex = lvElement->protobufIndex;
        type = (LVMessageMetadataType)lvElement->valueType;

        if (metadataVersion >= 2)
        {
            fieldName = GetLVString(lvElement->fieldName);
            isInOneof = lvElement->isInOneof;
            oneofContainerName = GetLVString(lvElement->oneofContainerName);
        }
        else
        {
            isInOneof = false;
        }

        // Calculate well known type last after initializing all other fields.
        auto it = _wellKnownTypeFunctionMap.find(embeddedMessageName);
        if (it != _wellKnownTypeFunctionMap.end())
        {
            wellKnownType = it->second(*this);
        }
        else
        {
            wellKnownType = wellknown::Types::None;
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    MessageMetadata::MessageMetadata(IMessageElementMetadataOwner* metadataOwner, LVMessageMetadata* lvMetadata)
    {
        auto name = GetLVString(lvMetadata->messageName);
        messageName = name;
        typeUrl = name;
        clusterSize = 0;
        alignmentRequirement = 0;

        if (lvMetadata->elements != nullptr)
        {
            // byteAlignment for LVMesageElementMetadata would be the size of its largest element which is a LStrHandle
            auto lvElement = (LVMesageElementMetadata*)(*lvMetadata->elements)->bytes(0, sizeof(LStrHandle));
            auto metadataVersion = 1;
            InitializeElements(metadataOwner, lvElement, (*lvMetadata->elements)->cnt, metadataVersion);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    MessageMetadata::MessageMetadata(IMessageElementMetadataOwner* metadataOwner, LVMessageMetadata2* lvMetadata)
    {
        auto name = GetLVString(lvMetadata->messageName);
        messageName = name;
        typeUrl = GetLVString(lvMetadata->typeUrl);
        clusterSize = 0;
        alignmentRequirement = 0;

        if (lvMetadata->elements != nullptr)
        {
            // byteAlignment for LVMesageElementMetadata would be the size of its largest element which is a LStrHandle
            auto lvElement = (LVMesageElementMetadata*)(*lvMetadata->elements)->bytes(0, sizeof(LStrHandle));
            auto metadataVersion = 2;
            InitializeElements(metadataOwner, lvElement, (*lvMetadata->elements)->cnt, metadataVersion);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void MessageMetadata::InitializeElements(IMessageElementMetadataOwner* metadataOwner, LVMesageElementMetadata* lvElement, int elementCount, int metadataVersion)
    {
        for (int i = 0; i < elementCount; i++, lvElement++)
        {
            auto element = std::make_shared<MessageElementMetadata>(metadataOwner, lvElement, metadataVersion);
            _elements.push_back(element);
            _mappedElements.emplace(element->protobufIndex, element);
        }
    }
}