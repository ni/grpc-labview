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

        MessageElementMetadata() {}

    public:
        IMessageElementMetadataOwner* _owner;
        std::string embeddedMessageName;
        int protobufIndex;
        int clusterOffset;
        LVMessageMetadataType type;    
        bool isRepeated;    
    };

    class MessageElementEnumMetadata : public MessageElementMetadata
    {
    public:
        MessageElementEnumMetadata(MessageElementMetadata messageMetadata) :
            MessageElementMetadata(messageMetadata._owner)
        {

        }

        int IsValid(int value)
        {
            std::vector<std::string> enumValues = split(embeddedMessageName, ";");
            std::map<int, std::string> enumKeyValues = CreateEnumFromMetadata(enumValues);

            return !(enumKeyValues.find(value) == enumKeyValues.end());
        }

    private:
        std::vector<std::string> split(std::string s, std::string delimiter)
        {
            size_t pos_start = 0, pos_end, delim_len = delimiter.length();
            std::string token;
            std::vector<std::string> res;

            while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos)
            {
                token = s.substr(pos_start, pos_end - pos_start);
                pos_start = pos_end + delim_len;
                res.push_back(token);
        }

            res.push_back(s.substr(pos_start));
            return res;
    }

        std::map<int, std::string> CreateEnumFromMetadata(std::vector<std::string> enumValues)
        {
            std::map<int, std::string> keyValuePairs;
            for (std::string enumValue : enumValues)
            {
                std::vector<std::string> keyValue = split(enumValue, "=");
                int key = std::stoi(keyValue[1]);
                keyValuePairs.insert(std::pair<int, std::string>(key, keyValue[0]));
            }
            return keyValuePairs;
        }
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
    struct LVMessageMetadata2
    {
        int version;
        LStrHandle messageName;
        LStrHandle typeUrl;
        LV1DArrayHandle elements;
    };
}
