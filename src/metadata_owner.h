//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <message_metadata.h>
#include <enum_metadata.h>

namespace grpc_labview
{
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class IMessageElementMetadataOwner
    {
    public:
        virtual std::shared_ptr<MessageMetadata> FindMetadata(const std::string& name) = 0;
        virtual std::shared_ptr<EnumMetadata> FindEnumMetadata(const std::string& name) = 0;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class MessageElementMetadataOwner : public IMessageElementMetadataOwner
    {
    public:
        void RegisterMetadata(std::shared_ptr<MessageMetadata> requestMetadata);
        void RegisterMetadata(std::shared_ptr<EnumMetadata> requestMetadata);
        // Used to find metadata registered in this context or the global well known context
        std::shared_ptr<MessageMetadata> FindMetadata(const std::string& name) override;
        // Used to find metadata registered in this context. Metadata from the global well known
        // context is not included. This is largely an implementation detail to prevent infinite
        // recursion when looking up well known types from the global context.
        std::shared_ptr<MessageMetadata> FindLocalMetadata(const std::string& name);
        std::shared_ptr<EnumMetadata> FindEnumMetadata(const std::string& name) override;
        void FinalizeMetadata();

    private:
        std::mutex _mutex;
        std::unordered_map<std::string, std::shared_ptr<MessageMetadata>> _registeredMessageMetadata;
        std::unordered_map<std::string, std::shared_ptr<EnumMetadata>> _registeredEnumMetadata;
        void UpdateMetadataClusterLayout(std::shared_ptr<MessageMetadata>& metadata);
    };
}
