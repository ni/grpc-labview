//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <message_value.h>
#include <message_metadata.h>
#include <google/protobuf/message.h>
#include "lv_message.h"
#include <type_traits>

using namespace google::protobuf::internal;

namespace grpc_labview 
{
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------

    class LVMessageEfficient : public LVMessage
    {
    public:
        LVMessageEfficient(std::shared_ptr<MessageMetadata> metadata) : LVMessage(metadata) {}
        ~LVMessageEfficient() {}

        Message* New(google::protobuf::Arena* arena) const override;
        void PostInteralParseAction() override;

    protected:
        struct RepeatedMessageValue {
            const MessageElementMetadata& _fieldInfo;
            google::protobuf::RepeatedPtrField<std::string> _buffer;
            uint64_t _numElements = 0;

            RepeatedMessageValue(const MessageElementMetadata& fieldInfo, google::protobuf::RepeatedPtrField<std::string> buffer) :
                _fieldInfo(fieldInfo), _buffer(buffer) {}
        };
    
    public:
        std::unordered_map<std::string, uint32_t> _repeatedField_continueIndex;
        std::unordered_map<std::string, std::shared_ptr<RepeatedMessageValue>> _repeatedMessageValuesMap;
        std::unordered_map<std::string, google::protobuf::RepeatedPtrField<std::string>> _repeatedStringValuesMap;

    protected:
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
    };

    template <typename MessageType, const char* (*ReadFunc)(const char*, MessageType*), const char* (*PackedFunc)(void*, const char*, google::protobuf::internal::ParseContext*)>
    class SinglePassMessageParser {
        
    private:
        LVMessage& _message;
        const char* _lv_ptr;
    public:
        // Constructor and other necessary member functions
        SinglePassMessageParser(LVMessage& message, const MessageElementMetadata& fieldInfo) : _message(message) {
            _lv_ptr = reinterpret_cast<const char*>(_message.GetLVClusterHandleSharedPtr()) + fieldInfo.clusterOffset;
        }

        // Parse and copy message in a single pass.
        template<typename RepeatedMessageValuePointer>
        const char* ParseAndCopyRepeatedMessage(const char *ptr, ParseContext *ctx, RepeatedMessageValuePointer v) {
            
            uint64_t numElements;
            ptr = PackedMessageType(ptr, ctx, reinterpret_cast<google::protobuf::RepeatedPtrField<MessageType>*>(&(v->_value)));
            numElements = v->_value.size();
            // get the LVClusterHandle

            // ContinueIndex is not required here as the _value vector created is of the corresponding type, and is not being used a buffer.
            // PackedMessageType will just be able to push_back or add the later parsed data to the type vector.

            // copy into LVCluster
            if (numElements != 0)
            {
                NumericArrayResize(0x08, 1, reinterpret_cast<void*>(const_cast<char*>(_lv_ptr)), numElements);
                auto array = *(LV1DArrayHandle*)_lv_ptr;
                (*array)->cnt = numElements;
                auto byteCount = numElements * sizeof(MessageType);
                std::memcpy((*array)->bytes<MessageType>(), v->_value.data(), byteCount);
            }
            
            return ptr;
        }

        const char* PackedMessageType(const char* ptr, ParseContext* ctx, google::protobuf::RepeatedPtrField<MessageType>* value)
        {
            return PackedFunc(value, ptr, ctx);
        }

        const char* ParseAndCopyMessage(const char *ptr) {
            ptr = ReadMessageType(ptr, reinterpret_cast<MessageType*>(const_cast<char *>(_lv_ptr)));
            return ptr;
        }

        const char* ReadMessageType(const char* ptr, MessageType* lv_ptr)
        {
            return ReadFunc(ptr, lv_ptr);
        }
    };

    // template <typename MessageType, const char* (*ReadFunc)(const char*, MessageType*), const char* (*PackedFunc)(void*, const char*, google::protobuf::internal::ParseContext*)>
    // class SinglePassMessageParser<MessageType, ReadFunc, PackedFunc, typename std::enable_if<std::is_same<MessageType, std::string>::value>::type> {
        
    // private:
    //     LVMessage& _message;
    //     const char* _lv_ptr;
    // public:
    //     // Constructor and other necessary member functions
    //     SinglePassMessageParser(LVMessage& message, const MessageElementMetadata& fieldInfo) : _message(message) {
    //         _lv_ptr = reinterpret_cast<const char*>(_message.GetLVClusterHandleSharedPtr()) + fieldInfo.clusterOffset;
    //     }

    //     // Parse and copy message in a single pass.
    //     template<typename RepeatedMessageValuePointer>
    //     const char* ParseAndCopyRepeatedMessage(const char *ptr, ParseContext *ctx, RepeatedMessageValuePointer v) {
            
    //         uint64_t numElements;
    //         ptr = PackedMessageType(ptr, ctx, reinterpret_cast<google::protobuf::RepeatedPtrField<MessageType>*>(&(v->_value)));
    //         numElements = v->_value.size();
    //         // get the LVClusterHandle

    //         // ContinueIndex is not required here as the _value vector created is of the corresponding type, and is not being used a buffer.
    //         // PackedMessageType will just be able to push_back or add the later parsed data to the type vector.

    //         // copy into LVCluster
    //         if (numElements != 0)
    //         {
    //             NumericArrayResize(0x08, 1, reinterpret_cast<void*>(const_cast<char*>(_lv_ptr)), numElements);
    //             auto array = *(LV1DArrayHandle*)_lv_ptr;
    //             (*array)->cnt = numElements;
    //             auto byteCount = numElements * sizeof(MessageType);
    //             std::memcpy((*array)->bytes<MessageType>(), v->_value.data(), byteCount);
    //         }
            
    //         return ptr;
    //     }

    //     const char* PackedMessageType(const char* ptr, ParseContext* ctx, google::protobuf::RepeatedPtrField<MessageType>* value)
    //     {
    //         return PackedFunc(value, ptr, ctx);
    //     }

    //     const char* ParseAndCopyMessage(const char *ptr) {
    //         ptr = ReadMessageType(ptr, reinterpret_cast<MessageType*>(const_cast<char *>(_lv_ptr)));
    //         return ptr;
    //     }

    //     const char* ReadMessageType(const char* ptr, MessageType* lv_ptr)
    //     {
    //         return ReadFunc(ptr, lv_ptr);
    //     }
    // };
}
