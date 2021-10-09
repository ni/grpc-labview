//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LVMessageValue
{
public:
    LVMessageValue(int protobufId);

public:
    int _protobufId;    

public:
    virtual void* RawValue() = 0;
    virtual size_t ByteSizeLong() = 0;
    virtual google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const = 0;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LVNestedMessageMessageValue : public LVMessageValue
{
public:
    LVNestedMessageMessageValue(int protobufId, std::shared_ptr<LVMessage> value);

public:
    std::shared_ptr<LVMessage> _value;

public:
    void* RawValue() override { return (void*)(_value.get()); };
    size_t ByteSizeLong() override;
    google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LVRepeatedNestedMessageMessageValue : public LVMessageValue, public LVgRPCid
{
public:
    LVRepeatedNestedMessageMessageValue(int protobufId);

public:
    std::vector<std::shared_ptr<LVMessage>> _value;

public:
    void* RawValue() override { return &_value; };
    size_t ByteSizeLong() override;
    google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LVStringMessageValue : public LVMessageValue
{
public:
    LVStringMessageValue(int protobufId, std::string& value);

public:
    std::string _value;

public:
    void* RawValue() override { return (void*)(_value.c_str()); };
    size_t ByteSizeLong() override;
    google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LVRepeatedStringMessageValue : public LVMessageValue
{
public:
    LVRepeatedStringMessageValue(int protobufId);

public:
    google::protobuf::RepeatedField<std::string> _value;

public:
    void* RawValue() override { return &_value; };
    size_t ByteSizeLong() override;
    google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LVBooleanMessageValue : public LVMessageValue
{
public:
    LVBooleanMessageValue(int protobufId, bool value);

public:
    bool _value;    

    void* RawValue() override { return &_value; };
    size_t ByteSizeLong() override;
    google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LVRepeatedBooleanMessageValue : public LVMessageValue
{
public:
    LVRepeatedBooleanMessageValue(int protobufId);

public:
    google::protobuf::RepeatedField<bool> _value;    

    void* RawValue() override { return &_value; };
    size_t ByteSizeLong() override;
    google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LVInt32MessageValue : public LVMessageValue
{
public:
    LVInt32MessageValue(int protobufId, int value);

public:
    int _value;    

    void* RawValue() override { return &_value; };
    size_t ByteSizeLong() override;
    google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LVUInt32MessageValue : public LVMessageValue
{
public:
    LVUInt32MessageValue(int protobufId, uint32_t value);

public:
    uint32_t _value;    

    void* RawValue() override { return &_value; };
    size_t ByteSizeLong() override;
    google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LVEnumMessageValue : public LVMessageValue
{
public:
    LVEnumMessageValue(int protobufId, int value);

public:
    int _value;    

    void* RawValue() override { return &_value; };
    size_t ByteSizeLong() override;
    google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LVInt64MessageValue : public LVMessageValue
{
public:
    LVInt64MessageValue(int protobufId, int64_t value);

public:
    int64_t _value;    

    void* RawValue() override { return &_value; };
    size_t ByteSizeLong() override;
    google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LVUInt64MessageValue : public LVMessageValue
{
public:
    LVUInt64MessageValue(int protobufId, uint64_t value);

public:
    uint64_t _value;    

public:
    void* RawValue() override { return &_value; };
    size_t ByteSizeLong() override;
    google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LVRepeatedInt32MessageValue : public LVMessageValue
{
public:
    LVRepeatedInt32MessageValue(int protobufId);

public:
    google::protobuf::RepeatedField<int> _value;    

    void* RawValue() override { return &_value; };
    size_t ByteSizeLong() override;
    google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;

private:
    int _cachedSize;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LVRepeatedUInt32MessageValue : public LVMessageValue
{
public:
    LVRepeatedUInt32MessageValue(int protobufId);

public:
    google::protobuf::RepeatedField<uint32_t> _value;    

    void* RawValue() override { return &_value; };
    size_t ByteSizeLong() override;
    google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;

private:
    int _cachedSize;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LVRepeatedEnumMessageValue : public LVMessageValue
{
public:
    LVRepeatedEnumMessageValue(int protobufId);

public:
    google::protobuf::RepeatedField<int> _value;    

    void* RawValue() override { return &_value; };
    size_t ByteSizeLong() override;
    google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;

private:
    int _cachedSize;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LVRepeatedInt64MessageValue : public LVMessageValue
{
public:
    LVRepeatedInt64MessageValue(int protobufId);

public:
    google::protobuf::RepeatedField<int64_t> _value;    

    void* RawValue() override { return &_value; };
    size_t ByteSizeLong() override;
    google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;

private:
    int _cachedSize;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LVRepeatedUInt64MessageValue : public LVMessageValue
{
public:
    LVRepeatedUInt64MessageValue(int protobufId);

public:
    google::protobuf::RepeatedField<uint64_t> _value;    

    void* RawValue() override { return &_value; };
    size_t ByteSizeLong() override;
    google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;

private:
    int _cachedSize;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LVFloatMessageValue : public LVMessageValue
{
public:
    LVFloatMessageValue(int protobufId, float value);

public:
    float _value;    

public:
    void* RawValue() override { return &_value; };
    size_t ByteSizeLong() override;
    google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LVRepeatedFloatMessageValue : public LVMessageValue
{
public:
    LVRepeatedFloatMessageValue(int protobufId);

public:
    google::protobuf::RepeatedField<float> _value;    

public:
    void* RawValue() override { return &_value; };
    size_t ByteSizeLong() override;
    google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LVDoubleMessageValue : public LVMessageValue
{
public:
    LVDoubleMessageValue(int protobufId, double value);

public:
    double _value;    

public:
    void* RawValue() override { return &_value; };
    size_t ByteSizeLong() override;
    google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;
};

//---------------------------------------------------------------------
//---------------------------------------------------------------------
class LVRepeatedDoubleMessageValue : public LVMessageValue
{
public:
    LVRepeatedDoubleMessageValue(int protobufId);

public:
    google::protobuf::RepeatedField<double> _value;    

public:
    void* RawValue() override { return &_value; };
    size_t ByteSizeLong() override;
    google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;
};
