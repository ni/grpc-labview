//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_server.h>
#include <google/protobuf/io/coded_stream.h>

namespace grpc_labview
{
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class LVMessage;

    // This enum contains all the gRPC datatypes. Might be useful in future.
    // enum class LVMessageType {
    //     INT32 = 0,
    //     INT64 = 1,
    //     UINT32 = 2,
    //     UINT64 = 3,
    //     FLOAT = 4,
    //     DOUBLE = 5,
    //     BOOL = 6,
    //     STRING = 7,
    //     BYTES = 8,
    //     ENUM = 9,
    //     SINT32 = 10,
    //     SINT64 = 11,
    //     FIXED32 = 12,
    //     FIXED64 = 13,
    //     SFIXED32 = 14,
    //     SFIXED64 = 15,
    //     DEFAULT = 100,
    // };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class LVMessageValue
    {
    public:
        LVMessageValue(int protobufId);
        virtual ~LVMessageValue() = default;

    public:
        int _protobufId;

    public:
        virtual void* RawValue() = 0;
        virtual size_t ByteSizeLong() = 0;
        virtual void Serialize(google::protobuf::io::CodedOutputStream* output) const = 0;
    };


    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    template <typename T>
    class LVRepeatedMessageValue : public LVMessageValue
    {
    public:
        LVRepeatedMessageValue(int protobufId) :
            LVMessageValue(protobufId)
        {
        }

        google::protobuf::RepeatedField<T> _value;

        // Cache for the packed payload byte count (sum of encoded element sizes,
        // NOT including the outer tag or length-prefix varint).
        // Sentinel value -1 means "not yet computed"; ByteSizeLong() always
        // refreshes it, and Serialize() reads it to avoid recomputing the
        // per-element sizes a second time.  No explicit invalidation is needed:
        // LVMessage values are effectively write-once — they are built during
        // parsing and then serialized.  For streaming calls where the same
        // LVMessage object is reused across writes, LVMessage::Clear() destroys
        // every LVMessageValue in _values entirely, so stale caches in the old
        // value objects cannot be observed.  New LVMessageValue objects created
        // for the next write start with the sentinel, so ByteSizeLong() will
        // always recompute before Serialize() is called.
        mutable size_t _cachedDataSize = static_cast<size_t>(-1);

        void* RawValue() override { return &_value; };
        size_t ByteSizeLong() override;
        void Serialize(google::protobuf::io::CodedOutputStream* output) const override;
    };

    template <typename T>
    class LVVariableMessageValue : public LVMessageValue
    {
    public:
        LVVariableMessageValue(int protobufId, T value) :
            LVMessageValue(protobufId),
            _value(value)
        {
        }

    public:
        T _value;

        void* RawValue() override { return &_value; };
        size_t ByteSizeLong() override;
        void Serialize(google::protobuf::io::CodedOutputStream* output) const override;
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
        void Serialize(google::protobuf::io::CodedOutputStream* output) const override;

    private:
        mutable size_t _cachedNestedByteSize = static_cast<size_t>(-1);
    };

    
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class LVRepeatedNestedMessageMessageValue : public LVMessageValue, public gRPCid
    {
    public:
        LVRepeatedNestedMessageMessageValue(int protobufId);

    public:
        std::vector<std::shared_ptr<LVMessage>> _value;

    public:
        void* RawValue() override { return &_value; };
        size_t ByteSizeLong() override;
        void Serialize(google::protobuf::io::CodedOutputStream* output) const override;

    private:
        mutable std::vector<size_t> _cachedNestedByteSizes;
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
        void Serialize(google::protobuf::io::CodedOutputStream* output) const override;
    };

    class LVRepeatedStringMessageValue : public LVMessageValue
    {
    public:
        LVRepeatedStringMessageValue(int protobufId);

    public:
        google::protobuf::RepeatedPtrField<std::string> _value;

    public:
        void* RawValue() override { return &_value; };
        size_t ByteSizeLong() override;
        void Serialize(google::protobuf::io::CodedOutputStream* output) const override;
    };


    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class LVBytesMessageValue : public LVMessageValue
    {
    public:
        LVBytesMessageValue(int protobufId, std::string& value);

    public:
        std::string _value;

    public:
        void* RawValue() override { return (void*)(_value.c_str()); };
        size_t ByteSizeLong() override;
        void Serialize(google::protobuf::io::CodedOutputStream* output) const override;
    };

    class LVRepeatedBytesMessageValue : public LVMessageValue
    {
    public:
        LVRepeatedBytesMessageValue(int protobufId);

    public:
        google::protobuf::RepeatedPtrField<std::string> _value;

    public:
        void* RawValue() override { return &_value; };
        size_t ByteSizeLong() override;
        void Serialize(google::protobuf::io::CodedOutputStream* output) const override;
    };


    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class LVEnumMessageValue : public LVVariableMessageValue<int>
    {
    public:
        LVEnumMessageValue(int protobufId, int value);
    };


    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class LVRepeatedEnumMessageValue : public LVRepeatedMessageValue<int>
    {
    public:
        LVRepeatedEnumMessageValue(int protobufId);
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class LVSInt32MessageValue : public LVMessageValue
    {
    public:
        LVSInt32MessageValue(int protobufId, int32_t value);

    public:
        int32_t _value;

    public:
        void* RawValue() override { return &_value; };
        size_t ByteSizeLong() override;
        void Serialize(google::protobuf::io::CodedOutputStream* output) const override;
    };


    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class LVRepeatedSInt32MessageValue : public LVRepeatedMessageValue<int32_t>
    {
    public:
        LVRepeatedSInt32MessageValue(int protobufId);

        size_t ByteSizeLong() override;
        void Serialize(google::protobuf::io::CodedOutputStream* output) const override;
    };


    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class LVSInt64MessageValue : public LVMessageValue
    {
    public:
        LVSInt64MessageValue(int protobufId, int64_t value);

    public:
        int64_t _value;

    public:
        void* RawValue() override { return &_value; };
        size_t ByteSizeLong() override;
        void Serialize(google::protobuf::io::CodedOutputStream* output) const override;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class LVRepeatedSInt64MessageValue : public LVRepeatedMessageValue<int64_t>
    {
    public:
        LVRepeatedSInt64MessageValue(int protobufId);

        size_t ByteSizeLong() override;
        void Serialize(google::protobuf::io::CodedOutputStream* output) const override;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class LVFixed32MessageValue : public LVMessageValue
    {
    public:
        LVFixed32MessageValue(int protobufId, uint32_t value);

    public:
        uint32_t _value;

    public:
        void* RawValue() override { return &_value; };
        size_t ByteSizeLong() override;
        void Serialize(google::protobuf::io::CodedOutputStream* output) const override;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class LVRepeatedFixed32MessageValue : public LVMessageValue
    {
    public:
        LVRepeatedFixed32MessageValue(int protobufId);

    public:
        google::protobuf::RepeatedField<uint32_t> _value;

    public:
        void* RawValue() override { return &_value; };
        size_t ByteSizeLong() override;
        void Serialize(google::protobuf::io::CodedOutputStream* output) const override;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class LVFixed64MessageValue : public LVMessageValue
    {
    public:
        LVFixed64MessageValue(int protobufId, uint64_t value);

    public:
        uint64_t _value;

    public:
        void* RawValue() override { return &_value; };
        size_t ByteSizeLong() override;
        void Serialize(google::protobuf::io::CodedOutputStream* output) const override;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class LVRepeatedFixed64MessageValue : public LVMessageValue
    {
    public:
        LVRepeatedFixed64MessageValue(int protobufId);

    public:
        google::protobuf::RepeatedField<uint64_t> _value;

    public:
        void* RawValue() override { return &_value; };
        size_t ByteSizeLong() override;
        void Serialize(google::protobuf::io::CodedOutputStream* output) const override;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class LVSFixed32MessageValue : public LVMessageValue
    {
    public:
        LVSFixed32MessageValue(int protobufId, int32_t value);

    public:
        int32_t _value;

    public:
        void* RawValue() override { return &_value; };
        size_t ByteSizeLong() override;
        void Serialize(google::protobuf::io::CodedOutputStream* output) const override;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class LVRepeatedSFixed32MessageValue : public LVMessageValue
    {
    public:
        LVRepeatedSFixed32MessageValue(int protobufId);

    public:
        google::protobuf::RepeatedField<int32_t> _value;

    public:
        void* RawValue() override { return &_value; };
        size_t ByteSizeLong() override;
        void Serialize(google::protobuf::io::CodedOutputStream* output) const override;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class LVSFixed64MessageValue : public LVMessageValue
    {
    public:
        LVSFixed64MessageValue(int protobufId, int64_t value);

    public:
        int64_t _value;

    public:
        void* RawValue() override { return &_value; };
        size_t ByteSizeLong() override;
        void Serialize(google::protobuf::io::CodedOutputStream* output) const override;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class LVRepeatedSFixed64MessageValue : public LVMessageValue
    {
    public:
        LVRepeatedSFixed64MessageValue(int protobufId);

    public:
        google::protobuf::RepeatedField<int64_t> _value;

    public:
        void* RawValue() override { return &_value; };
        size_t ByteSizeLong() override;
        void Serialize(google::protobuf::io::CodedOutputStream* output) const override;
    };
}
