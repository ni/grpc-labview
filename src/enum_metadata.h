//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

#include <list>
#include <exceptions.h>

namespace grpc_labview
{
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    struct EnumMetadata
    {
    public:
        EnumMetadata() :
            clusterSize(0)
        {
        }

        int GetLVEnumValueFromProtoValue(int protoValue)
        {
            int value = 0;
            auto lvValue = ProtoEnumToLVEnum.find(protoValue);
            if (lvValue != ProtoEnumToLVEnum.end())
                value = (lvValue->second).front(); // Since one proto value can be mapped to multiple LV enum values, always return the first element.
            else
            {
                throw InvalidEnumValueException("Invalid enum value!");
            }
            return value;
        }

        int GetProtoValueFromLVEnumValue(int enumValueFromLV)
        {
            int value = 0;
            // Find the equivalent proto value for enumValueFromLV
            auto protoValue = LVEnumToProtoEnum.find(enumValueFromLV);
            if (protoValue != LVEnumToProtoEnum.end())
                value = protoValue->second;
            else
            {
                throw InvalidEnumValueException("Invalid enum value!");
            }
            return value;
        }

    public:
        std::string messageName;
        std::string typeUrl;
        std::string elements;
        bool allowAlias;
        int clusterSize;
        int alignmentRequirement;
        std::map<int, int32_t> LVEnumToProtoEnum;
        std::map<int32_t, std::list<int>> ProtoEnumToLVEnum;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    struct LVEnumMetadata2
    {
        int version;
        LStrHandle messageName;
        LStrHandle typeUrl;
        LStrHandle elements;
        bool allowAlias;
    };
}