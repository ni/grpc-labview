//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

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

    public:
        std::string messageName;
        std::string typeUrl;
        std::string elements;
        bool allowAlias;
        int clusterSize;
        int alignmentRequirement;
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