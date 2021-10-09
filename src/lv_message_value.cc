//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_server.h>
#include <lv_message.h>

//---------------------------------------------------------------------
//---------------------------------------------------------------------
using namespace google::protobuf::internal;

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
    return WireFormatLite::InternalWriteMessage(_protobufId, *_value, target, stream);        
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
    totalSize += 1UL * _value.size();
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
        target = WireFormatLite::InternalWriteMessage(_protobufId, *_value[i], target, stream);
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

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVRepeatedStringMessageValue::ByteSizeLong()
{    
    size_t totalSize = 0;
    totalSize += 1 * FromIntSize(_value.size());
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
LVBooleanMessageValue::LVBooleanMessageValue(int protobufId, bool value) :
    LVMessageValue(protobufId),
    _value(value)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVBooleanMessageValue::ByteSizeLong()
{
    return WireFormatLite::TagSize(_protobufId, WireFormatLite::TYPE_BOOL) + WireFormatLite::kBoolSize;    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8* LVBooleanMessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{    
    target = stream->EnsureSpace(target);
    return WireFormatLite::WriteBoolToArray(_protobufId, _value, target);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVRepeatedBooleanMessageValue::LVRepeatedBooleanMessageValue(int protobufId) :
    LVMessageValue(protobufId)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVRepeatedBooleanMessageValue::ByteSizeLong()
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
google::protobuf::uint8* LVRepeatedBooleanMessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{
    if (_value.size() > 0)
    {
        target = stream->WriteFixedPacked(_protobufId, _value, target);
    }
    return target;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVInt32MessageValue::LVInt32MessageValue(int protobufId, int value) :
    LVMessageValue(protobufId),
    _value(value)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVInt32MessageValue::ByteSizeLong()
{
    return WireFormatLite::TagSize(_protobufId, WireFormatLite::FieldType::TYPE_INT32) +  WireFormatLite::Int32Size(_value);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8* LVInt32MessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{    
    target = stream->EnsureSpace(target);
    return WireFormatLite::WriteInt32ToArray(_protobufId, _value, target);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVUInt32MessageValue::LVUInt32MessageValue(int protobufId, uint32_t value) :
    LVMessageValue(protobufId),
    _value(value)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVUInt32MessageValue::ByteSizeLong()
{
    return WireFormatLite::TagSize(_protobufId, WireFormatLite::FieldType::TYPE_UINT32) +  WireFormatLite::UInt32Size(_value);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8* LVUInt32MessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
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
LVInt64MessageValue::LVInt64MessageValue(int protobufId, int64_t value) :
    LVMessageValue(protobufId),
    _value(value)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVInt64MessageValue::ByteSizeLong()
{
    return WireFormatLite::TagSize(_protobufId, WireFormatLite::FieldType::TYPE_INT64) + WireFormatLite::Int64Size(_value);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8* LVInt64MessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{    
    target = stream->EnsureSpace(target);
    return WireFormatLite::WriteInt64ToArray(_protobufId, _value, target);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVUInt64MessageValue::LVUInt64MessageValue(int protobufId, uint64_t value) :
    LVMessageValue(protobufId),
    _value(value)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVUInt64MessageValue::ByteSizeLong()
{
    return WireFormatLite::TagSize(_protobufId, WireFormatLite::FieldType::TYPE_UINT64) +  WireFormatLite::UInt64Size(_value);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8* LVUInt64MessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{    
    target = stream->EnsureSpace(target);
    return WireFormatLite::WriteUInt64ToArray(_protobufId, _value, target);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVRepeatedInt32MessageValue::LVRepeatedInt32MessageValue(int protobufId) :
    LVMessageValue(protobufId)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVRepeatedInt32MessageValue::ByteSizeLong()
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
google::protobuf::uint8* LVRepeatedInt32MessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{
    if (_cachedSize > 0)
    {
        target = stream->WriteInt32Packed(_protobufId, _value, _cachedSize, target);
    }
    return target;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVRepeatedUInt32MessageValue::LVRepeatedUInt32MessageValue(int protobufId) :
    LVMessageValue(protobufId)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVRepeatedUInt32MessageValue::ByteSizeLong()
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
google::protobuf::uint8* LVRepeatedUInt32MessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
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
    LVMessageValue(protobufId)
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
LVRepeatedInt64MessageValue::LVRepeatedInt64MessageValue(int protobufId) :
    LVMessageValue(protobufId)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVRepeatedInt64MessageValue::ByteSizeLong()
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
google::protobuf::uint8* LVRepeatedInt64MessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{
    if (_cachedSize > 0)
    {
        target = stream->WriteInt64Packed(_protobufId, _value, _cachedSize, target);
    }
    return target;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVRepeatedUInt64MessageValue::LVRepeatedUInt64MessageValue(int protobufId) :
    LVMessageValue(protobufId)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVRepeatedUInt64MessageValue::ByteSizeLong()
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
google::protobuf::uint8* LVRepeatedUInt64MessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{
    if (_cachedSize > 0)
    {
        target = stream->WriteUInt64Packed(_protobufId, _value, _cachedSize, target);
    }
    return target;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVFloatMessageValue::LVFloatMessageValue(int protobufId, float value) :
    LVMessageValue(protobufId),
    _value(value)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVFloatMessageValue::ByteSizeLong()
{    
    return WireFormatLite::TagSize(_protobufId, WireFormatLite::TYPE_STRING) + WireFormatLite::kFloatSize;    
}
    
//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8* LVFloatMessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{    
    target = stream->EnsureSpace(target);
    return WireFormatLite::WriteFloatToArray(_protobufId, _value, target);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVRepeatedFloatMessageValue::LVRepeatedFloatMessageValue(int protobufId) :
    LVMessageValue(protobufId)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVRepeatedFloatMessageValue::ByteSizeLong()
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
google::protobuf::uint8* LVRepeatedFloatMessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{    
    if (_value.size() > 0)
    {
        target = stream->WriteFixedPacked(_protobufId, _value, target);
    }
    return target;
}


//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVDoubleMessageValue::LVDoubleMessageValue(int protobufId, double value) :
    LVMessageValue(protobufId),
    _value(value)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVDoubleMessageValue::ByteSizeLong()
{    
    return WireFormatLite::TagSize(_protobufId, WireFormatLite::FieldType::TYPE_DOUBLE) +  WireFormatLite::kDoubleSize;    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
google::protobuf::uint8* LVDoubleMessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{
    target = stream->EnsureSpace(target);
    return WireFormatLite::WriteDoubleToArray(_protobufId, _value, target);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LVRepeatedDoubleMessageValue::LVRepeatedDoubleMessageValue(int protobufId) :
    LVMessageValue(protobufId)
{    
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
size_t LVRepeatedDoubleMessageValue::ByteSizeLong()
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
google::protobuf::uint8* LVRepeatedDoubleMessageValue::Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const
{    
    if (_value.size() > 0)
    {
        target = stream->WriteFixedPacked(_protobufId, _value, target);
    }
    return target;
}
