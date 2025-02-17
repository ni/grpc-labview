//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_server.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <thread>
#include <sstream>
#include <fstream>
#include <iostream>
#include <future>
#include <well_known_messages.h>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

namespace grpc_labview {
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    int ClusterElementSize(LVMessageMetadataType type, bool repeated)
    {
        if (repeated)
        {
            return sizeof(void*);
        }
        switch (type)
        {
        case LVMessageMetadataType::BoolValue:
            return 1;
        case LVMessageMetadataType::Int32Value:
        case LVMessageMetadataType::UInt32Value:
        case LVMessageMetadataType::EnumValue:
        case LVMessageMetadataType::FloatValue:
        case LVMessageMetadataType::SFixed32Value:
        case LVMessageMetadataType::SInt32Value:
        case LVMessageMetadataType::Fixed32Value:
            return 4;
        case LVMessageMetadataType::Int64Value:
        case LVMessageMetadataType::UInt64Value:
        case LVMessageMetadataType::DoubleValue:
        case LVMessageMetadataType::SFixed64Value:
        case LVMessageMetadataType::SInt64Value:
        case LVMessageMetadataType::Fixed64Value:
            return 8;
        case LVMessageMetadataType::StringValue:
        case LVMessageMetadataType::BytesValue:
        case LVMessageMetadataType::MessageValue:
            return sizeof(void*);
        }
        return 0;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    static int WellKnownTypeElementSize(wellknown::Types type, bool repeated)
    {
        if (repeated)
        {
            return sizeof(void*);
        }
        switch (type)
        {
        case wellknown::Types::Double2DArray:
        case wellknown::Types::String2DArray:
            return sizeof(void*);
        }
        return 0;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    static int WellKnownTypeAlignment(wellknown::Types type, bool repeated)
    {
        if (repeated)
        {
            return sizeof(void*);
        }
        switch (type)
        {
        case wellknown::Types::Double2DArray:
        case wellknown::Types::String2DArray:
            return sizeof(void*);
        }
        return 0;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void MessageElementMetadataOwner::RegisterMetadata(std::shared_ptr<MessageMetadata> requestMetadata)
    {
        std::lock_guard<std::mutex> lock(_mutex);

        _registeredMessageMetadata.insert({requestMetadata->messageName, requestMetadata});
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void MessageElementMetadataOwner::RegisterMetadata(std::shared_ptr<EnumMetadata> metadata)
    {
        std::lock_guard<std::mutex> lock(_mutex);

        _registeredEnumMetadata.insert({ metadata->messageName, metadata });
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    std::shared_ptr<MessageMetadata> MessageElementMetadataOwner::FindMetadata(const std::string& name)
    {
        auto metadata = FindLocalMetadata(name);
        if (metadata)
        {
            return metadata;
        }
        return wellknown::MetadataOwner::GetInstance().FindMetadata(name);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    std::shared_ptr<MessageMetadata> MessageElementMetadataOwner::FindLocalMetadata(const std::string& name)
    {
        auto it = _registeredMessageMetadata.find(name);
        if (it != _registeredMessageMetadata.end())
        {
            return (*it).second;
        }
        return nullptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    std::shared_ptr<EnumMetadata> MessageElementMetadataOwner::FindEnumMetadata(const std::string& name)
    {
        auto it = _registeredEnumMetadata.find(name);
        if (it != _registeredEnumMetadata.end())
        {
            return (*it).second;
        }
        return nullptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void MessageElementMetadataOwner::UpdateMetadataClusterLayout(std::shared_ptr<MessageMetadata>& metadata)
    {
        if (metadata->clusterSize != 0 || metadata->_elements.size() == 0)
        {
            return;
        }

        int clusterOffset = 0;
        int maxAlignmentRequirement = 0;
        for (auto& element : metadata->_elements)
        {
            int alignmentRequirement = 0;
            int elementSize = 0;
            if (element->type == LVMessageMetadataType::MessageValue)
            {
                if (element->wellKnownType != wellknown::Types::None)
                {
                    alignmentRequirement = WellKnownTypeAlignment(element->wellKnownType, element->isRepeated);
                    elementSize = WellKnownTypeElementSize(element->wellKnownType, element->isRepeated);
                }
                else
                {
                    auto nestedMetadata = FindMetadata(element->embeddedMessageName);
                    UpdateMetadataClusterLayout(nestedMetadata);
                    if (element->isRepeated)
                    {
                        alignmentRequirement = elementSize = ClusterElementSize(element->type, element->isRepeated);
                    }
                    else
                    {
                        alignmentRequirement = nestedMetadata->alignmentRequirement;
                        elementSize = nestedMetadata->clusterSize;
                    }
                }
            }
            else
            {
                alignmentRequirement = elementSize = ClusterElementSize(element->type, element->isRepeated);
            }

            clusterOffset = AlignClusterOffset(clusterOffset, alignmentRequirement);
            element->clusterOffset = clusterOffset;
            clusterOffset += elementSize;
            if (maxAlignmentRequirement < alignmentRequirement)
            {
                maxAlignmentRequirement = alignmentRequirement;
            }
        }
        metadata->alignmentRequirement = maxAlignmentRequirement;
        metadata->clusterSize = AlignClusterOffset(clusterOffset, maxAlignmentRequirement);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void MessageElementMetadataOwner::FinalizeMetadata()
    {
        for (auto& metadata : _registeredMessageMetadata)
        {
            UpdateMetadataClusterLayout(metadata.second);
        }
    }
}
