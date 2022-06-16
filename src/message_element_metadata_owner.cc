//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_server.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <thread>
#include <sstream>
#include <fstream>
#include <iostream>
#include <future>

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
    #ifndef _PS_4
        if (repeated)
        {
            return 8;
        }
        switch (type)
        {
        case LVMessageMetadataType::BoolValue:
            return 1;
        case LVMessageMetadataType::EnumValue:
        case LVMessageMetadataType::Int32Value:
        case LVMessageMetadataType::UInt32Value:
        case LVMessageMetadataType::FloatValue:
            return 4;
        case LVMessageMetadataType::Int64Value:
        case LVMessageMetadataType::UInt64Value:
        case LVMessageMetadataType::DoubleValue:
        case LVMessageMetadataType::StringValue:
        case LVMessageMetadataType::BytesValue:
        case LVMessageMetadataType::MessageValue:
            return 8;
        }
    #else
        if (repeated)
        {
            return 4;
        }
        switch (type)
        {
        case LVMessageMetadataType::BoolValue:
            return 1;
        case LVMessageMetadataType::Int32Value:
        case LVMessageMetadataType::UInt32Value:
        case LVMessageMetadataType::EnumValue:
        case LVMessageMetadataType::FloatValue:
            return 4;
        case LVMessageMetadataType::StringValue:
        case LVMessageMetadataType::BytesValue:
        case LVMessageMetadataType::MessageValue:
            return 4;
        case LVMessageMetadataType::Int64Value:
        case LVMessageMetadataType::UInt64Value:
        case LVMessageMetadataType::DoubleValue:
            return 8;
        }
    #endif
        return 0;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    int AlignClusterOffset(int clusterOffset, LVMessageMetadataType type, bool repeated)
    {
    #ifndef _PS_4
        if (clusterOffset == 0)
        {
            return 0;
        }
        auto multiple = ClusterElementSize(type, repeated);
        int remainder = abs(clusterOffset) % multiple;
        if (remainder == 0)
        {
            return clusterOffset;
        }
        return clusterOffset + multiple - remainder;
    #else
        return clusterOffset;
    #endif
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
    std::shared_ptr<MessageMetadata> MessageElementMetadataOwner::FindMetadata(const std::string& name)
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
    void MessageElementMetadataOwner::UpdateMetadataClusterLayout(std::shared_ptr<MessageMetadata>& metadata)
    {
        if (metadata->clusterSize != 0)
        {
            return;
        }
        int clusterOffset = 0;
        for (auto element: metadata->_elements)
        {
            auto elementType = element->type;
            auto nestedElement = element;
            clusterOffset = AlignClusterOffset(clusterOffset, element->type, element->isRepeated);
            element->clusterOffset = clusterOffset;
            if (element->type == LVMessageMetadataType::MessageValue)
            {
                auto nestedMetadata = FindMetadata(element->embeddedMessageName);
                UpdateMetadataClusterLayout(nestedMetadata);
                if (element->isRepeated)
                {
                    clusterOffset += ClusterElementSize(element->type, element->isRepeated);
                }
                else
                {
                    clusterOffset += nestedMetadata->clusterSize;
                }
            }
            else
            {
                clusterOffset += ClusterElementSize(element->type, element->isRepeated);
            }
        }
        metadata->clusterSize = AlignClusterOffset(clusterOffset, LVMessageMetadataType::StringValue, true);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void MessageElementMetadataOwner::FinalizeMetadata()
    {
        for (auto metadata: _registeredMessageMetadata)
        {
            UpdateMetadataClusterLayout(metadata.second);
        }
    }
}
