//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_server.h>
#include "lv_message_efficient.h"
#include <sstream>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
using namespace google::protobuf::internal;

namespace grpc_labview
{
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    google::protobuf::Message* LVMessageEfficient::New(google::protobuf::Arena* arena) const
    {
        assert(false); // not expected to be called
        return nullptr;
    }

#define DEFINE_PARSE_FUNCTION(Type, TypeName, ReadType, ParserType) \
    const char *LVMessageEfficient::Parse##TypeName(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx) \
    { \
        SinglePassMessageParser<Type, Read##ReadType, Packed##ParserType##Parser> parser(*this, fieldInfo); \
        if (fieldInfo.isRepeated) \
        { \
            auto v = std::make_shared<LVRepeatedMessageValue<Type>>(index); \
            ptr = parser.ParseAndCopyRepeatedMessage(ptr, ctx, v); \
        } \
        else \
        { \
            ptr = parser.ParseAndCopyMessage(ptr); \
        } \
        return ptr; \
    }

#define DEFINE_PARSE_FUNCTION_SPECIAL(Type, TypeName, ReadType, ParserType) \
    const char *LVMessageEfficient::Parse##TypeName(const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx) \
    { \
        SinglePassMessageParser<Type, Read##ReadType, Packed##ParserType##Parser> parser(*this, fieldInfo); \
        if (fieldInfo.isRepeated) \
        { \
            auto v = std::make_shared<LVRepeated##TypeName##MessageValue>(index); \
            ptr = parser.ParseAndCopyRepeatedMessage(ptr, ctx, v); \
        } \
        else \
        { \
            ptr = parser.ParseAndCopyMessage(ptr); \
        } \
        return ptr; \
    }

    DEFINE_PARSE_FUNCTION(bool, Boolean, BOOL, Bool)
    DEFINE_PARSE_FUNCTION(int32_t, Int32, INT32, Int32)
    DEFINE_PARSE_FUNCTION(uint32_t, UInt32, UINT32, UInt32)
    DEFINE_PARSE_FUNCTION(int64_t, Int64, INT64, Int64)
    DEFINE_PARSE_FUNCTION(uint64_t, UInt64, UINT64, UInt64)
    DEFINE_PARSE_FUNCTION(float, Float, FLOAT, Float)
    DEFINE_PARSE_FUNCTION(double, Double, DOUBLE, Double)
    DEFINE_PARSE_FUNCTION_SPECIAL(int32_t, Enum, ENUM, Enum)
    DEFINE_PARSE_FUNCTION_SPECIAL(int32_t, SInt32, SINT32, SInt32)
    DEFINE_PARSE_FUNCTION_SPECIAL(int64_t, SInt64, SINT64, SInt64)
    DEFINE_PARSE_FUNCTION_SPECIAL(uint32_t, Fixed32, FIXED32, Fixed32)
    DEFINE_PARSE_FUNCTION_SPECIAL(uint64_t, Fixed64, FIXED64, Fixed64)
    DEFINE_PARSE_FUNCTION_SPECIAL(int32_t, SFixed32, SFIXED32, SFixed32)
    DEFINE_PARSE_FUNCTION_SPECIAL(int64_t, SFixed64, SFIXED64, SFixed64)

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessageEfficient::ParseString(google::protobuf::uint32 tag, const MessageElementMetadata& fieldInfo, uint32_t index, const char *protobuf_ptr, ParseContext *ctx)
    {    
        const char* lv_ptr = (this->GetLVClusterHandleSharedPtr()) + fieldInfo.clusterOffset;

        if (fieldInfo.isRepeated)
        {
            // Get the _repeatedMessageValues vector from the map
            auto _repeatedStringValuesIt = _repeatedStringValuesMap.find(fieldInfo.fieldName);
            if (_repeatedStringValuesIt == _repeatedStringValuesMap.end())
            {
                _repeatedStringValuesIt = _repeatedStringValuesMap.emplace(fieldInfo.fieldName, google::protobuf::RepeatedPtrField<std::string>()).first;
            }

            protobuf_ptr -= 1;
            do {
                protobuf_ptr += 1;
                auto str = _repeatedStringValuesIt->second.Add();
                protobuf_ptr = InlineGreedyStringParser(str, protobuf_ptr, ctx);
                if (!ctx->DataAvailable(protobuf_ptr))
                {
                    break;
                }
            } while (ExpectTag(tag, protobuf_ptr));

            auto arraySize = sizeof(void*) * _repeatedStringValuesIt->second.size();
            auto _lvProvidedArrayHandle = *(void**)lv_ptr; 
            *(void**)lv_ptr = DSNewHandle(arraySize);
            auto arrayHandle = *(LV1DArrayHandle*)lv_ptr;
            (*arrayHandle)->cnt = _repeatedStringValuesIt->second.size();

            // Copy the repeated string values into the LabVIEW array
            auto lvStringPtr = (*arrayHandle)->bytes<LStrHandle>();
            for (auto str:_repeatedStringValuesIt->second)
            {
                *lvStringPtr = nullptr;
                SetLVString(lvStringPtr, str);
                lvStringPtr++;
            }
        }
        else {
            auto str = std::string();
            protobuf_ptr = InlineGreedyStringParser(&str, protobuf_ptr, ctx);
            SetLVString((LStrHandle*)lv_ptr, str);
        }
        return protobuf_ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessageEfficient::ParseBytes(google::protobuf::uint32 tag, const MessageElementMetadata& fieldInfo, uint32_t index, const char *ptr, ParseContext *ctx)
    {
        return ParseString(tag, fieldInfo, index, ptr, ctx);    
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char *LVMessageEfficient::ParseNestedMessage(google::protobuf::uint32 tag, const MessageElementMetadata& fieldInfo, uint32_t index, const char *protobuf_ptr, ParseContext *ctx)
    {    
        auto metadata = fieldInfo._owner->FindMetadata(fieldInfo.embeddedMessageName);
        
        LVMessageEfficient nestedMessage(metadata);

        if (fieldInfo.isRepeated) {
            // if the array is not big enough, resize it to 2x the size
            auto numElements = 128;
            auto elementIndex = 0;
            auto clusterSize = metadata->clusterSize;
            auto alignment = metadata->alignmentRequirement;
            auto arraySize = numElements * clusterSize;
            char _fillData = '\0';

            // Get the _repeatedMessageValues vector from the map
            auto _repeatedMessageValuesIt = _repeatedMessageValuesMap.find(metadata->messageName);
            if (_repeatedMessageValuesIt == _repeatedMessageValuesMap.end())
            {
                auto m_val = std::make_shared<RepeatedMessageValue>(fieldInfo, google::protobuf::RepeatedPtrField<std::string>());
                _repeatedMessageValuesIt = _repeatedMessageValuesMap.emplace(metadata->messageName, m_val).first;
            }

            // There are situations where the protobuf message is not complete, and we need to continue from the last index.
            // This function returns to _internalParse, and then gets back to this function.
            // If we are continuing from a previous parse, then we need to continue from the last index
            auto _continueFromIndex = _repeatedField_continueIndex.find(metadata->messageName);
            if (_continueFromIndex != _repeatedField_continueIndex.end()) {
                elementIndex = _continueFromIndex->second;
                _repeatedField_continueIndex.erase(_continueFromIndex);
                // find next largest power of 2, as we assume that we previously resized it to a power of 2
                auto _size = (int)ceil(log2(elementIndex));
                numElements = ((1 << _size) > 128) ? (1 << _size) : 128;
            }
            else {
                // occurs on the first time this function is called
                _repeatedMessageValuesIt->second.get()->_buffer.Reserve(numElements);
            }

            protobuf_ptr -= 1;
            do
            {
                protobuf_ptr += 1;

                // Resize the vector if we need more memory
                if (elementIndex >= numElements - 1) {
                    numElements *= 2;
                    arraySize = numElements * clusterSize;
                    auto& repeatedField = _repeatedMessageValuesIt->second.get()->_buffer;
                    repeatedField.Reserve(numElements);
                }

                auto _vectorPtr = _repeatedMessageValuesIt->second.get()->_buffer.data();
                _vectorPtr = _vectorPtr + (elementIndex * clusterSize);
                nestedMessage.SetLVClusterHandle(reinterpret_cast<const char*>(_vectorPtr));
                protobuf_ptr = ctx->ParseMessage(&nestedMessage, protobuf_ptr);

                elementIndex++;

                if (!ctx->DataAvailable(protobuf_ptr)) {
                    break;
                }
            } while (ExpectTag(tag, protobuf_ptr));
            
            _repeatedMessageValuesIt->second.get()->_numElements = elementIndex;
            _repeatedField_continueIndex.emplace(metadata->messageName, elementIndex);
        }
        else {
            const char* lv_ptr = (this->GetLVClusterHandleSharedPtr()) + fieldInfo.clusterOffset;
            nestedMessage.SetLVClusterHandle(lv_ptr);
            protobuf_ptr = ctx->ParseMessage(&nestedMessage, protobuf_ptr);
        }
        return protobuf_ptr;
    }

    void LVMessageEfficient::PostInteralParseAction()
    {
        for (auto nestedMessage : _repeatedMessageValuesMap)
        {
            auto fieldInfo = nestedMessage.second.get()->_fieldInfo;
            auto buffer = nestedMessage.second.get()->_buffer;
            auto numElements = nestedMessage.second.get()->_numElements;

            auto metadata = fieldInfo._owner->FindMetadata(fieldInfo.embeddedMessageName);
            const char* lv_ptr = (this->GetLVClusterHandleSharedPtr()) + fieldInfo.clusterOffset;
            auto clusterSize = metadata->clusterSize;
            auto alignment = metadata->alignmentRequirement;


            // shrink the array to the correct size
            auto arraySize = numElements * clusterSize;
            auto old_arrayHandle = *(void**)lv_ptr;
            DSDisposeHandle(old_arrayHandle);
            *(void**)lv_ptr = DSNewHandle(arraySize);
            auto arrayHandle = *(LV1DArrayHandle*)lv_ptr;
            (*arrayHandle)->cnt = numElements;

            auto _vectorDataPtr = buffer.data();
            auto _lvArrayDataPtr = (*arrayHandle)->bytes(0, alignment);
            memcpy(_lvArrayDataPtr, _vectorDataPtr, arraySize);
        }
    }

}

