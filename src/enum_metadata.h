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
        //LVMessageMetadataMap _mappedElements;

        // Create the map between LV enum and proto enum values. Use that map in CopyFromCluster and CopyToCluster. Map should be created when RegisterMetadata is called.
        std::map<int, int32_t> LVEnumToProtoEnum; // Should the map contain enum string name as well?
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