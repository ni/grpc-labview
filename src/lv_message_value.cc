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

    constexpr int WT_VARINT  = 0;
    constexpr int WT_FIXED64 = 1;
    constexpr int WT_LEN     = 2;
    constexpr int WT_FIXED32 = 5;

    inline uint32_t ZigZagEncode32(int32_t n) { return google::protobuf::internal::WireFormatLite::ZigZagEncode32(n); }
    inline uint64_t ZigZagEncode64(int64_t n) { return google::protobuf::internal::WireFormatLite::ZigZagEncode64(n); }
    inline uint32_t MakeTag(int fieldNumber, int wireType) {
        return static_cast<uint32_t>((fieldNumber << 3) | wireType);
    }
    inline size_t TagSize(int fieldNumber, int wireType) {
        return COS::VarintSize32(MakeTag(fieldNumber, wireType));
    }
    inline void WriteTag(COS* out, int fieldNumber, int wireType) {
        out->WriteTag(MakeTag(fieldNumber, wireType));
    }
    // Writes tag + length prefix + raw bytes
    inline void WriteLenDelim(COS* out, int fieldNumber, const void* data, size_t len) {
        WriteTag(out, fieldNumber, WT_LEN);
        out->WriteVarint32(static_cast<uint32_t>(len));
        out->WriteRaw(data, static_cast<int>(len));
    }
    // Size of a length-delimited field (tag + varint length + payload)
    inline size_t LenDelimSize(int fieldNumber, size_t payloadLen) {
        return TagSize(fieldNumber, WT_LEN)
               + COS::VarintSize32(static_cast<uint32_t>(payloadLen))
               + payloadLen;
    }
    // Varint size for a signed int32 (sign-extended encoding)
    inline size_t Int32VarintSize(int32_t v) {
        return v < 0 ? 10 : COS::VarintSize32(static_cast<uint32_t>(v));
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
        WriteTag(output, _protobufId, WT_LEN);
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
            WriteTag(output, _protobufId, WT_LEN);
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
        return LenDelimSize(_protobufId, _value.size());
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVStringMessageValue::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        VerifyUtf8String(_value); // log only, no error
        WriteLenDelim(output, _protobufId, _value.data(), _value.size());
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
            totalSize += LenDelimSize(_protobufId, _value.Get(i).size());
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
            WriteLenDelim(output, _protobufId, s.data(), s.size());
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
        return LenDelimSize(_protobufId, _value.size());
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVBytesMessageValue::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        WriteLenDelim(output, _protobufId, _value.data(), _value.size());
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
            totalSize += LenDelimSize(_protobufId, _value.Get(i).size());
        }
        return totalSize;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------

    void LVRepeatedBytesMessageValue::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        for (int i = 0, n = _value.size(); i < n; i++)
        {
            const auto& s = _value[i];
            WriteLenDelim(output, _protobufId, s.data(), s.size());
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
        return TagSize(_protobufId, WT_VARINT) + 1;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    void LVVariableMessageValue<bool>::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        WriteTag(output, _protobufId, WT_VARINT);
        output->WriteVarint32(_value ? 1u : 0u);
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
        size_t dataSize = 1UL * static_cast<unsigned int>(_value.size());
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
        size_t dataSize = static_cast<size_t>(count);
        WriteTag(output, _protobufId, WT_LEN);
        output->WriteVarint32(static_cast<uint32_t>(dataSize));
        for (int i = 0; i < count; i++)
        {
            output->WriteVarint32(_value.Get(i) ? 1u : 0u);
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
        return TagSize(_protobufId, WT_VARINT) + Int32VarintSize(_value);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    void LVVariableMessageValue<int>::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        WriteTag(output, _protobufId, WT_VARINT);
        output->WriteVarint64(static_cast<uint64_t>(static_cast<int64_t>(_value)));
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
        return TagSize(_protobufId, WT_VARINT) + COS::VarintSize32(_value);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    void LVVariableMessageValue<uint32_t>::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        WriteTag(output, _protobufId, WT_VARINT);
        output->WriteVarint32(_value);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    LVEnumMessageValue::LVEnumMessageValue(int protobufId, int value) :
        LVMessageValue(protobufId),
        _value(value)
    {
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    size_t LVEnumMessageValue::ByteSizeLong()
    {
        return TagSize(_protobufId, WT_VARINT) + Int32VarintSize(_value);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVEnumMessageValue::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        WriteTag(output, _protobufId, WT_VARINT);
        output->WriteVarint64(static_cast<uint64_t>(static_cast<int64_t>(_value)));
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
        return TagSize(_protobufId, WT_VARINT) + COS::VarintSize64(static_cast<uint64_t>(_value));
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    void LVVariableMessageValue<int64_t>::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        WriteTag(output, _protobufId, WT_VARINT);
        output->WriteVarint64(static_cast<uint64_t>(_value));
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
        return TagSize(_protobufId, WT_VARINT) + COS::VarintSize64(_value);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    void LVVariableMessageValue<uint64_t>::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        WriteTag(output, _protobufId, WT_VARINT);
        output->WriteVarint64(_value);
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
        size_t dataSize = 0;
        for (int i = 0, n = _value.size(); i < n; i++)
        {
            dataSize += Int32VarintSize(_value.Get(i));
        }
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
            dataSize = 0;
            for (int i = 0, n = _value.size(); i < n; i++)
            {
                dataSize += Int32VarintSize(_value.Get(i));
            }
            _cachedDataSize = dataSize;
        }
        WriteTag(output, _protobufId, WT_LEN);
        output->WriteVarint32(static_cast<uint32_t>(dataSize));
        for (int i = 0, n = _value.size(); i < n; i++)
        {
            output->WriteVarint64(static_cast<uint64_t>(static_cast<int64_t>(_value.Get(i))));
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
        size_t dataSize = 0;
        for (int i = 0, n = _value.size(); i < n; i++)
        {
            dataSize += COS::VarintSize32(_value.Get(i));
        }
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
        if (dataSize == static_cast<size_t>(-1))
        {
            dataSize = 0;
            for (int i = 0, n = _value.size(); i < n; i++)
            {
                dataSize += COS::VarintSize32(_value.Get(i));
            }
            _cachedDataSize = dataSize;
        }
        WriteTag(output, _protobufId, WT_LEN);
        output->WriteVarint32(static_cast<uint32_t>(dataSize));
        for (int i = 0, n = _value.size(); i < n; i++)
        {
            output->WriteVarint32(_value.Get(i));
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
    size_t LVRepeatedEnumMessageValue::ByteSizeLong()
    {
        size_t dataSize = 0;
        for (int i = 0, n = _value.size(); i < n; i++)
        {
            dataSize += Int32VarintSize(_value.Get(i));
        }
        _cachedDataSize = dataSize;
        if (dataSize == 0) return 0;
        return LenDelimSize(_protobufId, dataSize);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVRepeatedEnumMessageValue::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        if (_value.size() == 0) return;
        size_t dataSize = _cachedDataSize;
        if (dataSize == static_cast<size_t>(-1))
        {
            dataSize = 0;
            for (int i = 0, n = _value.size(); i < n; i++)
            {
                dataSize += Int32VarintSize(_value.Get(i));
            }
            _cachedDataSize = dataSize;
        }
        WriteTag(output, _protobufId, WT_LEN);
        output->WriteVarint32(static_cast<uint32_t>(dataSize));
        for (int i = 0, n = _value.size(); i < n; i++)
        {
            output->WriteVarint64(static_cast<uint64_t>(static_cast<int64_t>(_value.Get(i))));
        }
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
        size_t dataSize = 0;
        for (int i = 0, n = _value.size(); i < n; i++)
        {
            dataSize += COS::VarintSize64(static_cast<uint64_t>(_value.Get(i)));
        }
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
        if (dataSize == static_cast<size_t>(-1))
        {
            dataSize = 0;
            for (int i = 0, n = _value.size(); i < n; i++)
            {
                dataSize += COS::VarintSize64(static_cast<uint64_t>(_value.Get(i)));
            }
            _cachedDataSize = dataSize;
        }
        WriteTag(output, _protobufId, WT_LEN);
        output->WriteVarint32(static_cast<uint32_t>(dataSize));
        for (int i = 0, n = _value.size(); i < n; i++)
        {
            output->WriteVarint64(static_cast<uint64_t>(_value.Get(i)));
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
        size_t dataSize = 0;
        for (int i = 0, n = _value.size(); i < n; i++)
        {
            dataSize += COS::VarintSize64(_value.Get(i));
        }
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
        if (dataSize == static_cast<size_t>(-1))
        {
            dataSize = 0;
            for (int i = 0, n = _value.size(); i < n; i++)
            {
                dataSize += COS::VarintSize64(_value.Get(i));
            }
            _cachedDataSize = dataSize;
        }
        WriteTag(output, _protobufId, WT_LEN);
        output->WriteVarint32(static_cast<uint32_t>(dataSize));
        for (int i = 0, n = _value.size(); i < n; i++)
        {
            output->WriteVarint64(_value.Get(i));
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
        return TagSize(_protobufId, WT_FIXED32) + 4;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    void LVVariableMessageValue<float>::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        WriteTag(output, _protobufId, WT_FIXED32);
        uint32_t bits;
        memcpy(&bits, &_value, 4);
        output->WriteLittleEndian32(bits);
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
        size_t dataSize = 4UL * static_cast<unsigned int>(_value.size());
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
        size_t dataSize = 4UL * static_cast<unsigned int>(count);
        WriteTag(output, _protobufId, WT_LEN);
        output->WriteVarint32(static_cast<uint32_t>(dataSize));
        for (int i = 0; i < count; i++)
        {
            uint32_t bits;
            float v = _value.Get(i);
            memcpy(&bits, &v, 4);
            output->WriteLittleEndian32(bits);
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
        return TagSize(_protobufId, WT_FIXED64) + 8;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    void LVVariableMessageValue<double>::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        WriteTag(output, _protobufId, WT_FIXED64);
        uint64_t bits;
        memcpy(&bits, &_value, 8);
        output->WriteLittleEndian64(bits);
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
        size_t dataSize = 8UL * static_cast<unsigned int>(_value.size());
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
        size_t dataSize = 8UL * static_cast<unsigned int>(count);
        WriteTag(output, _protobufId, WT_LEN);
        output->WriteVarint32(static_cast<uint32_t>(dataSize));
        for (int i = 0; i < count; i++)
        {
            uint64_t bits;
            double v = _value.Get(i);
            memcpy(&bits, &v, 8);
            output->WriteLittleEndian64(bits);
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
        return TagSize(_protobufId, WT_VARINT) + COS::VarintSize32(ZigZagEncode32(_value));
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVSInt32MessageValue::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        WriteTag(output, _protobufId, WT_VARINT);
        output->WriteVarint32(ZigZagEncode32(_value));
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
        size_t dataSize = 0;
        for (int i = 0, n = _value.size(); i < n; i++)
        {
            dataSize += COS::VarintSize32(ZigZagEncode32(_value.Get(i)));
        }
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
        if (dataSize == static_cast<size_t>(-1))
        {
            dataSize = 0;
            for (int i = 0, n = _value.size(); i < n; i++)
            {
                dataSize += COS::VarintSize32(ZigZagEncode32(_value.Get(i)));
            }
            _cachedDataSize = dataSize;
        }
        WriteTag(output, _protobufId, WT_LEN);
        output->WriteVarint32(static_cast<uint32_t>(dataSize));
        for (int i = 0, n = _value.size(); i < n; i++)
        {
            output->WriteVarint32(ZigZagEncode32(_value.Get(i)));
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
        return TagSize(_protobufId, WT_VARINT) + COS::VarintSize64(ZigZagEncode64(_value));
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVSInt64MessageValue::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        WriteTag(output, _protobufId, WT_VARINT);
        output->WriteVarint64(ZigZagEncode64(_value));
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
        size_t dataSize = 0;
        for (int i = 0, n = _value.size(); i < n; i++)
        {
            dataSize += COS::VarintSize64(ZigZagEncode64(_value.Get(i)));
        }
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
        if (dataSize == static_cast<size_t>(-1))
        {
            dataSize = 0;
            for (int i = 0, n = _value.size(); i < n; i++)
            {
                dataSize += COS::VarintSize64(ZigZagEncode64(_value.Get(i)));
            }
            _cachedDataSize = dataSize;
        }
        WriteTag(output, _protobufId, WT_LEN);
        output->WriteVarint32(static_cast<uint32_t>(dataSize));
        for (int i = 0, n = _value.size(); i < n; i++)
        {
            output->WriteVarint64(ZigZagEncode64(_value.Get(i)));
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
        return TagSize(_protobufId, WT_FIXED32) + 4;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVFixed32MessageValue::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        WriteTag(output, _protobufId, WT_FIXED32);
        output->WriteLittleEndian32(_value);
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
        size_t dataSize = 4UL * static_cast<unsigned int>(_value.size());
        if (dataSize == 0) return 0;
        return LenDelimSize(_protobufId, dataSize);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVRepeatedFixed32MessageValue::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        int count = _value.size();
        if (count == 0) return;
        size_t dataSize = 4UL * static_cast<unsigned int>(count);
        WriteTag(output, _protobufId, WT_LEN);
        output->WriteVarint32(static_cast<uint32_t>(dataSize));
        for (int i = 0; i < count; i++)
        {
            output->WriteLittleEndian32(_value.Get(i));
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
        return TagSize(_protobufId, WT_FIXED64) + 8;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVFixed64MessageValue::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        WriteTag(output, _protobufId, WT_FIXED64);
        output->WriteLittleEndian64(_value);
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
        size_t dataSize = 8UL * static_cast<unsigned int>(_value.size());
        if (dataSize == 0) return 0;
        return LenDelimSize(_protobufId, dataSize);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVRepeatedFixed64MessageValue::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        int count = _value.size();
        if (count == 0) return;
        size_t dataSize = 8UL * static_cast<unsigned int>(count);
        WriteTag(output, _protobufId, WT_LEN);
        output->WriteVarint32(static_cast<uint32_t>(dataSize));
        for (int i = 0; i < count; i++)
        {
            output->WriteLittleEndian64(_value.Get(i));
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
        return TagSize(_protobufId, WT_FIXED32) + 4;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVSFixed32MessageValue::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        WriteTag(output, _protobufId, WT_FIXED32);
        output->WriteLittleEndian32(static_cast<uint32_t>(_value));
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
        size_t dataSize = 4UL * static_cast<unsigned int>(_value.size());
        if (dataSize == 0) return 0;
        return LenDelimSize(_protobufId, dataSize);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVRepeatedSFixed32MessageValue::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        int count = _value.size();
        if (count == 0) return;
        size_t dataSize = 4UL * static_cast<unsigned int>(count);
        WriteTag(output, _protobufId, WT_LEN);
        output->WriteVarint32(static_cast<uint32_t>(dataSize));
        for (int i = 0; i < count; i++)
        {
            output->WriteLittleEndian32(static_cast<uint32_t>(_value.Get(i)));
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
        return TagSize(_protobufId, WT_FIXED64) + 8;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVSFixed64MessageValue::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        WriteTag(output, _protobufId, WT_FIXED64);
        output->WriteLittleEndian64(static_cast<uint64_t>(_value));
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
        size_t dataSize = 8UL * static_cast<unsigned int>(_value.size());
        if (dataSize == 0) return 0;
        return LenDelimSize(_protobufId, dataSize);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void LVRepeatedSFixed64MessageValue::Serialize(google::protobuf::io::CodedOutputStream* output) const
    {
        int count = _value.size();
        if (count == 0) return;
        size_t dataSize = 8UL * static_cast<unsigned int>(count);
        WriteTag(output, _protobufId, WT_LEN);
        output->WriteVarint32(static_cast<uint32_t>(dataSize));
        for (int i = 0; i < count; i++)
        {
            output->WriteLittleEndian64(static_cast<uint64_t>(_value.Get(i)));
        }
    }
}
