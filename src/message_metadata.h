//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <string>
#include <lv_interop.h>
#include <vector>
#include <map>
#include <grpcpp/impl/codegen/proto_utils.h>

namespace grpc_labview 
{
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class IMessageElementMetadataOwner;

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    // There's an enum equivalent to this on the LabVIEW side: Message Element Type.ctl
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
    class MessageElementMetadata
    {
    public:
        MessageElementMetadata(IMessageElementMetadataOwner* owner) :
            _owner(owner)
        {            
        }

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
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    // The struct below is the equivalanet to grpc-lvsupport-release.lvlib:Message Element Metadata.ctl.
    // If there is any change to this cluster, make sure the corresponding .ctl is updated as well,
    // or be prepared to debug any crashes that may occur.
    //
    // The struct gets used while creating message metadata. The .ctl constants array (Register Message
    //  Metadata.vi) from the LabVIEW side is parsed and those values are written to the instances of this
    //  struct. This is for parsing fields inside a message.
    // What does the C++ layer do with this information now?
    // It writes the data into another struct very similar to this one: MessageElementMetadata
    #ifdef _PS_4
    #pragma pack (push, 1)
    #endif
    struct LVMesageElementMetadata
    {
        LStrHandle fieldName; // Change order in typedef?
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
    using LVMessageMetadataMap = std::map<google::protobuf::uint32, std::shared_ptr<MessageElementMetadata>>;
    using LVMessageMetadataList = std::vector<std::shared_ptr<MessageElementMetadata>>;

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    struct MessageMetadata
    {
    public:
        MessageMetadata() :
            clusterSize(0)
        {            
        }

    public:
        std::string messageName;
        std::string typeUrl;
        int clusterSize;
        int alignmentRequirement;
        LVMessageMetadataList _elements;
        LVMessageMetadataMap _mappedElements;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    struct LVMessageMetadata
    {
        LStrHandle messageName;
        LV1DArrayHandle elements;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    // The struct below is the equivalanet to grpc-lvsupport-release.lvlib:Message Metadata.ctl.
    // If there is any change to this cluster, make sure the corresponding .ctl is updated as well,
    // or be prepared to debug any crashes that may occur.
    //
    // The struct gets used while creating message metadata. The .ctl constants array (Register Message
    //  Metadata.vi) from the LabVIEW side is parsed and those values are written to the instances of this
    //  struct.
    // What does the C++ layer do with this information now?
    // It writes the data into another struct very similar to this one: MessageMetadata
    struct LVMessageMetadata2
    {
        int version;
        LStrHandle messageName;
        LStrHandle typeUrl;
        LV1DArrayHandle elements;
    };
}
