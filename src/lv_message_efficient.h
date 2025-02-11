//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <message_value.h>
#include <message_metadata.h>
#include <google/protobuf/message.h>
#include "lv_message.h"

using namespace google::protobuf::internal;

namespace grpc_labview
{
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class LVMessageEfficient : public LVMessage
    {
    public:
        LVMessageEfficient(std::shared_ptr<MessageMetadata> metadata, int8_t* cluster) : LVMessage(metadata), _LVClusterHandle(cluster) {}
        ~LVMessageEfficient() {}

        Message* New(google::protobuf::Arena* arena) const override;
        void PostInteralParseAction() override;
        int8_t* GetLVClusterHandle() { return _LVClusterHandle; };

    protected:
        struct RepeatedMessageValue {
            const MessageElementMetadata& _fieldInfo;
            google::protobuf::RepeatedField<char> _buffer;
            uint64_t _numElements = 0;

            RepeatedMessageValue(const MessageElementMetadata& fieldInfo, google::protobuf::RepeatedField<char> buffer) :
                _fieldInfo(fieldInfo), _buffer(buffer) {}
        };

        struct RepeatedStringValue {
            const MessageElementMetadata& _fieldInfo;
            google::protobuf::RepeatedField<std::string> _repeatedString;

            RepeatedStringValue(const MessageElementMetadata& fieldInfo) :
                _fieldInfo(fieldInfo), _repeatedString(google::protobuf::RepeatedField<std::string>()) {}
        };

    public:
        std::unordered_map<std::string, std::shared_ptr<RepeatedMessageValue>> _repeatedMessageValuesMap;
        std::unordered_map<std::string, std::shared_ptr<RepeatedStringValue>> _repeatedStringValuesMap;

    protected:
        int8_t* _LVClusterHandle;

        const char* ParseBoolean(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, google::protobuf::internal::ParseContext* ctx) override;
        const char* ParseInt32(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, google::protobuf::internal::ParseContext* ctx) override;
        const char* ParseUInt32(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, google::protobuf::internal::ParseContext* ctx) override;
        const char* ParseEnum(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, google::protobuf::internal::ParseContext* ctx) override;
        const char* ParseInt64(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, google::protobuf::internal::ParseContext* ctx) override;
        const char* ParseUInt64(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, google::protobuf::internal::ParseContext* ctx) override;
        const char* ParseFloat(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, google::protobuf::internal::ParseContext* ctx) override;
        const char* ParseDouble(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, google::protobuf::internal::ParseContext* ctx) override;
        const char* ParseSInt32(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, google::protobuf::internal::ParseContext* ctx) override;
        const char* ParseSInt64(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, google::protobuf::internal::ParseContext* ctx) override;
        const char* ParseFixed32(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, google::protobuf::internal::ParseContext* ctx) override;
        const char* ParseFixed64(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, google::protobuf::internal::ParseContext* ctx) override;
        const char* ParseSFixed32(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, google::protobuf::internal::ParseContext* ctx) override;
        const char* ParseSFixed64(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, google::protobuf::internal::ParseContext* ctx) override;
        const char* ParseString(unsigned int tag, const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, google::protobuf::internal::ParseContext* ctx) override;
        const char* ParseBytes(unsigned int tag, const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, google::protobuf::internal::ParseContext* ctx) override;
        const char* ParseNestedMessage(google::protobuf::uint32 tag, const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, google::protobuf::internal::ParseContext* ctx) override;
        const char* ParseDouble2DArrayMessage(google::protobuf::uint32 tag, const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, google::protobuf::internal::ParseContext* ctx);
    };

    template <typename MessageType, const char* (*ReadFunc)(const char*, MessageType*), const char* (*PackedFunc)(void*, const char*, google::protobuf::internal::ParseContext*)>
    class SinglePassMessageParser {
    private:
        LVMessageEfficient& _message;
        int8_t* _lv_ptr;
    public:
        // Constructor and other necessary member functions
        SinglePassMessageParser(LVMessageEfficient& message, const MessageElementMetadata& fieldInfo) : _message(message) {
            _lv_ptr = _message.GetLVClusterHandle() + fieldInfo.clusterOffset;
        }

        // Parse and copy message in a single pass.
        template<typename RepeatedMessageValuePointer>
        const char* ParseAndCopyRepeatedMessage(const char* ptr, ParseContext* ctx, RepeatedMessageValuePointer v) {

            uint64_t numElements;
            ptr = PackedMessageType(ptr, ctx, reinterpret_cast<google::protobuf::RepeatedField<MessageType>*>(&(v->_value)));
            numElements = v->_value.size();
            // get the LVClusterHandle

            // ContinueIndex is not required here as the _value vector created is of the corresponding type, and is not being used a buffer.
            // PackedMessageType will just be able to push_back or add the later parsed data to the type vector.

            // copy into LVCluster
            if (numElements != 0)
            {
                auto messageTypeSize = sizeof(MessageType);
                NumericArrayResize(GetTypeCodeForSize(messageTypeSize), 1, _lv_ptr, numElements);
                auto array = *(LV1DArrayHandle*)_lv_ptr;
                (*array)->cnt = numElements;
                auto byteCount = numElements * messageTypeSize;
                std::memcpy((*array)->bytes<MessageType>(), v->_value.data(), byteCount);
            }

            return ptr;
        }

        const char* PackedMessageType(const char* ptr, ParseContext* ctx, google::protobuf::RepeatedField<MessageType>* value)
        {
            return PackedFunc(value, ptr, ctx);
        }

        const char* ParseAndCopyMessage(const char* ptr) {
            ptr = ReadMessageType(ptr, reinterpret_cast<MessageType*>(_lv_ptr));
            return ptr;
        }

        const char* ReadMessageType(const char* ptr, MessageType* lv_ptr)
        {
            return ReadFunc(ptr, lv_ptr);
        }
    };
}
