//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <grpc_server.h>

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

    public:
        int _protobufId;

    public:
        virtual void* RawValue() = 0;
        virtual size_t ByteSizeLong() = 0;
        virtual google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const = 0;
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

        void* RawValue() override { return &_value; };
        size_t ByteSizeLong() override;
        google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;

    protected:
        int _cachedSize;
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
        google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;
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
    class LVRepeatedNestedMessageMessageValue : public LVMessageValue, public gRPCid
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

    class LVRepeatedStringMessageValue : public LVMessageValue
    {
    public:
        LVRepeatedStringMessageValue(int protobufId);

    public:
        google::protobuf::RepeatedPtrField<std::string> _value;

    public:
        void* RawValue() override { return &_value; };
        size_t ByteSizeLong() override;
        google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;
    };


    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class LVEnumMessageValue : public LVMessageValue
    {
    public:
        LVEnumMessageValue(int protobufId, int _value);

    public:
        int _value;

        void* RawValue() override { return &_value; };
        size_t ByteSizeLong() override;
        google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;
    };


    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class LVRepeatedEnumMessageValue : public LVRepeatedMessageValue<int>
    {
        public:
            LVRepeatedEnumMessageValue(int protobufId);

            google::protobuf::RepeatedField<int> _value;

            void* RawValue() override { return &_value; };
            size_t ByteSizeLong() override;
            google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;
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
        google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;
    };


    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class LVRepeatedSInt32MessageValue : public LVMessageValue
    {
    public:
        LVRepeatedSInt32MessageValue(int protobufId);

    public:
        google::protobuf::RepeatedField<int32_t> _value;

    public:
        void* RawValue() override { return &_value; };
        size_t ByteSizeLong() override;
        google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;

    private:
        int _cachedSize;
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
        google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class LVRepeatedSInt64MessageValue : public LVMessageValue
    {
    public:
        LVRepeatedSInt64MessageValue(int protobufId);

    public:
        google::protobuf::RepeatedField<int64_t> _value;

    public:
        void* RawValue() override { return &_value; };
        size_t ByteSizeLong() override;
        google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;

    private:
        int _cachedSize;
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
        google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;
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
        google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;
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
        google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;
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
        google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;
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
        google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;
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
        google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;
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
        google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;
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
        google::protobuf::uint8* Serialize(google::protobuf::uint8* target, google::protobuf::io::EpsCopyOutputStream* stream) const override;
    };
}
