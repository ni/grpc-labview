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
    const char* LVMessageEfficient::ParseString(google::protobuf::uint32 tag, const MessageElementMetadata& fieldInfo, uint32_t index, const char* protobuf_ptr, ParseContext* ctx)
    {
        const char* lv_ptr = (this->GetLVClusterHandleSharedPtr()) + fieldInfo.clusterOffset;

        if (fieldInfo.isRepeated)
        {
            auto repeatedString = google::protobuf::RepeatedField<std::string>();
            protobuf_ptr -= 1;
            do {
                protobuf_ptr += 1;
                auto str = repeatedString.Add();
                protobuf_ptr = InlineGreedyStringParser(str, protobuf_ptr, ctx);
                if (!ctx->DataAvailable(protobuf_ptr))
                {
                    break;
                }
            } while (ExpectTag(tag, protobuf_ptr));


            NumericArrayResize(GetTypeCodeForSize(sizeof(char*)), 1, reinterpret_cast<void*>(const_cast<char*>(lv_ptr)), repeatedString.size());
            auto arrayHandle = *(LV1DArrayHandle*)lv_ptr;
            (*arrayHandle)->cnt = repeatedString.size();

            // Copy the repeated string values into the LabVIEW array
            auto lvStringPtr = (*arrayHandle)->bytes<LStrHandle>();
            for (auto str : repeatedString)
            {
                *lvStringPtr = nullptr;
                SetLVString(lvStringPtr, str);
                lvStringPtr++;
            }
        }
        else
        {
            auto str = std::string();
            protobuf_ptr = InlineGreedyStringParser(&str, protobuf_ptr, ctx);
            SetLVString((LStrHandle*)lv_ptr, str);
        }
        return protobuf_ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char* LVMessageEfficient::ParseBytes(google::protobuf::uint32 tag, const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, ParseContext* ctx)
    {
        return ParseString(tag, fieldInfo, index, ptr, ctx);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char* LVMessageEfficient::ParseNestedMessage(google::protobuf::uint32 tag, const MessageElementMetadata& fieldInfo, uint32_t index, const char* protobuf_ptr, ParseContext* ctx)
    {
        auto metadata = fieldInfo._owner->FindMetadata(fieldInfo.embeddedMessageName);

        LVMessageEfficient nestedMessage(metadata);

        if (fieldInfo.isRepeated)
        {
            // if the array is not big enough, resize it to 2x the size
            auto numElements = 128;
            auto elementIndex = 0;
            auto clusterSize = metadata->clusterSize;
            auto arraySize = numElements * clusterSize;
            char _fillData = '\0';

            // Get the _repeatedMessageValues vector from the map
            auto _repeatedMessageValuesIt = _repeatedMessageValuesMap.find(fieldInfo.fieldName);
            if (_repeatedMessageValuesIt == _repeatedMessageValuesMap.end())
            {
                auto m_val = std::make_shared<RepeatedMessageValue>(fieldInfo, google::protobuf::RepeatedField<char>());
                _repeatedMessageValuesIt = _repeatedMessageValuesMap.emplace(fieldInfo.fieldName, m_val).first;
                _repeatedMessageValuesIt->second.get()->_buffer.Resize(arraySize, _fillData);
            }
            else
            {
                // Zero out data from the previous use of the buffer.
                auto bytesUsed = _repeatedMessageValuesIt->second.get()->_numElements * clusterSize;
                auto bufferPtr = _repeatedMessageValuesIt->second.get()->_buffer.data();
                memset(reinterpret_cast<void*>(const_cast<char*>(bufferPtr)), _fillData, bytesUsed);

                // Recalculate number of cluster elements that can fit in the buffer based on its current capacity.
                numElements = _repeatedMessageValuesIt->second.get()->_buffer.Capacity() / clusterSize;
            }
            _repeatedMessageValuesIt->second.get()->_validData = true;

            protobuf_ptr -= 1;
            do
            {
                protobuf_ptr += 1;

                // Resize the vector if we need more memory
                if (elementIndex >= numElements - 1)
                {
                    numElements *= 2;
                    arraySize = numElements * clusterSize;
                    _repeatedMessageValuesIt->second.get()->_buffer.Resize(arraySize, _fillData);
                }

                auto _vectorPtr = _repeatedMessageValuesIt->second.get()->_buffer.data();
                _vectorPtr = _vectorPtr + (elementIndex * clusterSize);
                nestedMessage.SetLVClusterHandle(_vectorPtr);
                protobuf_ptr = ctx->ParseMessage(&nestedMessage, protobuf_ptr);

                elementIndex++;

                if (!ctx->DataAvailable(protobuf_ptr))
                {
                    break;
                }
            } while (ExpectTag(tag, protobuf_ptr));

            _repeatedMessageValuesIt->second.get()->_numElements = elementIndex;
        }
        else
        {
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
            if (!nestedMessage.second.get()->_validData)
            {
                continue;
            }

            auto fieldInfo = nestedMessage.second.get()->_fieldInfo;
            auto buffer = nestedMessage.second.get()->_buffer;
            auto numClusters = nestedMessage.second.get()->_numElements;

            auto metadata = fieldInfo._owner->FindMetadata(fieldInfo.embeddedMessageName);
            const char* lv_ptr = (this->GetLVClusterHandleSharedPtr()) + fieldInfo.clusterOffset;
            auto clusterSize = metadata->clusterSize;
            auto alignment = metadata->alignmentRequirement;


            // Allocate an array with the correct size and alignment for the cluster.
            auto byteSize = numClusters * clusterSize;
            auto alignedElementSize = byteSize / alignment;
            if (byteSize % alignment != 0)
            {
                alignedElementSize++;
            }
            auto numDimensions = 1;
            NumericArrayResize(GetTypeCodeForSize(alignment), numDimensions, reinterpret_cast<void*>(const_cast<char*>(lv_ptr)), alignedElementSize);
            auto arrayHandle = *(LV1DArrayHandle*)lv_ptr;
            (*arrayHandle)->cnt = numClusters;

            auto _vectorDataPtr = buffer.data();
            auto _lvArrayDataPtr = (*arrayHandle)->bytes(0, alignment);
            memcpy(_lvArrayDataPtr, _vectorDataPtr, byteSize);
            nestedMessage.second.get()->_validData = false;
        }
    }
}

