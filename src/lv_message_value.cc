//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_server.h>
#include <lv_message.h>
#include <message_value.h>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
using namespace google::protobuf::internal;

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
        return WireFormatLite::TagSize(_protobufId, WireFormatLite::TYPE_MESSAGE) + WireFormatLite::MessageSize(*_value);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    google::protobuf::uint8* LVNestedMessageMessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
    {
        target = stream->EnsureSpace(target);
        return WireFormatLite::InternalWriteMessage(_protobufId, *_value, _value->GetCachedSize(), target, stream);
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
        size_t totalSize = 0;
        totalSize += WireFormatLite::TagSize(_protobufId, WireFormatLite::TYPE_MESSAGE) * _value.size();
        for (const auto& msg : _value)
        {
            totalSize += WireFormatLite::MessageSize(*msg);
        }
        return totalSize;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    google::protobuf::uint8* LVRepeatedNestedMessageMessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
    {
        for (unsigned int i = 0, n = static_cast<unsigned int>(_value.size()); i < n; i++)
        {
            target = stream->EnsureSpace(target);
            target = WireFormatLite::InternalWriteMessage(_protobufId, *_value[i], _value[i]->GetCachedSize(), target, stream);
        }
        return target;
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
        return WireFormatLite::TagSize(_protobufId, WireFormatLite::TYPE_STRING) + WireFormatLite::StringSize(_value);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    google::protobuf::uint8* LVStringMessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
    {
        target = stream->EnsureSpace(target);
        return stream->WriteString(_protobufId, _value, target);
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
        totalSize += WireFormatLite::TagSize(_protobufId, WireFormatLite::TYPE_STRING) * static_cast<unsigned int>(_value.size());
        for (int i = 0, n = _value.size(); i < n; i++)
        {
            totalSize += WireFormatLite::StringSize(_value.Get(i));
        }
        return totalSize;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
   
    google::protobuf::uint8* LVRepeatedStringMessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
    {
        for (int i = 0, n = _value.size(); i < n; i++)
        {
            const auto& s = _value[i];
            target = stream->WriteString(_protobufId, s, target);
        }
        return target;
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
        return WireFormatLite::TagSize(_protobufId, WireFormatLite::TYPE_BOOL) + WireFormatLite::kBoolSize;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    google::protobuf::uint8* LVVariableMessageValue<bool>::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
    {
        target = stream->EnsureSpace(target);
        return WireFormatLite::WriteBoolToArray(_protobufId, _value, target);
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
        size_t totalSize = 0;
        unsigned int count = static_cast<unsigned int>(_value.size());
        size_t dataSize = 1UL * count;
        if (dataSize > 0)
        {
            // passing 2 as type to TagSize because that is what WriteLengthDelim passes during serialize
            totalSize += WireFormatLite::TagSize(_protobufId, (WireFormatLite::FieldType)2) + WireFormatLite::Int32Size(static_cast<google::protobuf::int32>(dataSize));
        }
        totalSize += dataSize;
        return totalSize;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    google::protobuf::uint8* LVRepeatedMessageValue<bool>::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
    {
        if (_value.size() > 0)
        {
            target = stream->WriteFixedPacked(_protobufId, _value, target);
        }
        return target;
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
        return WireFormatLite::TagSize(_protobufId, WireFormatLite::FieldType::TYPE_INT32) +  WireFormatLite::Int32Size(_value);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    google::protobuf::uint8* LVVariableMessageValue<int>::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
    {
        target = stream->EnsureSpace(target);
        return WireFormatLite::WriteInt32ToArray(_protobufId, _value, target);
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
        return WireFormatLite::TagSize(_protobufId, WireFormatLite::FieldType::TYPE_UINT32) +  WireFormatLite::UInt32Size(_value);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    google::protobuf::uint8* LVVariableMessageValue<uint32_t>::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
    {
        target = stream->EnsureSpace(target);
        return WireFormatLite::WriteUInt32ToArray(_protobufId, _value, target);
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
        return WireFormatLite::TagSize(_protobufId, WireFormatLite::FieldType::TYPE_ENUM) +  WireFormatLite::EnumSize(_value);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    google::protobuf::uint8* LVEnumMessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
    {
        target = stream->EnsureSpace(target);
        return WireFormatLite::WriteEnumToArray(_protobufId, _value, target);
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
        return WireFormatLite::TagSize(_protobufId, WireFormatLite::FieldType::TYPE_INT64) + WireFormatLite::Int64Size(_value);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    google::protobuf::uint8* LVVariableMessageValue<int64_t>::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
    {
        target = stream->EnsureSpace(target);
        return WireFormatLite::WriteInt64ToArray(_protobufId, _value, target);
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
        return WireFormatLite::TagSize(_protobufId, WireFormatLite::FieldType::TYPE_UINT64) +  WireFormatLite::UInt64Size(_value);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    google::protobuf::uint8* LVVariableMessageValue<uint64_t>::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
    {
        target = stream->EnsureSpace(target);
        return WireFormatLite::WriteUInt64ToArray(_protobufId, _value, target);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    /*LVRepeatedMessageValue<int>::LVRepeatedMessageValue(int protobufId) :
        LVMessageValue(protobufId)
    {
    }*/

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    size_t LVRepeatedMessageValue<int>::ByteSizeLong()
    {
        size_t totalSize = 0;
        size_t dataSize = WireFormatLite::Int32Size(_value);
        if (dataSize > 0)
        {
            // passing 2 as type to TagSize because that is what WriteLengthDelim passes during serialize
            totalSize += WireFormatLite::TagSize(_protobufId, (WireFormatLite::FieldType)2) + WireFormatLite::Int32Size(static_cast<google::protobuf::int32>(dataSize));
        }
        _cachedSize = ToCachedSize(dataSize);
        totalSize += dataSize;
        return totalSize;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    google::protobuf::uint8* LVRepeatedMessageValue<int>::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
    {
        if (_cachedSize > 0)
        {
            target = stream->WriteInt32Packed(_protobufId, _value, _cachedSize, target);
        }
        return target;
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
        size_t totalSize = 0;
        size_t dataSize = WireFormatLite::UInt32Size(_value);
        if (dataSize > 0)
        {
            // passing 2 as type to TagSize because that is what WriteLengthDelim passes during serialize
            totalSize += WireFormatLite::TagSize(_protobufId, (WireFormatLite::FieldType)2) + WireFormatLite::UInt32Size(static_cast<google::protobuf::int32>(dataSize));
        }
        _cachedSize = ToCachedSize(dataSize);
        totalSize += dataSize;
        return totalSize;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    google::protobuf::uint8* LVRepeatedMessageValue<uint32_t>::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
    {
        if (_cachedSize > 0)
        {
            target = stream->WriteUInt32Packed(_protobufId, _value, _cachedSize, target);
        }
        return target;
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
        size_t totalSize = 0;
        size_t dataSize = WireFormatLite::EnumSize(_value);
        if (dataSize > 0)
        {
            // passing 2 as type to TagSize because that is what WriteLengthDelim passes during serialize
            totalSize += WireFormatLite::TagSize(_protobufId, (WireFormatLite::FieldType)2) + WireFormatLite::EnumSize(static_cast<google::protobuf::int32>(dataSize));
        }
        _cachedSize = ToCachedSize(dataSize);
        totalSize += dataSize;
        return totalSize;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    google::protobuf::uint8* LVRepeatedEnumMessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
    {
        if (_cachedSize > 0)
        {
            target = stream->WriteEnumPacked(_protobufId, _value, _cachedSize, target);
        }
        return target;
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
        size_t totalSize = 0;
        size_t dataSize = WireFormatLite::Int64Size(_value);
        if (dataSize > 0)
        {
            // passing 2 as type to TagSize because that is what WriteLengthDelim passes during serialize
            totalSize += WireFormatLite::TagSize(_protobufId, (WireFormatLite::FieldType)2) + WireFormatLite::Int64Size(static_cast<google::protobuf::int32>(dataSize));
        }
        _cachedSize = ToCachedSize(dataSize);
        totalSize += dataSize;
        return totalSize;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    google::protobuf::uint8* LVRepeatedMessageValue<int64_t>::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
    {
        if (_cachedSize > 0)
        {
            target = stream->WriteInt64Packed(_protobufId, _value, _cachedSize, target);
        }
        return target;
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
        size_t totalSize = 0;
        size_t dataSize = WireFormatLite::UInt64Size(_value);
        if (dataSize > 0)
        {
            // passing 2 as type to TagSize because that is what WriteLengthDelim passes during serialize
            totalSize += WireFormatLite::TagSize(_protobufId, (WireFormatLite::FieldType)2) + WireFormatLite::UInt64Size(static_cast<google::protobuf::int32>(dataSize));
        }
        _cachedSize = ToCachedSize(dataSize);
        totalSize += dataSize;
        return totalSize;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    google::protobuf::uint8* LVRepeatedMessageValue<uint64_t>::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
    {
        if (_cachedSize > 0)
        {
            target = stream->WriteUInt64Packed(_protobufId, _value, _cachedSize, target);
        }
        return target;
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
        return WireFormatLite::TagSize(_protobufId, WireFormatLite::TYPE_STRING) + WireFormatLite::kFloatSize;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    google::protobuf::uint8* LVVariableMessageValue<float>::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
    {
        target = stream->EnsureSpace(target);
        return WireFormatLite::WriteFloatToArray(_protobufId, _value, target);
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
        size_t totalSize = 0;
        unsigned int count = static_cast<unsigned int>(_value.size());
        size_t dataSize = 4UL * count;
        if (dataSize > 0)
        {
            // passing 2 as type to TagSize because that is what WriteLengthDelim passes during serialize
            totalSize += WireFormatLite::TagSize(_protobufId, (WireFormatLite::FieldType)2) + WireFormatLite::Int32Size(static_cast<google::protobuf::int32>(dataSize));
        }
        totalSize += dataSize;
        return totalSize;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    google::protobuf::uint8* LVRepeatedMessageValue<float>::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
    {
        if (_value.size() > 0)
        {
            target = stream->WriteFixedPacked(_protobufId, _value, target);
        }
        return target;
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
        return WireFormatLite::TagSize(_protobufId, WireFormatLite::FieldType::TYPE_DOUBLE) +  WireFormatLite::kDoubleSize;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    google::protobuf::uint8* LVVariableMessageValue<double>::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
    {
        target = stream->EnsureSpace(target);
        return WireFormatLite::WriteDoubleToArray(_protobufId, _value, target);
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
        size_t totalSize = 0;
        unsigned int count = static_cast<unsigned int>(_value.size());
        size_t dataSize = 8UL * count;
        if (dataSize > 0)
        {
            // passing 2 as type to TagSize because that is what WriteLengthDelim passes during serialize
            totalSize += WireFormatLite::TagSize(_protobufId, (WireFormatLite::FieldType)2) + WireFormatLite::Int32Size(static_cast<google::protobuf::int32>(dataSize));
        }
        totalSize += dataSize;
        return totalSize;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <>
    google::protobuf::uint8* LVRepeatedMessageValue<double>::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
    {
        if (_value.size() > 0)
        {
            target = stream->WriteFixedPacked(_protobufId, _value, target);
        }
        return target;
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
        return WireFormatLite::TagSize(_protobufId, WireFormatLite::TYPE_SINT32) + WireFormatLite::SInt32Size(_value);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    google::protobuf::uint8* LVSInt32MessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
    {
        target = stream->EnsureSpace(target);
        return WireFormatLite::WriteSInt32ToArray(_protobufId, _value, target);
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
        size_t totalSize = 0;
        size_t dataSize = WireFormatLite::SInt32Size(_value);
        if (dataSize > 0)
        {
            // passing 2 as type to TagSize because that is what WriteLengthDelim passes during serialize
            totalSize += WireFormatLite::TagSize(_protobufId, (WireFormatLite::FieldType)2) + WireFormatLite::SInt32Size(static_cast<google::protobuf::int32>(dataSize));
        }
        _cachedSize = ToCachedSize(dataSize);
        totalSize += dataSize;
        return totalSize;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    google::protobuf::uint8* LVRepeatedSInt32MessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
    {
        if (_cachedSize > 0)
        {
            target = stream->WriteSInt32Packed(_protobufId, _value, _cachedSize, target);
        }
        return target;
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
        return WireFormatLite::TagSize(_protobufId, WireFormatLite::TYPE_SINT64) + WireFormatLite::SInt64Size(_value);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    google::protobuf::uint8* LVSInt64MessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
    {
        target = stream->EnsureSpace(target);
        return WireFormatLite::WriteSInt64ToArray(_protobufId, _value, target);
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
        size_t totalSize = 0;
        size_t dataSize = WireFormatLite::SInt64Size(_value);
        if (dataSize > 0)
        {
            // passing 2 as type to TagSize because that is what WriteLengthDelim passes during serialize
            totalSize += WireFormatLite::TagSize(_protobufId, (WireFormatLite::FieldType)2) + WireFormatLite::SInt64Size(static_cast<google::protobuf::int32>(dataSize));
        }
        _cachedSize = ToCachedSize(dataSize);
        totalSize += dataSize;
        return totalSize;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    google::protobuf::uint8* LVRepeatedSInt64MessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
    {
        if (_cachedSize > 0)
        {
            target = stream->WriteSInt64Packed(_protobufId, _value, _cachedSize, target);
        }
        return target;
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
        return WireFormatLite::TagSize(_protobufId, WireFormatLite::TYPE_FIXED32) + WireFormatLite::kFixed32Size;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    google::protobuf::uint8* LVFixed32MessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
    {
        target = stream->EnsureSpace(target);
        return WireFormatLite::WriteFixed32ToArray(_protobufId, _value, target);
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
        size_t totalSize = 0;
        unsigned int count = static_cast<unsigned int>(_value.size());
        size_t dataSize = WireFormatLite::kFixed32Size * count;
        if (dataSize > 0)
        {
            // passing 2 as type to TagSize because that is what WriteLengthDelim passes during serialize
            totalSize += WireFormatLite::TagSize(_protobufId, (WireFormatLite::FieldType)2) + WireFormatLite::UInt32Size(static_cast<google::protobuf::int32>(dataSize));
        }
        totalSize += dataSize;
        return totalSize;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    google::protobuf::uint8* LVRepeatedFixed32MessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
    {
        if (_value.size() > 0)
        {
            target = stream->WriteFixedPacked(_protobufId, _value, target);
        }
        return target;
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
        return WireFormatLite::TagSize(_protobufId, WireFormatLite::TYPE_FIXED64) + WireFormatLite::kFixed64Size;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    google::protobuf::uint8* LVFixed64MessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
    {
        target = stream->EnsureSpace(target);
        return WireFormatLite::WriteFixed64ToArray(_protobufId, _value, target);
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
        size_t totalSize = 0;
        unsigned int count = static_cast<unsigned int>(_value.size());
        size_t dataSize = WireFormatLite::kFixed64Size * count;
        if (dataSize > 0)
        {
            // passing 2 as type to TagSize because that is what WriteLengthDelim passes during serialize
            totalSize += WireFormatLite::TagSize(_protobufId, (WireFormatLite::FieldType)2) + WireFormatLite::UInt64Size(static_cast<google::protobuf::int32>(dataSize));
        }
        totalSize += dataSize;
        return totalSize;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    google::protobuf::uint8* LVRepeatedFixed64MessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
    {
        if (_value.size() > 0)
        {
            target = stream->WriteFixedPacked(_protobufId, _value, target);
        }
        return target;
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
        return WireFormatLite::TagSize(_protobufId, WireFormatLite::TYPE_SFIXED32) + WireFormatLite::kSFixed32Size;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    google::protobuf::uint8* LVSFixed32MessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
    {
        target = stream->EnsureSpace(target);
        return WireFormatLite::WriteSFixed32ToArray(_protobufId, _value, target);
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
        size_t totalSize = 0;
        unsigned int count = static_cast<unsigned int>(_value.size());
        size_t dataSize = WireFormatLite::kSFixed32Size * count;
        if (dataSize > 0)
        {
            // passing 2 as type to TagSize because that is what WriteLengthDelim passes during serialize
            totalSize += WireFormatLite::TagSize(_protobufId, (WireFormatLite::FieldType)2) + WireFormatLite::Int32Size(static_cast<google::protobuf::int32>(dataSize));
        }
        totalSize += dataSize;
        return totalSize;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    google::protobuf::uint8* LVRepeatedSFixed32MessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
    {
        if (_value.size() > 0)
        {
            target = stream->WriteFixedPacked(_protobufId, _value, target);
        }
        return target;
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
        return WireFormatLite::TagSize(_protobufId, WireFormatLite::TYPE_SFIXED64) + WireFormatLite::kSFixed64Size;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    google::protobuf::uint8* LVSFixed64MessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
    {
        target = stream->EnsureSpace(target);
        return WireFormatLite::WriteSFixed64ToArray(_protobufId, _value, target);
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
        size_t totalSize = 0;
        unsigned int count = static_cast<unsigned int>(_value.size());
        size_t dataSize = WireFormatLite::kSFixed64Size * count;
        if (dataSize > 0)
        {
            // passing 2 as type to TagSize because that is what WriteLengthDelim passes during serialize
            totalSize += WireFormatLite::TagSize(_protobufId, (WireFormatLite::FieldType)2) + WireFormatLite::Int64Size(static_cast<google::protobuf::int32>(dataSize));
        }
        totalSize += dataSize;
        return totalSize;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    google::protobuf::uint8* LVRepeatedSFixed64MessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
    {
        if (_value.size() > 0)
        {
            target = stream->WriteFixedPacked(_protobufId, _value, target);
        }
        return target;
    }
}
