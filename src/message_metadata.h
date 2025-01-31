//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <string>
#include <lv_interop.h>
#include <vector>
#include <map>
#include <unordered_map>
#include <grpcpp/impl/codegen/proto_utils.h>
#include <well_known_types.h>

namespace grpc_labview
{
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class IMessageElementMetadataOwner;

    //---------------------------------------------------------------------
    // Enum equivalent to this on the LabVIEW side: Message Element Type.ctl
    //---------------------------------------------------------------------
    enum class LVMessageMetadataType
    {
        Int32Value = 0,
        FloatValue = 1,
        DoubleValue = 2,
        BoolValue = 3,
        StringValue = 4,
        MessageValue = 5,
        Int64Value = 6,
        UInt32Value = 7,
        UInt64Value = 8,
        EnumValue = 9,
        BytesValue = 10,
        Fixed64Value = 11,
        Fixed32Value = 12,
        SFixed64Value = 13,
        SFixed32Value = 14,
        SInt64Value = 15,
        SInt32Value = 16
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    // The struct below is the equivalent to grpc-lvsupport-release.lvlib:Message Element Metadata.ctl.
    // If there is any change to this cluster, make sure the corresponding .ctl is updated as well,
    // or be prepared to debug any crashes that may occur.
    //
    // The struct gets used while creating message metadata. The .ctl constants array (Register Message
    //  Metadata.vi) from the LabVIEW side is parsed and those values are written to the instances of this
    //  struct. This is for parsing fields inside a message.
    // The C++ layer writes the data into another struct very similar to this one: MessageElementMetadata
    #ifdef _PS_4
    #pragma pack (push, 1)
    #endif
    struct LVMessageElementMetadata
    {
        LStrHandle fieldName;
        LStrHandle embeddedMessageName;
        int protobufIndex;
        int valueType;
        bool isRepeated;
        bool isInOneof;
        LStrHandle oneofContainerName;
    };
    #ifdef _PS_4
    #pragma pack (pop)
    #endif

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class MessageElementMetadata
    {
    public:
        MessageElementMetadata(LVMessageMetadataType valueType, bool isRepeated, int protobufIndex);
        MessageElementMetadata(IMessageElementMetadataOwner* owner, LVMessageElementMetadata* elementMetadata, int metadataVersion = 2);

    public:
        IMessageElementMetadataOwner* _owner;
        std::string fieldName;
        std::string embeddedMessageName;
        int protobufIndex;
        int clusterOffset;
        LVMessageMetadataType type;
        bool isRepeated;
        bool isInOneof;
        std::string oneofContainerName;
        wellknown::Types wellKnownType;

    private:
        static std::map<const std::string, wellknown::Types(*)(const MessageElementMetadata&)> _wellKnownTypeFunctionMap;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    using LVMessageMetadataMap = std::unordered_map<google::protobuf::uint32, std::shared_ptr<MessageElementMetadata>>;
    using LVMessageMetadataList = std::vector<std::shared_ptr<MessageElementMetadata>>;

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    struct LVMessageMetadata
    {
        LStrHandle messageName;
        LV1DArrayHandle elements;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    // The struct below is the equivalent to grpc-lvsupport-release.lvlib:Message Metadata.ctl.
    // If there is any change to this cluster, make sure the corresponding .ctl is updated as well,
    // or be prepared to debug any crashes that may occur.
    //
    // The struct gets used while creating message metadata. The .ctl constants array (Register Message
    //  Metadata.vi) from the LabVIEW side is parsed and those values are written to the instances of this
    //  struct.
    // The C++ layer writes the data into another struct very similar to this one: MessageMetadata
    struct LVMessageMetadata2
    {
        int version;
        LStrHandle messageName;
        LStrHandle typeUrl;
        LV1DArrayHandle elements;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    struct MessageMetadata
    {
    public:
        MessageMetadata() : clusterSize(0) {}
        MessageMetadata(IMessageElementMetadataOwner* metadataOwner, LVMessageMetadata* lvMetadata);
        MessageMetadata(IMessageElementMetadataOwner* metadataOwner, LVMessageMetadata2* lvMetadata);

    private:
        void InitializeElements(IMessageElementMetadataOwner* metadataOwner, LVMessageElementMetadata* lvElement, int elementCount, int metadataVersion);

    public:
        std::string messageName;
        std::string typeUrl;
        int clusterSize;
        int alignmentRequirement;
        LVMessageMetadataList _elements;
        LVMessageMetadataMap _mappedElements;
    };
}
