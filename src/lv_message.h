//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <message_value.h>
#include <message_metadata.h>
#include <google/protobuf/message.h>

using namespace google::protobuf::internal;

namespace grpc_labview
{

    inline const char* ReadBOOL(const char* ptr, bool* value) {
        *value = static_cast<bool>(ReadVarint64(&ptr));
        return ptr;
    }

    inline const char* ReadINT32(const char* ptr, int32_t* value) {
        return VarintParse(ptr, reinterpret_cast<uint32_t*>(value));
    }

    inline const char* ReadUINT32(const char* ptr, uint32_t* value) {
        return VarintParse(ptr, value);
    }

    template <typename E>
    inline const char* ReadENUM(const char* ptr, E* value) {
        *value = static_cast<E>(ReadVarint32(&ptr));
        return ptr;
    }

    inline const char* ReadINT64(const char* ptr, int64_t* value) {
        return VarintParse(ptr, reinterpret_cast<uint64_t*>(value));
    }

    inline const char* ReadUINT64(const char* ptr, uint64_t* value) {
        return VarintParse(ptr, value);
    }

    template <typename F>
    inline const char* ReadUnaligned(const char* ptr, F* value) {
        *value = UnalignedLoad<F>(ptr);
        return ptr + sizeof(F);
    }

    inline const char* ReadFLOAT(const char* ptr, float* value) {
        return ReadUnaligned(ptr, value);
    }

    inline const char* ReadDOUBLE(const char* ptr, double* value) {
        return ReadUnaligned(ptr, value);
    }

    inline const char* ReadSINT32(const char* ptr, int32_t* value) {
        *value = ReadVarintZigZag32(&ptr);
        return ptr;
    }

    inline const char* ReadSINT64(const char* ptr, int64_t* value) {
        *value = ReadVarintZigZag64(&ptr);
        return ptr;
    }

    inline const char* ReadFIXED32(const char* ptr, uint32_t* value) {
        return ReadUnaligned(ptr, value);
    }

    inline const char* ReadFIXED64(const char* ptr, uint64_t* value) {
        return ReadUnaligned(ptr, value);
    }

    inline const char* ReadSFIXED32(const char* ptr, int32_t* value) {
        return ReadUnaligned(ptr, value);
    }

    inline const char* ReadSFIXED64(const char* ptr, int64_t* value) {
        return ReadUnaligned(ptr, value);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class LVMessage : public google::protobuf::Message, public gRPCid
    {
    public:
        LVMessage(std::shared_ptr<MessageMetadata> metadata);
        ~LVMessage();

        google::protobuf::UnknownFieldSet& UnknownFields();

        Message* New(google::protobuf::Arena* arena) const;
        void SharedCtor();
        void SharedDtor();
        void ArenaDtor(void* object);
        void RegisterArenaDtor(google::protobuf::Arena*);

        void Clear()  final;
        bool IsInitialized() const;

        const char* _InternalParse(const char* ptr, google::protobuf::internal::ParseContext* ctx);
        google::protobuf::uint8* _InternalSerialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override final;
        void SetCachedSize(int size) const;
        int GetCachedSize(void) const;
        size_t ByteSizeLong() const final;
        virtual void PostInteralParseAction() {};

        void MergeFrom(const google::protobuf::Message &from);
        void MergeFrom(const LVMessage &from);
        void CopyFrom(const google::protobuf::Message &from);
        void CopyFrom(const LVMessage &from);
        void CopyOneofIndicesToCluster(int8_t* cluster) const;
        void InternalSwap(LVMessage *other);
        google::protobuf::Metadata GetMetadata() const;

        bool ParseFromByteBuffer(const grpc::ByteBuffer& buffer);
        std::unique_ptr<grpc::ByteBuffer> SerializeToByteBuffer();

        std::map<int, std::shared_ptr<LVMessageValue>> _values;
        std::shared_ptr<MessageMetadata> _metadata;
        std::map<std::string, int> _oneofContainerToSelectedIndexMap;
        const google::protobuf::internal::ClassData* GetClassData() const override final;
    protected:
        mutable google::protobuf::internal::CachedSize _cached_size_;
        google::protobuf::UnknownFieldSet _unknownFields;

        virtual const char *ParseBoolean(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        virtual const char *ParseInt32(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        virtual const char *ParseUInt32(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        virtual const char *ParseEnum(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        virtual const char *ParseInt64(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        virtual const char *ParseUInt64(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        virtual const char *ParseFloat(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        virtual const char *ParseDouble(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        virtual const char* ParseSInt32(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, google::protobuf::internal::ParseContext* ctx);
        virtual const char* ParseSInt64(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, google::protobuf::internal::ParseContext* ctx);
        virtual const char* ParseFixed32(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, google::protobuf::internal::ParseContext* ctx);
        virtual const char* ParseFixed64(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, google::protobuf::internal::ParseContext* ctx);
        virtual const char* ParseSFixed32(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, google::protobuf::internal::ParseContext* ctx);
        virtual const char* ParseSFixed64(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, google::protobuf::internal::ParseContext* ctx);
        virtual const char *ParseString(unsigned int tag, const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        virtual const char *ParseBytes(unsigned int tag, const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        virtual const char *ParseNestedMessage(google::protobuf::uint32 tag, const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        bool ExpectTag(google::protobuf::uint32 tag, const char* ptr);
        int CalculateTagWireSize(google::protobuf::uint32 tag);
    };
}
