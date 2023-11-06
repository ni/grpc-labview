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
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class LVMessage : public google::protobuf::Message, public gRPCid
    {
    public:
        LVMessage(std::shared_ptr<MessageMetadata> metadata);
        LVMessage(std::shared_ptr<MessageMetadata> metadata, bool use_hardcoded_parse, bool skipCopyOnFirstParse);
        ~LVMessage();

        google::protobuf::UnknownFieldSet& UnknownFields();

        Message* New(google::protobuf::Arena* arena) const override;
        void SharedCtor();
        void SharedDtor();
        void ArenaDtor(void* object);
        void RegisterArenaDtor(google::protobuf::Arena*);

        void Clear()  final;
        bool IsInitialized() const final;

        const char* _InternalParse(const char *ptr, google::protobuf::internal::ParseContext *ctx)  override final;
        google::protobuf::uint8* _InternalSerialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override final;
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
        // std::vector<uint64_t> _messageValues;
        // std::vector<> _repeatedMessageValues;
        bool _use_hardcoded_parse;
        bool _skipCopyOnFirstParse;

        void setLVClusterHandle(const char* lvClusterHandle) {
            _LVClusterHandle = std::make_shared<const char*>(lvClusterHandle);
        };

        std::shared_ptr<const char*> getLVClusterHandleSharedPtr() {
            return _LVClusterHandle;
        };

    private:
        mutable google::protobuf::internal::CachedSize _cached_size_;
        google::protobuf::UnknownFieldSet _unknownFields;
        std::shared_ptr<const char*> _LVClusterHandle;

        const char *ParseBoolean(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        const char *ParseInt32(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        const char *ParseUInt32(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        const char *ParseEnum(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        const char *ParseInt64(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        const char *ParseUInt64(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        const char *ParseFloat(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        const char *ParseDouble(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        const char* ParseSInt32(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, google::protobuf::internal::ParseContext* ctx);
        const char* ParseSInt64(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, google::protobuf::internal::ParseContext* ctx);
        const char* ParseFixed32(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, google::protobuf::internal::ParseContext* ctx);
        const char* ParseFixed64(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, google::protobuf::internal::ParseContext* ctx);
        const char* ParseSFixed32(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, google::protobuf::internal::ParseContext* ctx);
        const char* ParseSFixed64(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, google::protobuf::internal::ParseContext* ctx);
        const char *ParseString(unsigned int tag, const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        const char *ParseBytes(unsigned int tag, const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        const char *ParseNestedMessage(google::protobuf::uint32 tag, const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, google::protobuf::internal::ParseContext *ctx);
        bool ExpectTag(google::protobuf::uint32 tag, const char* ptr);
    };

    template <typename MessageType, const char* (*ReadFunc)(const char*, MessageType*), const char* (*PackedFunc)(void*, const char*, google::protobuf::internal::ParseContext*)>
    class SinglePassMessageParser {
    private:
        LVMessage& _message;
    public:
        // Constructor and other necessary member functions
        SinglePassMessageParser(LVMessage& message) : _message(message) {}

        // Parse and copy message in a single pass.
        const char* ParseAndCopyMessage(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx, const LVMessageType type=LVMessageType::DEFAULT) {
            if (fieldInfo.isRepeated)
            {
                // Read the repeated elements into a temporary vector
                switch (type) {
                    case LVMessageType::ENUM:{
                        auto v = std::make_shared<LVRepeatedEnumMessageValue>(index);
                        ptr = CopyRepeatedMessageToCluster<std::shared_ptr<LVRepeatedEnumMessageValue>>(v, ptr, ctx, index, fieldInfo);
                        break;
                    }

                    case LVMessageType::SINT32:{
                        auto v = std::make_shared<LVRepeatedSInt32MessageValue>(index);
                        ptr = CopyRepeatedMessageToCluster<std::shared_ptr<LVRepeatedSInt32MessageValue>>(v, ptr, ctx, index, fieldInfo);
                        break;
                    }

                    case LVMessageType::SINT64:{
                        auto v = std::make_shared<LVRepeatedSInt64MessageValue>(index);
                        ptr = CopyRepeatedMessageToCluster<std::shared_ptr<LVRepeatedSInt64MessageValue>>(v, ptr, ctx, index, fieldInfo);
                        break;
                    }

                    case LVMessageType::FIXED32:{
                        auto v = std::make_shared<LVRepeatedFixed32MessageValue>(index);
                        ptr = CopyRepeatedMessageToCluster<std::shared_ptr<LVRepeatedFixed32MessageValue>>(v, ptr, ctx, index, fieldInfo);
                        break;
                    }

                    case LVMessageType::FIXED64:{
                        auto v = std::make_shared<LVRepeatedFixed64MessageValue>(index);
                        ptr = CopyRepeatedMessageToCluster<std::shared_ptr<LVRepeatedFixed64MessageValue>>(v, ptr, ctx, index, fieldInfo);
                        break;
                    }

                    case LVMessageType::SFIXED32:{
                        auto v = std::make_shared<LVRepeatedSFixed32MessageValue>(index);
                        ptr = CopyRepeatedMessageToCluster<std::shared_ptr<LVRepeatedSFixed32MessageValue>>(v, ptr, ctx, index, fieldInfo);
                        break;
                    }

                    case LVMessageType::SFIXED64:{
                        auto v = std::make_shared<LVRepeatedSFixed64MessageValue>(index);
                        ptr = CopyRepeatedMessageToCluster<std::shared_ptr<LVRepeatedSFixed64MessageValue>>(v, ptr, ctx, index, fieldInfo);
                        break; 
                    }

                    case LVMessageType::DEFAULT:{
                        auto v = std::make_shared<LVRepeatedMessageValue<MessageType>>(index);
                        ptr = CopyRepeatedMessageToCluster<std::shared_ptr<LVRepeatedMessageValue<MessageType>>>(v, ptr, ctx, index, fieldInfo);
                    }
                }
            }
            else
            {
                auto _lv_ptr = reinterpret_cast<const char*>(*(_message.getLVClusterHandleSharedPtr().get())) + fieldInfo.clusterOffset;
                ptr = ReadMessageType(ptr, reinterpret_cast<MessageType*>(const_cast<char *>(_lv_ptr)));
            }
            return ptr;
        }

        template <typename RepeatedMessageType>
        const char* CopyRepeatedMessageToCluster(RepeatedMessageType v, const char *ptr, ParseContext* ctx, uint32_t index, MessageElementMetadata fieldInfo) {
            uint64_t numElements;
            ptr = PackedMessageType(ptr, ctx, index, reinterpret_cast<google::protobuf::RepeatedField<MessageType>*>(&(v->_value)));
            numElements = v->_value.size();
            // get the LVClusterHandle
            auto start = reinterpret_cast<const char*>(*(_message.getLVClusterHandleSharedPtr().get())) + fieldInfo.clusterOffset;

            // copy into LVCluster
            if (numElements != 0)
            {
                NumericArrayResize(0x08, 1, reinterpret_cast<void*>(const_cast<char*>(start)), numElements);
                auto array = *(LV1DArrayHandle*)start;
                (*array)->cnt = numElements;
                auto byteCount = numElements * sizeof(MessageType);
                std::memcpy((*array)->bytes<MessageType>(), v->_value.data(), byteCount);
            }
            return ptr;
        }

        const char* ReadMessageType(const char* ptr, MessageType* lv_ptr)
        {
            return ReadFunc(ptr, lv_ptr);
        }

        const char* PackedMessageType(const char* ptr, ParseContext* ctx, int index, google::protobuf::RepeatedField<MessageType>* value)
        {
            return PackedFunc(value, ptr, ctx);
        }
    };
}
