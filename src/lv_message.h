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
        bool _use_hardcoded_parse;
        bool _skipCopyOnFirstParse;

        void setLVClusterHandle(int8_t* lvClusterHandle) {
            _LVClusterHandle = std::make_shared<int8_t*>(lvClusterHandle);
        };

        std::shared_ptr<int8_t*> getLVClusterHandleSharedPtr() {
            return _LVClusterHandle;
        };

    private:
        mutable google::protobuf::internal::CachedSize _cached_size_;
        google::protobuf::UnknownFieldSet _unknownFields;
        std::shared_ptr<int8_t*> _LVClusterHandle;

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
    
    /*public:
        template <typename MessageType>
        class SinglePassMessageParser {
        private:
            LVMessage& _message;
        public:
            // Constructor and other necessary member functions
            SinglePassMessageParser(LVMessage& message) : _message(message) {}

            // Parse and copy message in a single pass.
            void ParseAndCopyMessage(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx) {
                if (fieldInfo.isRepeated)
                {
                    // Read the repeated elements into a temporary vector
                    uint64_t numElements;
                    auto v = PackedMessageType(ptr, ctx, &numElements);
                    // get the LVClusterHandle
                    auto start = reinterpret_cast<int8_t*>(*(_message.getLVClusterHandleSharedPtr().get())) + fieldInfo.clusterOffset;

                    // copy into LVCluster
                    if (numElements != 0)
                    {
                        NumericArrayResize(0x08, 1, start, numElements);
                        auto array = *(LV1DArrayHandle*)start;
                        (*array)->cnt = numElements;
                        auto byteCount = numElements * sizeof(MessageType);
                        std::memcpy((*array)->bytes<MessageType>(), v->_value.data(), byteCount);
                    }
                }
                else
                {
                    auto _lv_ptr = reinterpret_cast<int8_t*>(*(_message.getLVClusterHandleSharedPtr().get())) + fieldInfo.clusterOffset;
                    ptr = ReadMessageType(ptr, reinterpret_cast<MessageType*>(_lv_ptr));
                }
            }

            const char* ReadMessageType(const char* ptr, MessageType* lv_ptr)
            {
                return nullptr;
            }

            LVRepeatedMessageValue* PackedMessageType(const char* ptr, ParseContext* ctx, uint64_t* numElements)
            {
                return nullptr;
            }

            // const char* ReadMessageType<int32_t>(const char* ptr, int32_t* lv_ptr)
            // {
            //     return ReadINT32(ptr, _lv_ptr);
            // }

            // LVRepeatedMessageValue<int>* PackedMessageType<int32_t>(const char* ptr, ParseContext* ctx, uint64_t* numElements)
            // {
            //     auto v = std::make_shared<LVRepeatedMessageValue<int>>(index);
            //     ptr = PackedInt32Parser(&(v->_value), ptr, ctx);
            //     *numElements = v->_value.size();
            //     return v;
            // }

            // const char* ReadMessageType<int64_t>(const char* ptr, int32_t* lv_ptr)
            // {
            //     return ReadINT32(ptr, _lv_ptr);
            // }

            // LVRepeatedInt64MessageValue* PackedMessageType<int64_t>(const char* ptr, ParseContext* ctx, uint64_t* numElements)
            // {
            //     auto v = std::make_shared<LVRepeatedMessageValue<int>>(index);
            //     ptr = PackedInt32Parser(&(v->_value), ptr, ctx);
            //     *numElements = v->_value.size();
            //     return v;
            // }

            // const char* ReadMessageType<uint32_t>(const char* ptr, int32_t* lv_ptr)
            // {
            //     return ReadUINT32(ptr, _lv_ptr);
            // }

            // LVRepeatedMessageValue<uint32_t>* PackedMessageType<uint32_t>(const char* ptr, ParseContext* ctx, uint64_t* numElements)
            // {
            //     auto v = std::make_shared<LVRepeatedMessageValue<uint32_t>>(index);
            //     ptr = PackedUInt32Parser(&(v->_value), ptr, ctx);
            //     *numElements = v->_value.size();
            //     return v;
            // }

            // const char* ReadMessageType<uint64_t>(const char* ptr, int32_t* lv_ptr)
            // {
            //     return ReadUINT64(ptr, _lv_ptr);
            // }

            // LVRepeatedUInt64MessageValue* PackedMessageType<uint64_t>(const char* ptr, ParseContext* ctx, uint64_t* numElements)
            // {
            //     auto v = std::make_shared<LVRepeatedUInt64MessageValue>(index);
            //     ptr = PackedUInt64Parser(&(v->_value), ptr, ctx);
            //     *numElements = v->_value.size();
            //     return v;
            // }
        };*/

    };
}
