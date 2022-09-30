//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <message_metadata.h>

namespace grpc_labview 
{
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class IMessageElementMetadataOwner
    {
    public:
        IMessageElementMetadataOwner() : supportNullTerminatedMessages(false) { }
        virtual std::shared_ptr<MessageMetadata> FindMetadata(const std::string& name) = 0;
        bool supportNullTerminatedMessages;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class MessageElementMetadataOwner : public IMessageElementMetadataOwner
    {    
    public:
        void RegisterMetadata(std::shared_ptr<MessageMetadata> requestMetadata);
        std::shared_ptr<MessageMetadata> FindMetadata(const std::string& name) override;
        void FinalizeMetadata();

    private:
        std::mutex _mutex;
        std::map<std::string, std::shared_ptr<MessageMetadata>> _registeredMessageMetadata;
        void UpdateMetadataClusterLayout(std::shared_ptr<MessageMetadata>& metadata);
    };
}
