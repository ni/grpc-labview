//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_server.h>
#include <lv_message.h>
#include <message_value.h>
#include <string_utils.h>
#include <cstring>
#include <google/protobuf/wire_format_lite.h>

//---------------------------------------------------------------------
// Helpers for serialization using public CodedOutputStream and WireFormatLite APIs
//---------------------------------------------------------------------
namespace {
    using COS = google::protobuf::io::CodedOutputStream;
    using WFL = google::protobuf::internal::WireFormatLite;

    // Size of a length-delimited field (tag + varint length + payload)
    inline size_t LenDelimSize(int fieldNumber, size_t payloadLen) {
        return WFL::TagSize(fieldNumber, WFL::TYPE_MESSAGE)
               + COS::VarintSize32(static_cast<uint32_t>(payloadLen))
               + payloadLen;
    }
}

namespace grpc_labview
{
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    LVMessageValue::LVMessageValue(int protobufId) :
        _protobufId(protobufId)
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    LVNestedMessageMessageValue::LVNestedMessageMessageValue(int protobufId, std::shared_ptr<LVMessage> value) :
        LVMessageValue(protobufId),
        _value(value)
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    size_t LVNestedMessageMessageValue::ByteSizeLong()
    {
        _cachedNestedByteSize = _value->ByteSizeLong();
        return LenDelimSize(_protobufId, _cachedNestedByteSize);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVNestedMessageMessageValue::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        // Reuse size computed by ByteSizeLong() if available; fall back to recomputing.
        size_t nestedSize = (_cachedNestedByteSize != static_cast<size_t>(-1))
            ? _cachedNestedByteSize
            : _value->ByteSizeLong();
        WFL::WriteTag(_protobufId, WFL::WIRETYPE_LENGTH_DELIMITED, output);
        output->WriteVarint32(static_cast<uint32_t>(nestedSize));
        _value->SerializeToCodedStream(output);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    LVRepeatedNestedMessageMessageValue::LVRepeatedNestedMessageMessageValue(int protobufId) :
        LVMessageValue(protobufId)
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    size_t LVRepeatedNestedMessageMessageValue::ByteSizeLong()
    {
        _cachedNestedByteSizes.clear();
        _cachedNestedByteSizes.reserve(_value.size());
        size_t totalSize = 0;
        for (const auto& msg : _value)
        {
            size_t nestedSize = msg->ByteSizeLong();
            _cachedNestedByteSizes.push_back(nestedSize);
            totalSize += LenDelimSize(_protobufId, nestedSize);
        }
        return totalSize;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVRepeatedNestedMessageMessageValue::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        const bool haveCached = (_cachedNestedByteSizes.size() == _value.size());
        for (size_t i = 0; i < _value.size(); ++i)
        {
            size_t nestedSize = haveCached ? _cachedNestedByteSizes[i] : _value[i]->ByteSizeLong();
            WFL::WriteTag(_protobufId, WFL::WIRETYPE_LENGTH_DELIMITED, output);
            output->WriteVarint32(static_cast<uint32_t>(nestedSize));
            _value[i]->SerializeToCodedStream(output);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    LVStringMessageValue::LVStringMessageValue(int protobufId, std::string& value) :
        LVMessageValue(protobufId),
        _value(value)
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    size_t LVStringMessageValue::ByteSizeLong()
    {
        return WFL::TagSize(_protobufId, WFL::TYPE_STRING) + WFL::StringSize(_value);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVStringMessageValue::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        VerifyUtf8String(_value); // log only, no error
        WFL::WriteString(_protobufId, _value, output);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------

    LVRepeatedStringMessageValue::LVRepeatedStringMessageValue(int protobufId) :
        LVMessageValue(protobufId)
    {
    }

    size_t LVRepeatedStringMessageValue::ByteSizeLong()
    {
        size_t totalSize = 0;
        for (int i = 0, n = _value.size(); i < n; i++)
        {
            totalSize += WFL::TagSize(_protobufId, WFL::TYPE_STRING) + WFL::StringSize(_value.Get(i));
        }
        return totalSize;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------

    void LVRepeatedStringMessageValue::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        for (int i = 0, n = _value.size(); i < n; i++)
        {
            const auto& s = _value[i];
            VerifyUtf8String(s); // log only, no error
            WFL::WriteString(_protobufId, s, output);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    LVBytesMessageValue::LVBytesMessageValue(int protobufId, std::string& value) :
        LVMessageValue(protobufId),
        _value(value)
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    size_t LVBytesMessageValue::ByteSizeLong()
    {
        return WFL::TagSize(_protobufId, WFL::TYPE_BYTES) + WFL::BytesSize(_value);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVBytesMessageValue::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        WFL::WriteBytes(_protobufId, _value, output);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------

    LVRepeatedBytesMessageValue::LVRepeatedBytesMessageValue(int protobufId) :
        LVMessageValue(protobufId)
    {
    }

    size_t LVRepeatedBytesMessageValue::ByteSizeLong()
    {
        size_t totalSize = 0;
        for (int i = 0, n = _value.size(); i < n; i++)
        {
            totalSize += WFL::TagSize(_protobufId, WFL::TYPE_BYTES) + WFL::BytesSize(_value.Get(i));
        }
        return totalSize;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------

    void LVRepeatedBytesMessageValue::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        for (int i = 0, n = _value.size(); i < n; i++)
        {
            WFL::WriteBytes(_protobufId, _value[i], output);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    /*LVVariableMessageValue<bool>::LVVariableMessageValue(int protobufId, bool value) :
        LVMessageValue(protobufId),
        _value(value)
    {
    }*/

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    size_t LVVariableMessageValue<bool>::ByteSizeLong()
    {
        return WFL::TagSize(_protobufId, WFL::TYPE_BOOL) + WFL::kBoolSize;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    void LVVariableMessageValue<bool>::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        WFL::WriteBool(_protobufId, _value, output);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    /*LVRepeatedMessageValue<bool>::LVRepeatedMessageValue(int protobufId) :
        LVMessageValue(protobufId)
    {
    }*/

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    size_t LVRepeatedMessageValue<bool>::ByteSizeLong()
    {
        size_t dataSize = WFL::kBoolSize * static_cast<size_t>(_value.size());
        if (dataSize == 0) return 0;
        return LenDelimSize(_protobufId, dataSize);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    void LVRepeatedMessageValue<bool>::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        int count = _value.size();
        if (count == 0) return;
        size_t dataSize = WFL::kBoolSize * static_cast<size_t>(count);
        WFL::WriteTag(_protobufId, WFL::WIRETYPE_LENGTH_DELIMITED, output);
        output->WriteVarint32(static_cast<uint32_t>(dataSize));
        for (int i = 0; i < count; i++)
        {
            WFL::WriteBoolNoTag(_value.Get(i), output);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    /*LVVariableMessageValue<int>::LVVariableMessageValue(int protobufId, int value) :
        LVMessageValue(protobufId),
        _value(value)
    {
    }*/

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    size_t LVVariableMessageValue<int>::ByteSizeLong()
    {
        return WFL::TagSize(_protobufId, WFL::TYPE_INT32) + WFL::Int32Size(_value);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    void LVVariableMessageValue<int>::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        WFL::WriteInt32(_protobufId, _value, output);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    /*LVVariableMessageValue<uint32_t>::LVVariableMessageValue(int protobufId, uint32_t value) :
        LVMessageValue(protobufId),
        _value(value)
    {
    }*/

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    size_t LVVariableMessageValue<uint32_t>::ByteSizeLong()
    {
        return WFL::TagSize(_protobufId, WFL::TYPE_UINT32) + WFL::UInt32Size(_value);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    void LVVariableMessageValue<uint32_t>::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        WFL::WriteUInt32(_protobufId, _value, output);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    LVEnumMessageValue::LVEnumMessageValue(int protobufId, int value) :
        LVVariableMessageValue<int>(protobufId, value)
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    /*LVVariableMessageValue<int64_t>::LVVariableMessageValue(int protobufId, int64_t value) :
        LVMessageValue(protobufId),
        _value(value)
    {
    }*/

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    size_t LVVariableMessageValue<int64_t>::ByteSizeLong()
    {
        return WFL::TagSize(_protobufId, WFL::TYPE_INT64) + WFL::Int64Size(_value);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    void LVVariableMessageValue<int64_t>::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        WFL::WriteInt64(_protobufId, _value, output);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    /*LVVariableMessageValue<uint64_t>::LVVariableMessageValue(int protobufId, uint64_t value) :
        LVMessageValue(protobufId),
        _value(value)
    {
    }*/

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    size_t LVVariableMessageValue<uint64_t>::ByteSizeLong()
    {
        return WFL::TagSize(_protobufId, WFL::TYPE_UINT64) + WFL::UInt64Size(_value);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    void LVVariableMessageValue<uint64_t>::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        WFL::WriteUInt64(_protobufId, _value, output);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    /*LVRepeatedMessageValue<int>::LVRepeatedMessageValue(int protobufId) :
        LVMessageValue(protobufId)
    {
    }*/

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    // ByteSizeLong() always recomputes and stores the packed payload size in
    // _cachedDataSize, which Serialize() can then read directly without a second
    // pass over the elements.  The normal call sequence is ByteSizeLong() then
    // Serialize() (gRPC's serialization pipeline always calls them in that
    // order), so the cache will almost always be warm when Serialize() runs.
    // Serialize() includes a sentinel-check fallback for the rare case where it
    // is called without a preceding ByteSizeLong() — e.g. directly via
    // SerializeToCodedStream() in tests.  No explicit cache invalidation is
    // necessary: LVMessage::Clear() destroys the entire _values map (and
    // therefore these value objects) before a streaming message is reused, so
    // stale cache values can never be observed.
    template <>
    size_t LVRepeatedMessageValue<int>::ByteSizeLong()
    {
        size_t dataSize = WFL::Int32Size(_value);
        _cachedDataSize = dataSize;
        if (dataSize == 0) return 0;
        return LenDelimSize(_protobufId, dataSize);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    void LVRepeatedMessageValue<int>::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        if (_value.size() == 0) return;
        size_t dataSize = _cachedDataSize;
        if (dataSize == static_cast<size_t>(-1)) // fallback: ByteSizeLong() not called first
        {
            const_cast<LVRepeatedMessageValue<int>*>(this)->ByteSizeLong();
            dataSize = _cachedDataSize;
        }
        WFL::WriteTag(_protobufId, WFL::WIRETYPE_LENGTH_DELIMITED, output);
        output->WriteVarint32(static_cast<uint32_t>(dataSize));
        for (int i = 0, n = _value.size(); i < n; i++)
        {
            WFL::WriteInt32NoTag(_value.Get(i), output);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    /*LVRepeatedMessageValue<uint32_t>::LVRepeatedMessageValue(int protobufId) :
        LVMessageValue(protobufId)
    {
    }*/

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    size_t LVRepeatedMessageValue<uint32_t>::ByteSizeLong()
    {
        size_t dataSize = WFL::UInt32Size(_value);
        _cachedDataSize = dataSize;
        if (dataSize == 0) return 0;
        return LenDelimSize(_protobufId, dataSize);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    void LVRepeatedMessageValue<uint32_t>::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        if (_value.size() == 0) return;
        size_t dataSize = _cachedDataSize;
        if (dataSize == static_cast<size_t>(-1)) // fallback: ByteSizeLong() not called first
        {
            const_cast<LVRepeatedMessageValue<uint32_t>*>(this)->ByteSizeLong();
            dataSize = _cachedDataSize;
        }
        WFL::WriteTag(_protobufId, WFL::WIRETYPE_LENGTH_DELIMITED, output);
        output->WriteVarint32(static_cast<uint32_t>(dataSize));
        for (int i = 0, n = _value.size(); i < n; i++)
        {
            WFL::WriteUInt32NoTag(_value.Get(i), output);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    LVRepeatedEnumMessageValue::LVRepeatedEnumMessageValue(int protobufId) :
        LVRepeatedMessageValue<int>(protobufId)
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    /*LVRepeatedMessageValue<int64_t>::LVRepeatedMessageValue(int protobufId) :
        LVMessageValue(protobufId)
    {
    }*/

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    size_t LVRepeatedMessageValue<int64_t>::ByteSizeLong()
    {
        size_t dataSize = WFL::Int64Size(_value);
        _cachedDataSize = dataSize;
        if (dataSize == 0) return 0;
        return LenDelimSize(_protobufId, dataSize);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    void LVRepeatedMessageValue<int64_t>::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        if (_value.size() == 0) return;
        size_t dataSize = _cachedDataSize;
        if (dataSize == static_cast<size_t>(-1)) // fallback: ByteSizeLong() not called first
        {
            const_cast<LVRepeatedMessageValue<int64_t>*>(this)->ByteSizeLong();
            dataSize = _cachedDataSize;
        }
        WFL::WriteTag(_protobufId, WFL::WIRETYPE_LENGTH_DELIMITED, output);
        output->WriteVarint32(static_cast<uint32_t>(dataSize));
        for (int i = 0, n = _value.size(); i < n; i++)
        {
            WFL::WriteInt64NoTag(_value.Get(i), output);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    /*LVRepeatedMessageValue<uint64_t>::LVRepeatedMessageValue(int protobufId) :
        LVMessageValue(protobufId)
    {
    }*/

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    size_t LVRepeatedMessageValue<uint64_t>::ByteSizeLong()
    {
        size_t dataSize = WFL::UInt64Size(_value);
        _cachedDataSize = dataSize;
        if (dataSize == 0) return 0;
        return LenDelimSize(_protobufId, dataSize);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    void LVRepeatedMessageValue<uint64_t>::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        if (_value.size() == 0) return;
        size_t dataSize = _cachedDataSize;
        if (dataSize == static_cast<size_t>(-1)) // fallback: ByteSizeLong() not called first
        {
            const_cast<LVRepeatedMessageValue<uint64_t>*>(this)->ByteSizeLong();
            dataSize = _cachedDataSize;
        }
        WFL::WriteTag(_protobufId, WFL::WIRETYPE_LENGTH_DELIMITED, output);
        output->WriteVarint32(static_cast<uint32_t>(dataSize));
        for (int i = 0, n = _value.size(); i < n; i++)
        {
            WFL::WriteUInt64NoTag(_value.Get(i), output);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    /*LVVariableMessageValue<float>::LVVariableMessageValue(int protobufId, float value) :
        LVMessageValue(protobufId),
        _value(value)
    {
    }*/

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    size_t LVVariableMessageValue<float>::ByteSizeLong()
    {
        return WFL::TagSize(_protobufId, WFL::TYPE_FLOAT) + WFL::kFloatSize;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    void LVVariableMessageValue<float>::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        WFL::WriteFloat(_protobufId, _value, output);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    /*LVRepeatedMessageValue<float>::LVRepeatedMessageValue(int protobufId) :
        LVMessageValue(protobufId)
    {
    }*/

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    size_t LVRepeatedMessageValue<float>::ByteSizeLong()
    {
        size_t dataSize = WFL::kFloatSize * static_cast<size_t>(_value.size());
        if (dataSize == 0) return 0;
        return LenDelimSize(_protobufId, dataSize);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    void LVRepeatedMessageValue<float>::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        int count = _value.size();
        if (count == 0) return;
        size_t dataSize = WFL::kFloatSize * static_cast<size_t>(count);
        WFL::WriteTag(_protobufId, WFL::WIRETYPE_LENGTH_DELIMITED, output);
        output->WriteVarint32(static_cast<uint32_t>(dataSize));
        for (int i = 0; i < count; i++)
        {
            WFL::WriteFloatNoTag(_value.Get(i), output);
        }
    }


    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    /*LVVariableMessageValue<double>::LVVariableMessageValue(int protobufId, double value) :
        LVMessageValue(protobufId),
        _value(value)
    {
    }*/

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    size_t LVVariableMessageValue<double>::ByteSizeLong()
    {
        return WFL::TagSize(_protobufId, WFL::TYPE_DOUBLE) + WFL::kDoubleSize;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    void LVVariableMessageValue<double>::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        WFL::WriteDouble(_protobufId, _value, output);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    /*LVRepeatedMessageValue<double>::LVRepeatedMessageValue(int protobufId) :
        LVMessageValue(protobufId)
    {
    }*/

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    size_t LVRepeatedMessageValue<double>::ByteSizeLong()
    {
        size_t dataSize = WFL::kDoubleSize * static_cast<size_t>(_value.size());
        if (dataSize == 0) return 0;
        return LenDelimSize(_protobufId, dataSize);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    void LVRepeatedMessageValue<double>::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        int count = _value.size();
        if (count == 0) return;
        size_t dataSize = WFL::kDoubleSize * static_cast<size_t>(count);
        WFL::WriteTag(_protobufId, WFL::WIRETYPE_LENGTH_DELIMITED, output);
        output->WriteVarint32(static_cast<uint32_t>(dataSize));
        for (int i = 0; i < count; i++)
        {
            WFL::WriteDoubleNoTag(_value.Get(i), output);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    LVSInt32MessageValue::LVSInt32MessageValue(int protobufId, int32_t value) :
        LVMessageValue(protobufId),
        _value(value)
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    size_t LVSInt32MessageValue::ByteSizeLong()
    {
        return WFL::TagSize(_protobufId, WFL::TYPE_SINT32) + WFL::SInt32Size(_value);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVSInt32MessageValue::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        WFL::WriteSInt32(_protobufId, _value, output);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    LVRepeatedSInt32MessageValue::LVRepeatedSInt32MessageValue(int protobufId) :
        LVMessageValue(protobufId)
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    size_t LVRepeatedSInt32MessageValue::ByteSizeLong()
    {
        size_t dataSize = WFL::SInt32Size(_value);
        _cachedDataSize = dataSize;
        if (dataSize == 0) return 0;
        return LenDelimSize(_protobufId, dataSize);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVRepeatedSInt32MessageValue::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        if (_value.size() == 0) return;
        size_t dataSize = _cachedDataSize;
        if (dataSize == static_cast<size_t>(-1)) // fallback: ByteSizeLong() not called first
        {
            const_cast<LVRepeatedSInt32MessageValue*>(this)->ByteSizeLong();
            dataSize = _cachedDataSize;
        }
        WFL::WriteTag(_protobufId, WFL::WIRETYPE_LENGTH_DELIMITED, output);
        output->WriteVarint32(static_cast<uint32_t>(dataSize));
        for (int i = 0, n = _value.size(); i < n; i++)
        {
            WFL::WriteSInt32NoTag(_value.Get(i), output);
        }
    }
    //---------------------------------------------------------------------
     //---------------------------------------------------------------------
    LVSInt64MessageValue::LVSInt64MessageValue(int protobufId, int64_t value) :
        LVMessageValue(protobufId),
        _value(value)
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    size_t LVSInt64MessageValue::ByteSizeLong()
    {
        return WFL::TagSize(_protobufId, WFL::TYPE_SINT64) + WFL::SInt64Size(_value);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVSInt64MessageValue::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        WFL::WriteSInt64(_protobufId, _value, output);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    LVRepeatedSInt64MessageValue::LVRepeatedSInt64MessageValue(int protobufId) :
        LVMessageValue(protobufId)
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    size_t LVRepeatedSInt64MessageValue::ByteSizeLong()
    {
        size_t dataSize = WFL::SInt64Size(_value);
        _cachedDataSize = dataSize;
        if (dataSize == 0) return 0;
        return LenDelimSize(_protobufId, dataSize);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVRepeatedSInt64MessageValue::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        if (_value.size() == 0) return;
        size_t dataSize = _cachedDataSize;
        if (dataSize == static_cast<size_t>(-1)) // fallback: ByteSizeLong() not called first
        {
            const_cast<LVRepeatedSInt64MessageValue*>(this)->ByteSizeLong();
            dataSize = _cachedDataSize;
        }
        WFL::WriteTag(_protobufId, WFL::WIRETYPE_LENGTH_DELIMITED, output);
        output->WriteVarint32(static_cast<uint32_t>(dataSize));
        for (int i = 0, n = _value.size(); i < n; i++)
        {
            WFL::WriteSInt64NoTag(_value.Get(i), output);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    LVFixed32MessageValue::LVFixed32MessageValue(int protobufId, uint32_t value) :
        LVMessageValue(protobufId),
        _value(value)
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    size_t LVFixed32MessageValue::ByteSizeLong()
    {
        return WFL::TagSize(_protobufId, WFL::TYPE_FIXED32) + WFL::kFixed32Size;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVFixed32MessageValue::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        WFL::WriteFixed32(_protobufId, _value, output);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    LVRepeatedFixed32MessageValue::LVRepeatedFixed32MessageValue(int protobufId) :
        LVMessageValue(protobufId)
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    size_t LVRepeatedFixed32MessageValue::ByteSizeLong()
    {
        size_t dataSize = WFL::kFixed32Size * static_cast<size_t>(_value.size());
        if (dataSize == 0) return 0;
        return LenDelimSize(_protobufId, dataSize);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVRepeatedFixed32MessageValue::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        int count = _value.size();
        if (count == 0) return;
        size_t dataSize = WFL::kFixed32Size * static_cast<size_t>(count);
        WFL::WriteTag(_protobufId, WFL::WIRETYPE_LENGTH_DELIMITED, output);
        output->WriteVarint32(static_cast<uint32_t>(dataSize));
        for (int i = 0; i < count; i++)
        {
            WFL::WriteFixed32NoTag(_value.Get(i), output);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    LVFixed64MessageValue::LVFixed64MessageValue(int protobufId, uint64_t value) :
        LVMessageValue(protobufId),
        _value(value)
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    size_t LVFixed64MessageValue::ByteSizeLong()
    {
        return WFL::TagSize(_protobufId, WFL::TYPE_FIXED64) + WFL::kFixed64Size;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVFixed64MessageValue::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        WFL::WriteFixed64(_protobufId, _value, output);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    LVRepeatedFixed64MessageValue::LVRepeatedFixed64MessageValue(int protobufId) :
        LVMessageValue(protobufId)
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    size_t LVRepeatedFixed64MessageValue::ByteSizeLong()
    {
        size_t dataSize = WFL::kFixed64Size * static_cast<size_t>(_value.size());
        if (dataSize == 0) return 0;
        return LenDelimSize(_protobufId, dataSize);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVRepeatedFixed64MessageValue::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        int count = _value.size();
        if (count == 0) return;
        size_t dataSize = WFL::kFixed64Size * static_cast<size_t>(count);
        WFL::WriteTag(_protobufId, WFL::WIRETYPE_LENGTH_DELIMITED, output);
        output->WriteVarint32(static_cast<uint32_t>(dataSize));
        for (int i = 0; i < count; i++)
        {
            WFL::WriteFixed64NoTag(_value.Get(i), output);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    LVSFixed32MessageValue::LVSFixed32MessageValue(int protobufId, int32_t value) :
        LVMessageValue(protobufId),
        _value(value)
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    size_t LVSFixed32MessageValue::ByteSizeLong()
    {
        return WFL::TagSize(_protobufId, WFL::TYPE_SFIXED32) + WFL::kSFixed32Size;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVSFixed32MessageValue::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        WFL::WriteSFixed32(_protobufId, _value, output);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    LVRepeatedSFixed32MessageValue::LVRepeatedSFixed32MessageValue(int protobufId) :
        LVMessageValue(protobufId)
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    size_t LVRepeatedSFixed32MessageValue::ByteSizeLong()
    {
        size_t dataSize = WFL::kSFixed32Size * static_cast<size_t>(_value.size());
        if (dataSize == 0) return 0;
        return LenDelimSize(_protobufId, dataSize);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVRepeatedSFixed32MessageValue::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        int count = _value.size();
        if (count == 0) return;
        size_t dataSize = WFL::kSFixed32Size * static_cast<size_t>(count);
        WFL::WriteTag(_protobufId, WFL::WIRETYPE_LENGTH_DELIMITED, output);
        output->WriteVarint32(static_cast<uint32_t>(dataSize));
        for (int i = 0; i < count; i++)
        {
            WFL::WriteSFixed32NoTag(_value.Get(i), output);
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    LVSFixed64MessageValue::LVSFixed64MessageValue(int protobufId, int64_t value) :
        LVMessageValue(protobufId),
        _value(value)
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    size_t LVSFixed64MessageValue::ByteSizeLong()
    {
        return WFL::TagSize(_protobufId, WFL::TYPE_SFIXED64) + WFL::kSFixed64Size;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVSFixed64MessageValue::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        WFL::WriteSFixed64(_protobufId, _value, output);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    LVRepeatedSFixed64MessageValue::LVRepeatedSFixed64MessageValue(int protobufId) :
        LVMessageValue(protobufId)
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    size_t LVRepeatedSFixed64MessageValue::ByteSizeLong()
    {
        size_t dataSize = WFL::kSFixed64Size * static_cast<size_t>(_value.size());
        if (dataSize == 0) return 0;
        return LenDelimSize(_protobufId, dataSize);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVRepeatedSFixed64MessageValue::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        int count = _value.size();
        if (count == 0) return;
        size_t dataSize = WFL::kSFixed64Size * static_cast<size_t>(count);
        WFL::WriteTag(_protobufId, WFL::WIRETYPE_LENGTH_DELIMITED, output);
        output->WriteVarint32(static_cast<uint32_t>(dataSize));
        for (int i = 0; i < count; i++)
        {
            WFL::WriteSFixed64NoTag(_value.Get(i), output);
        }
    }
}
