//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class IMessageElementMetadataOwner;

//---------------------------------------------------------------------
//---------------------------------------------------------------------
enum class LVMessageMetadataType
{
    Int32Value,
    FloatValue,
    DoubleValue,
    BoolValue,
    StringValue,
    MessageValue,
    Int64Value,
    UInt32Value,
    UInt64Value,
    EnumValue
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
    std::string embeddedMessageName;
    int protobufIndex;
    int clusterOffset;
    LVMessageMetadataType type;    
    bool isRepeated;    
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#ifdef _PS_4
#pragma pack (push, 1)
#endif
struct LVMesageElementMetadata
{
    LStrHandle embeddedMessageName;
    int protobufIndex;
    int valueType;
    bool isRepeated;
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
    int clusterSize;
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
struct LVMessageMetadata2
{
    int version;
    LStrHandle messageName;
    LStrHandle typeUrl;
    LV1DArrayHandle elements;
};
