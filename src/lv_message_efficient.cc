//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_server.h>
#include <lv_message_efficient.h>
#include <well_known_messages.h>
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
    DEFINE_PARSE_FUNCTION_SPECIAL(int32_t, SInt32, SINT32, SInt32)
    DEFINE_PARSE_FUNCTION_SPECIAL(int64_t, SInt64, SINT64, SInt64)
    DEFINE_PARSE_FUNCTION_SPECIAL(uint32_t, Fixed32, FIXED32, Fixed32)
    DEFINE_PARSE_FUNCTION_SPECIAL(uint64_t, Fixed64, FIXED64, Fixed64)
    DEFINE_PARSE_FUNCTION_SPECIAL(int32_t, SFixed32, SFIXED32, SFixed32)
    DEFINE_PARSE_FUNCTION_SPECIAL(int64_t, SFixed64, SFIXED64, SFixed64)

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char* LVMessageEfficient::ParseEnum(const MessageElementMetadata& fieldInfo, uint32_t index, const char* ptr, ParseContext* ctx)
    {
        std::shared_ptr<EnumMetadata> enumMetadata = fieldInfo._owner->FindEnumMetadata(fieldInfo.embeddedMessageName);
        auto lv_ptr = _LVClusterHandle + fieldInfo.clusterOffset;

        if (fieldInfo.isRepeated)
        {
            auto repeatedEnum = std::make_shared<LVRepeatedEnumMessageValue>(index);
            ptr = PackedEnumParser(&(repeatedEnum->_value), ptr, ctx);

            int count = repeatedEnum->_value.size();
            for (size_t i = 0; i < count; i++)
            {
                auto enumValueFromProtobuf = repeatedEnum->_value[i];
                repeatedEnum->_value[i] = enumMetadata->GetLVEnumValueFromProtoValue(enumValueFromProtobuf);
            }

            if (count != 0)
            {
                auto messageTypeSize = sizeof(int32_t);
                NumericArrayResize(GetTypeCodeForSize(messageTypeSize), 1, reinterpret_cast<void*>(lv_ptr), count);
                auto array = *(LV1DArrayHandle*)lv_ptr;
                (*array)->cnt = count;
                auto byteCount = count * sizeof(int32_t);
                memcpy((*array)->bytes<int32_t>(), repeatedEnum->_value.data(), byteCount);
            }
        }
        else
        {
            int32_t enumValueFromProtobuf;
            ptr = ReadENUM(ptr, &enumValueFromProtobuf);
            *(int32_t*)lv_ptr = enumMetadata->GetLVEnumValueFromProtoValue(enumValueFromProtobuf);
        }
        return ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char* LVMessageEfficient::ParseString(google::protobuf::uint32 tag, const MessageElementMetadata& fieldInfo, uint32_t index, const char* protobuf_ptr, ParseContext* ctx)
    {
        if (fieldInfo.isRepeated)
        {
            auto repeatedStringValuesIt = _repeatedStringValuesMap.find(fieldInfo.fieldName);
            if (repeatedStringValuesIt == _repeatedStringValuesMap.end())
            {
                auto m_val = std::make_shared<RepeatedStringValue>(fieldInfo);
                repeatedStringValuesIt = _repeatedStringValuesMap.emplace(fieldInfo.fieldName, m_val).first;
            }

            auto& repeatedString = repeatedStringValuesIt->second.get()->_repeatedString;
            auto tagSize = CalculateTagWireSize(tag);
            protobuf_ptr -= tagSize;
            do {
                protobuf_ptr += tagSize;
                auto str = repeatedString.Add();
                protobuf_ptr = InlineGreedyStringParser(str, protobuf_ptr, ctx);
                if (!ctx->DataAvailable(protobuf_ptr))
                {
                    break;
                }
            } while (ExpectTag(tag, protobuf_ptr));
        }
        else
        {
            auto str = std::string();
            protobuf_ptr = InlineGreedyStringParser(&str, protobuf_ptr, ctx);
            auto lv_ptr = _LVClusterHandle + fieldInfo.clusterOffset;
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
        switch (fieldInfo.wellKnownType)
        {
        case wellknown::Types::Double2DArray:
            return ParseDouble2DArrayMessage(fieldInfo, index, protobuf_ptr, ctx);
        case wellknown::Types::String2DArray:
            return ParseString2DArrayMessage(fieldInfo, index, protobuf_ptr, ctx);
        }

        auto metadata = fieldInfo._owner->FindMetadata(fieldInfo.embeddedMessageName);
        if (fieldInfo.isRepeated)
        {
            // if the array is not big enough, resize it to 2x the size
            auto numElements = 128;
            auto elementIndex = 0;
            auto clusterSize = metadata->clusterSize;
            auto arraySize = numElements * clusterSize;
            char _fillData = '\0';

            // Get the _repeatedMessageValues vector from the map
            auto repeatedMessageValuesIt = _repeatedMessageValuesMap.find(fieldInfo.fieldName);
            if (repeatedMessageValuesIt == _repeatedMessageValuesMap.end())
            {
                auto m_val = std::make_shared<RepeatedMessageValue>(fieldInfo, google::protobuf::RepeatedPtrField<google::protobuf::Message>());
                repeatedMessageValuesIt = _repeatedMessageValuesMap.emplace(fieldInfo.fieldName, m_val).first;
                repeatedMessageValuesIt->second.get()->_buffer.Reserve(numElements);
            }
            else
            {
                // Write from where we left off last time. We can't just assume all repeated elements on the wire
                // are contiguous in the buffer. The wire protocol allows for them to be interleaved with other fields.
                elementIndex = repeatedMessageValuesIt->second.get()->_numElements;
                // Recalculate number of cluster elements that can fit in the buffer based on its current capacity.
                numElements = repeatedMessageValuesIt->second.get()->_buffer.Capacity() / clusterSize;
            }

            auto tagSize = CalculateTagWireSize(tag);
            protobuf_ptr -= tagSize;
            do
            {
                protobuf_ptr += tagSize;

                // Resize the vector if we need more memory
                if (elementIndex >= numElements - 1)
                {
                    numElements *= 2;
                    arraySize = numElements * clusterSize;
                    repeatedMessageValuesIt->second.get()->_buffer.Reserve(numElements);
                }

                auto nestedMessageCluster = const_cast<int8_t*>(reinterpret_cast<const int8_t*>(repeatedMessageValuesIt->second.get()->_buffer.data()));
                nestedMessageCluster = nestedMessageCluster + (elementIndex * clusterSize);
                LVMessageEfficient nestedMessage(metadata, nestedMessageCluster);
                protobuf_ptr = ctx->ParseMessage(&nestedMessage, protobuf_ptr);

                elementIndex++;

                if (!ctx->DataAvailable(protobuf_ptr))
                {
                    break;
                }
            } while (ExpectTag(tag, protobuf_ptr));

            repeatedMessageValuesIt->second.get()->_numElements = elementIndex;
        }
        else
        {
            auto nestedClusterPtr = _LVClusterHandle + fieldInfo.clusterOffset;
            LVMessageEfficient nestedMessage(metadata, nestedClusterPtr);
            protobuf_ptr = ctx->ParseMessage(&nestedMessage, protobuf_ptr);
        }
        return protobuf_ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char* LVMessageEfficient::Parse2DArrayMessage(const MessageElementMetadata& fieldInfo, uint32_t index, const char* protobuf_ptr, ParseContext* ctx, wellknown::I2DArray& array)
    {
        auto metadata = fieldInfo._owner->FindMetadata(fieldInfo.embeddedMessageName);
        auto nestedMessage = std::make_shared<LVMessage>(metadata);
        protobuf_ptr = ctx->ParseMessage(nestedMessage.get(), protobuf_ptr);
        auto nestedClusterPtr = _LVClusterHandle + fieldInfo.clusterOffset;
        auto nestedMessageValue = std::make_shared<LVNestedMessageMessageValue>(index, nestedMessage);
        array.CopyFromMessageToCluster(fieldInfo, nestedMessageValue, nestedClusterPtr);
        return protobuf_ptr;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char* LVMessageEfficient::ParseDouble2DArrayMessage(const MessageElementMetadata& fieldInfo, uint32_t index, const char* protobuf_ptr, ParseContext* ctx)
    {
        return Parse2DArrayMessage(fieldInfo, index, protobuf_ptr, ctx, wellknown::Double2DArray::GetInstance());
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const char* LVMessageEfficient::ParseString2DArrayMessage(const MessageElementMetadata& fieldInfo, uint32_t index, const char* protobuf_ptr, ParseContext* ctx)
    {
        return Parse2DArrayMessage(fieldInfo, index, protobuf_ptr, ctx, wellknown::String2DArray::GetInstance());
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVMessageEfficient::PostInteralParseAction()
    {
        CopyOneofIndicesToCluster(_LVClusterHandle);

        for (auto nestedMessage : _repeatedMessageValuesMap)
        {
            auto& fieldInfo = nestedMessage.second.get()->_fieldInfo;
            auto& buffer = nestedMessage.second.get()->_buffer;
            auto numClusters = nestedMessage.second.get()->_numElements;

            auto metadata = fieldInfo._owner->FindMetadata(fieldInfo.embeddedMessageName);
            auto lv_ptr = _LVClusterHandle + fieldInfo.clusterOffset;
            auto clusterSize = metadata->clusterSize;
            auto alignment = metadata->alignmentRequirement;


            // Allocate an array with the correct size and alignment for the cluster.
            auto byteSize = numClusters * clusterSize;
            auto alignedElementSize = byteSize / alignment;
            if (byteSize % alignment != 0)
            {
                alignedElementSize++;
            }
            NumericArrayResize(GetTypeCodeForSize(alignment), 1, reinterpret_cast<void*>(lv_ptr), alignedElementSize);
            auto arrayHandle = *(LV1DArrayHandle*)lv_ptr;
            (*arrayHandle)->cnt = numClusters;

            auto vectorDataPtr = buffer.data();
            auto lvArrayDataPtr = (*arrayHandle)->bytes(0, alignment);
            memcpy(lvArrayDataPtr, vectorDataPtr, byteSize);
        }

        for (auto repeatedStringValue : _repeatedStringValuesMap)
        {
            auto& fieldInfo = repeatedStringValue.second.get()->_fieldInfo;
            auto& repeatedString = repeatedStringValue.second.get()->_repeatedString;
            auto lv_ptr = _LVClusterHandle + fieldInfo.clusterOffset;

            NumericArrayResize(GetTypeCodeForSize(sizeof(char*)), 1, reinterpret_cast<void*>(lv_ptr), repeatedString.size());
            auto arrayHandle = *(LV1DArrayHandle*)lv_ptr;
            (*arrayHandle)->cnt = repeatedString.size();

            // Copy the repeated string values into the LabVIEW array
            auto lvStringPtr = (*arrayHandle)->bytes<LStrHandle>();
            for (auto& str : repeatedString)
            {
                *lvStringPtr = nullptr;
                SetLVString(lvStringPtr, str);
                lvStringPtr++;
            }
        }
    }
}

