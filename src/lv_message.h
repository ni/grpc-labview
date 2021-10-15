//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <message_value.h>

namespace grpc_labview 
{
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class LVMessage : public google::protobuf::Message, public LVgRPCid
    {
    public:
        LVMessage(std::shared_ptr<MessageMetadata> metadata);
        ~LVMessage();

        google::protobuf::Message* New() const final; 
        void SharedCtor();
        void SharedDtor();
        void ArenaDtor(void* object);
        void RegisterArenaDtor(google::protobuf::Arena*);

        void Clear()  final;
        bool IsInitialized() const final;

        const char* _InternalParse(const char *ptr, google::protobuf::internal::ParseContext *ctx)  override;
        google::protobuf::uint8 *_InternalSerialize(google::protobuf::uint8 *target, google::protobuf::io::EpsCopyOutputStream *stream) const override;
        void SetCachedSize(int size) const final;
        int GetCachedSize(void) const final;
        size_t ByteSizeLong() const final;
        
        void MergeFrom(const google::protobuf::Message &from) final;
        void MergeFrom(const LVMessage &from);
        void CopyFrom(const google::protobuf::Message &from) final;
        void CopyFrom(const LVMessage &from);
        void InternalSwap(LVMessage *other);
        google::protobuf::Metadata GetMetadata() const final;

        bool ParseFromByteBuffer(const grpc::ByteBuffer& buffer);
        std::unique_ptr<grpc::ByteBuffer> SerializeToByteBuffer();

    public:
        std::map<int, std::shared_ptr<LVMessageValue>> _values;
        std::shared_ptr<MessageMetadata> _metadata;

    private:
        mutable google::protobuf::internal::CachedSize _cached_size_;
        google::protobuf::UnknownFieldSet _unknownFields;

        const char *ParseBoolean(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        const char *ParseInt32(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        const char *ParseUInt32(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        const char *ParseEnum(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        const char *ParseInt64(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        const char *ParseUInt64(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        const char *ParseFloat(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        const char *ParseDouble(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        const char *ParseString(unsigned int tag, const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        const char *ParseBytes(unsigned int tag, const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        const char *ParseNestedMessage(google::protobuf::uint32 tag, const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        bool ExpectTag(google::protobuf::uint32 tag, const char* ptr);
    };
}
