//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_server.h>
#include <lv_message_efficient.h>
#include <well_known_messages.h>
#include <sstream>
#include <feature_toggles.h>
#include <string_utils.h>
#include <google/protobuf/wire_format_lite.h>

namespace {
    inline int32_t ZigZagDecode32(uint32_t n) { return google::protobuf::internal::WireFormatLite::ZigZagDecode32(n); }
    inline int64_t ZigZagDecode64(uint64_t n) { return google::protobuf::internal::WireFormatLite::ZigZagDecode64(n); }
}

namespace grpc_labview
{
    template<typename T>
    static void CopyVectorToLVArray(const std::vector<T>& vals, int8_t* lv_ptr)
    {
        auto cnt = vals.size();
        if (cnt > 0)
        {
            NumericArrayResize(GetTypeCodeForSize(sizeof(T)), 1, reinterpret_cast<void*>(lv_ptr), cnt);
            auto arr = *(LV1DArrayHandle*)lv_ptr;
            (*arr)->cnt = static_cast<int32_t>(cnt);
            std::memcpy((*arr)->bytes<T>(), vals.data(), cnt * sizeof(T));
        }
    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseInt32Field(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo)
    {
        auto lv_ptr = _LVClusterHandle + fieldInfo.clusterOffset;
        if (fieldInfo.isRepeated)
        {
            uint32_t length; if (!input->ReadVarint32(&length)) return false;
            auto limit = input->PushLimit(static_cast<int>(length));
            std::vector<int32_t> vals;
            while (input->BytesUntilLimit() > 0) {
                uint32_t raw; if (!input->ReadVarint32(&raw)) { input->PopLimit(limit); return false; }
                vals.push_back(static_cast<int32_t>(raw));
            }
            input->PopLimit(limit);
            CopyVectorToLVArray(vals, lv_ptr);
        }
        else
        {
            uint32_t raw; if (!input->ReadVarint32(&raw)) return false;
            *reinterpret_cast<int32_t*>(lv_ptr) = static_cast<int32_t>(raw);
        }
        return true;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseUInt32Field(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo)
    {
        auto lv_ptr = _LVClusterHandle + fieldInfo.clusterOffset;
        if (fieldInfo.isRepeated)
        {
            uint32_t length; if (!input->ReadVarint32(&length)) return false;
            auto limit = input->PushLimit(static_cast<int>(length));
            std::vector<uint32_t> vals;
            while (input->BytesUntilLimit() > 0) {
                uint32_t v; if (!input->ReadVarint32(&v)) { input->PopLimit(limit); return false; }
                vals.push_back(v);
            }
            input->PopLimit(limit);
            CopyVectorToLVArray(vals, lv_ptr);
        }
        else
        {
            if (!input->ReadVarint32(reinterpret_cast<uint32_t*>(lv_ptr))) return false;
        }
        return true;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseInt64Field(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo)
    {
        auto lv_ptr = _LVClusterHandle + fieldInfo.clusterOffset;
        if (fieldInfo.isRepeated)
        {
            uint32_t length; if (!input->ReadVarint32(&length)) return false;
            auto limit = input->PushLimit(static_cast<int>(length));
            std::vector<int64_t> vals;
            while (input->BytesUntilLimit() > 0) {
                uint64_t raw; if (!input->ReadVarint64(&raw)) { input->PopLimit(limit); return false; }
                vals.push_back(static_cast<int64_t>(raw));
            }
            input->PopLimit(limit);
            CopyVectorToLVArray(vals, lv_ptr);
        }
        else
        {
            uint64_t raw; if (!input->ReadVarint64(&raw)) return false;
            *reinterpret_cast<int64_t*>(lv_ptr) = static_cast<int64_t>(raw);
        }
        return true;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseUInt64Field(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo)
    {
        auto lv_ptr = _LVClusterHandle + fieldInfo.clusterOffset;
        if (fieldInfo.isRepeated)
        {
            uint32_t length; if (!input->ReadVarint32(&length)) return false;
            auto limit = input->PushLimit(static_cast<int>(length));
            std::vector<uint64_t> vals;
            while (input->BytesUntilLimit() > 0) {
                uint64_t v; if (!input->ReadVarint64(&v)) { input->PopLimit(limit); return false; }
                vals.push_back(v);
            }
            input->PopLimit(limit);
            CopyVectorToLVArray(vals, lv_ptr);
        }
        else
        {
            if (!input->ReadVarint64(reinterpret_cast<uint64_t*>(lv_ptr))) return false;
        }
        return true;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseBoolField(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo)
    {
        auto lv_ptr = _LVClusterHandle + fieldInfo.clusterOffset;
        if (fieldInfo.isRepeated)
        {
            uint32_t length; if (!input->ReadVarint32(&length)) return false;
            auto limit = input->PushLimit(static_cast<int>(length));
            std::vector<bool> vals;
            while (input->BytesUntilLimit() > 0) {
                uint64_t raw; if (!input->ReadVarint64(&raw)) { input->PopLimit(limit); return false; }
                vals.push_back(raw != 0);
            }
            input->PopLimit(limit);
            auto cnt = vals.size();
            if (cnt > 0) {
                NumericArrayResize(GetTypeCodeForSize(sizeof(bool)), 1, reinterpret_cast<void*>(lv_ptr), cnt);
                auto arr = *(LV1DArrayHandle*)lv_ptr;
                (*arr)->cnt = static_cast<int32_t>(cnt);
                for (size_t i = 0; i < cnt; ++i) {
                    (*arr)->bytes<bool>()[i] = vals[i];
                }
            }
        }
        else
        {
            uint64_t raw; if (!input->ReadVarint64(&raw)) return false;
            *reinterpret_cast<bool*>(lv_ptr) = (raw != 0);
        }
        return true;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseFloatField(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo)
    {
        auto lv_ptr = _LVClusterHandle + fieldInfo.clusterOffset;
        if (fieldInfo.isRepeated)
        {
            uint32_t length; if (!input->ReadVarint32(&length)) return false;
            auto limit = input->PushLimit(static_cast<int>(length));
            std::vector<float> vals;
            while (input->BytesUntilLimit() > 0) {
                uint32_t raw; if (!input->ReadLittleEndian32(&raw)) { input->PopLimit(limit); return false; }
                float f; memcpy(&f, &raw, sizeof(float));
                vals.push_back(f);
            }
            input->PopLimit(limit);
            CopyVectorToLVArray(vals, lv_ptr);
        }
        else
        {
            uint32_t raw; if (!input->ReadLittleEndian32(&raw)) return false;
            memcpy(lv_ptr, &raw, sizeof(float));
        }
        return true;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseDoubleField(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo)
    {
        auto lv_ptr = _LVClusterHandle + fieldInfo.clusterOffset;
        if (fieldInfo.isRepeated)
        {
            uint32_t length; if (!input->ReadVarint32(&length)) return false;
            auto limit = input->PushLimit(static_cast<int>(length));
            std::vector<double> vals;
            while (input->BytesUntilLimit() > 0) {
                uint64_t raw; if (!input->ReadLittleEndian64(&raw)) { input->PopLimit(limit); return false; }
                double d; memcpy(&d, &raw, sizeof(double));
                vals.push_back(d);
            }
            input->PopLimit(limit);
            CopyVectorToLVArray(vals, lv_ptr);
        }
        else
        {
            uint64_t raw; if (!input->ReadLittleEndian64(&raw)) return false;
            memcpy(lv_ptr, &raw, sizeof(double));
        }
        return true;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseSInt32Field(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo)
    {
        auto lv_ptr = _LVClusterHandle + fieldInfo.clusterOffset;
        if (fieldInfo.isRepeated)
        {
            uint32_t length; if (!input->ReadVarint32(&length)) return false;
            auto limit = input->PushLimit(static_cast<int>(length));
            std::vector<int32_t> vals;
            while (input->BytesUntilLimit() > 0) {
                uint32_t raw; if (!input->ReadVarint32(&raw)) { input->PopLimit(limit); return false; }
                vals.push_back(ZigZagDecode32(raw));
            }
            input->PopLimit(limit);
            CopyVectorToLVArray(vals, lv_ptr);
        }
        else
        {
            uint32_t raw; if (!input->ReadVarint32(&raw)) return false;
            *reinterpret_cast<int32_t*>(lv_ptr) = ZigZagDecode32(raw);
        }
        return true;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseSInt64Field(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo)
    {
        auto lv_ptr = _LVClusterHandle + fieldInfo.clusterOffset;
        if (fieldInfo.isRepeated)
        {
            uint32_t length; if (!input->ReadVarint32(&length)) return false;
            auto limit = input->PushLimit(static_cast<int>(length));
            std::vector<int64_t> vals;
            while (input->BytesUntilLimit() > 0) {
                uint64_t raw; if (!input->ReadVarint64(&raw)) { input->PopLimit(limit); return false; }
                vals.push_back(ZigZagDecode64(raw));
            }
            input->PopLimit(limit);
            CopyVectorToLVArray(vals, lv_ptr);
        }
        else
        {
            uint64_t raw; if (!input->ReadVarint64(&raw)) return false;
            *reinterpret_cast<int64_t*>(lv_ptr) = ZigZagDecode64(raw);
        }
        return true;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseFixed32Field(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo)
    {
        auto lv_ptr = _LVClusterHandle + fieldInfo.clusterOffset;
        if (fieldInfo.isRepeated)
        {
            uint32_t length; if (!input->ReadVarint32(&length)) return false;
            auto limit = input->PushLimit(static_cast<int>(length));
            std::vector<uint32_t> vals;
            while (input->BytesUntilLimit() > 0) {
                uint32_t v; if (!input->ReadLittleEndian32(&v)) { input->PopLimit(limit); return false; }
                vals.push_back(v);
            }
            input->PopLimit(limit);
            CopyVectorToLVArray(vals, lv_ptr);
        }
        else
        {
            if (!input->ReadLittleEndian32(reinterpret_cast<uint32_t*>(lv_ptr))) return false;
        }
        return true;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseFixed64Field(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo)
    {
        auto lv_ptr = _LVClusterHandle + fieldInfo.clusterOffset;
        if (fieldInfo.isRepeated)
        {
            uint32_t length; if (!input->ReadVarint32(&length)) return false;
            auto limit = input->PushLimit(static_cast<int>(length));
            std::vector<uint64_t> vals;
            while (input->BytesUntilLimit() > 0) {
                uint64_t v; if (!input->ReadLittleEndian64(&v)) { input->PopLimit(limit); return false; }
                vals.push_back(v);
            }
            input->PopLimit(limit);
            CopyVectorToLVArray(vals, lv_ptr);
        }
        else
        {
            if (!input->ReadLittleEndian64(reinterpret_cast<uint64_t*>(lv_ptr))) return false;
        }
        return true;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseSFixed32Field(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo)
    {
        auto lv_ptr = _LVClusterHandle + fieldInfo.clusterOffset;
        if (fieldInfo.isRepeated)
        {
            uint32_t length; if (!input->ReadVarint32(&length)) return false;
            auto limit = input->PushLimit(static_cast<int>(length));
            std::vector<int32_t> vals;
            while (input->BytesUntilLimit() > 0) {
                uint32_t raw; if (!input->ReadLittleEndian32(&raw)) { input->PopLimit(limit); return false; }
                vals.push_back(static_cast<int32_t>(raw));
            }
            input->PopLimit(limit);
            CopyVectorToLVArray(vals, lv_ptr);
        }
        else
        {
            uint32_t raw; if (!input->ReadLittleEndian32(&raw)) return false;
            *reinterpret_cast<int32_t*>(lv_ptr) = static_cast<int32_t>(raw);
        }
        return true;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseSFixed64Field(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo)
    {
        auto lv_ptr = _LVClusterHandle + fieldInfo.clusterOffset;
        if (fieldInfo.isRepeated)
        {
            uint32_t length; if (!input->ReadVarint32(&length)) return false;
            auto limit = input->PushLimit(static_cast<int>(length));
            std::vector<int64_t> vals;
            while (input->BytesUntilLimit() > 0) {
                uint64_t raw; if (!input->ReadLittleEndian64(&raw)) { input->PopLimit(limit); return false; }
                vals.push_back(static_cast<int64_t>(raw));
            }
            input->PopLimit(limit);
            CopyVectorToLVArray(vals, lv_ptr);
        }
        else
        {
            uint64_t raw; if (!input->ReadLittleEndian64(&raw)) return false;
            *reinterpret_cast<int64_t*>(lv_ptr) = static_cast<int64_t>(raw);
        }
        return true;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseEnumField(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo)
    {
        std::shared_ptr<EnumMetadata> enumMetadata = fieldInfo._owner->FindEnumMetadata(fieldInfo.embeddedMessageName);
        auto lv_ptr = _LVClusterHandle + fieldInfo.clusterOffset;

        if (fieldInfo.isRepeated)
        {
            uint32_t length; if (!input->ReadVarint32(&length)) return false;
            auto limit = input->PushLimit(static_cast<int>(length));
            std::vector<int32_t> vals;
            while (input->BytesUntilLimit() > 0)
            {
                uint32_t raw; if (!input->ReadVarint32(&raw)) { input->PopLimit(limit); return false; }
                vals.push_back(enumMetadata->GetLVEnumValueFromProtoValue(static_cast<int32_t>(raw)));
            }
            input->PopLimit(limit);
            auto cnt = vals.size();
            if (cnt > 0)
            {
                NumericArrayResize(GetTypeCodeForSize(sizeof(int32_t)), 1, reinterpret_cast<void*>(lv_ptr), cnt);
                auto arr = *(LV1DArrayHandle*)lv_ptr;
                (*arr)->cnt = static_cast<int32_t>(cnt);
                memcpy((*arr)->bytes<int32_t>(), vals.data(), cnt * sizeof(int32_t));
            }
        }
        else
        {
            uint32_t raw; if (!input->ReadVarint32(&raw)) return false;
            *reinterpret_cast<int32_t*>(lv_ptr) = enumMetadata->GetLVEnumValueFromProtoValue(static_cast<int32_t>(raw));
        }
        return true;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseStringField(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo)
    {
        uint32_t length;
        if (!input->ReadVarint32(&length)) return false;
        std::string str;
        if (!input->ReadString(&str, static_cast<int>(length))) return false;

        if (!VerifyUtf8String(str, fieldInfo.fieldName.c_str())) {
            throw std::runtime_error("String contains invalid UTF-8 data.");
        }

        if (fieldInfo.isRepeated)
        {
            auto it = _repeatedStringValuesMap.find(fieldInfo.fieldName);
            if (it == _repeatedStringValuesMap.end())
            {
                auto m_val = std::make_shared<RepeatedStringValue>(fieldInfo);
                it = _repeatedStringValuesMap.emplace(fieldInfo.fieldName, m_val).first;
            }
            *it->second->_repeatedString.Add() = str;
        }
        else
        {
            auto lv_ptr = _LVClusterHandle + fieldInfo.clusterOffset;
            SetLVString((LStrHandle*)lv_ptr, str);
        }
        return true;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseBytesField(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo)
    {
        uint32_t length;
        if (!input->ReadVarint32(&length)) return false;
        std::string bytes;
        if (!input->ReadString(&bytes, static_cast<int>(length))) return false;

        if (!FeatureConfig::getInstance().AreUtf8StringsEnabled())
        {
            // Treat bytes as string
            if (fieldInfo.isRepeated)
            {
                auto it = _repeatedStringValuesMap.find(fieldInfo.fieldName);
                if (it == _repeatedStringValuesMap.end())
                {
                    auto m_val = std::make_shared<RepeatedStringValue>(fieldInfo);
                    it = _repeatedStringValuesMap.emplace(fieldInfo.fieldName, m_val).first;
                }
                *it->second->_repeatedString.Add() = bytes;
            }
            else
            {
                auto lv_ptr = _LVClusterHandle + fieldInfo.clusterOffset;
                SetLVString((LStrHandle*)lv_ptr, bytes);
            }
        }
        else
        {
            if (fieldInfo.isRepeated)
            {
                auto it = _repeatedBytesValuesMap.find(fieldInfo.fieldName);
                if (it == _repeatedBytesValuesMap.end())
                {
                    auto m_val = std::make_shared<RepeatedBytesValue>(fieldInfo);
                    it = _repeatedBytesValuesMap.emplace(fieldInfo.fieldName, m_val).first;
                }
                *it->second->_repeatedBytes.Add() = bytes;
            }
            else
            {
                auto lv_ptr = _LVClusterHandle + fieldInfo.clusterOffset;
                SetLVBytes((LStrHandle*)lv_ptr, bytes);
            }
        }
        return true;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseMessageField(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo)
    {
        switch (fieldInfo.wellKnownType)
        {
        case wellknown::Types::Double2DArray:
            return ParseDouble2DArrayField(input, field_number, fieldInfo);
        case wellknown::Types::String2DArray:
            return ParseString2DArrayField(input, field_number, fieldInfo);
        }

        uint32_t length;
        if (!input->ReadVarint32(&length)) return false;
        auto limit = input->PushLimit(static_cast<int>(length));

        auto metadata = fieldInfo._owner->FindMetadata(fieldInfo.embeddedMessageName);
        if (fieldInfo.isRepeated)
        {
            auto it = _repeatedMessageValuesMap.find(fieldInfo.fieldName);
            if (it == _repeatedMessageValuesMap.end())
            {
                auto m_val = std::make_shared<RepeatedMessageValue>(fieldInfo, google::protobuf::RepeatedPtrField<google::protobuf::Message>());
                it = _repeatedMessageValuesMap.emplace(fieldInfo.fieldName, m_val).first;
                it->second->_buffer.Reserve(128);
            }

            auto& repeatedValue = *it->second;
            auto elementIndex = repeatedValue._numElements;
            auto clusterSize = metadata->clusterSize;
            auto numElements = static_cast<uint64_t>(repeatedValue._buffer.Capacity()) / clusterSize;
            if (numElements == 0)
            {
                numElements = 128;
                repeatedValue._buffer.Reserve(static_cast<int>(numElements));
            }

            if (elementIndex >= numElements - 1)
            {
                numElements = std::max((uint64_t)128, numElements * 2);
                repeatedValue._buffer.Reserve(static_cast<int>(numElements));
            }

            auto nestedCluster = const_cast<int8_t*>(reinterpret_cast<const int8_t*>(repeatedValue._buffer.data()));
            nestedCluster += elementIndex * clusterSize;
            LVMessageEfficient nestedMessage(metadata, nestedCluster);
            if (!nestedMessage.ParseFromCodedStream(input))
            {
                input->PopLimit(limit);
                return false;
            }
            repeatedValue._numElements = elementIndex + 1;
        }
        else
        {
            auto nestedClusterPtr = _LVClusterHandle + fieldInfo.clusterOffset;
            LVMessageEfficient nestedMessage(metadata, nestedClusterPtr);
            if (!nestedMessage.ParseFromCodedStream(input))
            {
                input->PopLimit(limit);
                return false;
            }
        }

        input->PopLimit(limit);
        return true;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::Parse2DArrayField(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo, wellknown::I2DArray& array)
    {
        uint32_t length;
        if (!input->ReadVarint32(&length)) return false;
        auto limit = input->PushLimit(static_cast<int>(length));

        auto metadata = fieldInfo._owner->FindMetadata(fieldInfo.embeddedMessageName);
        auto nestedMessage = std::make_shared<LVMessage>(metadata);
        if (!nestedMessage->ParseFromCodedStream(input))
        {
            input->PopLimit(limit);
            return false;
        }
        input->PopLimit(limit);

        auto nestedClusterPtr = _LVClusterHandle + fieldInfo.clusterOffset;
        auto nestedMessageValue = std::make_shared<LVNestedMessageMessageValue>(field_number, nestedMessage);
        array.CopyFromMessageToCluster(fieldInfo, nestedMessageValue, nestedClusterPtr);
        return true;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseDouble2DArrayField(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo)
    {
        return Parse2DArrayField(input, field_number, fieldInfo, wellknown::Double2DArray::GetInstance());
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    bool LVMessageEfficient::ParseString2DArrayField(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo)
    {
        return Parse2DArrayField(input, field_number, fieldInfo, wellknown::String2DArray::GetInstance());
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

        if (FeatureConfig::getInstance().AreUtf8StringsEnabled()) {
            for (auto repeatedBytesValue : _repeatedBytesValuesMap)
            {
                auto& fieldInfo = repeatedBytesValue.second.get()->_fieldInfo;
                auto& repeatedBytes = repeatedBytesValue.second.get()->_repeatedBytes;
                auto lv_ptr = _LVClusterHandle + fieldInfo.clusterOffset;

                NumericArrayResize(GetTypeCodeForSize(sizeof(char*)), 1, reinterpret_cast<void*>(lv_ptr), repeatedBytes.size());
                auto arrayHandle = *(LV1DArrayHandle*)lv_ptr;
                (*arrayHandle)->cnt = repeatedBytes.size();

                // Copy the repeated bytes values into the LabVIEW array
                auto lvBytesPtr = (*arrayHandle)->bytes<LStrHandle>();
                for (auto& bytes : repeatedBytes)
                {
                    *lvBytesPtr = nullptr;
                    SetLVBytes(lvBytesPtr, bytes);
                    lvBytesPtr++;
                }
            }
        }
    }
}

