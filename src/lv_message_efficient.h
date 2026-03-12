//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <message_value.h>
#include <message_metadata.h>
// Note: google/protobuf/message.h kept for RepeatedPtrField<google::protobuf::Message> used in RepeatedMessageValue
#include <google/protobuf/message.h>
#include <google/protobuf/io/coded_stream.h>
#include <lv_message.h>
#include <well_known_messages.h>

namespace grpc_labview
{
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class LVMessageEfficient : public LVMessage
    {
    public:
        LVMessageEfficient(std::shared_ptr<MessageMetadata> metadata, int8_t* cluster) : LVMessage(metadata), _LVClusterHandle(cluster) {}
        ~LVMessageEfficient() {}

        void PostInteralParseAction() override;
        int8_t* GetLVClusterHandle() { return _LVClusterHandle; };

    protected:
        struct RepeatedMessageValue {
            const MessageElementMetadata& _fieldInfo;
            google::protobuf::RepeatedPtrField<google::protobuf::Message> _buffer;
            uint64_t _numElements = 0;

            RepeatedMessageValue(const MessageElementMetadata& fieldInfo, google::protobuf::RepeatedPtrField<google::protobuf::Message> buffer) :
                _fieldInfo(fieldInfo), _buffer(buffer) {}
        };

        struct RepeatedStringValue {
            const MessageElementMetadata& _fieldInfo;
            google::protobuf::RepeatedPtrField<std::string> _repeatedString;

            RepeatedStringValue(const MessageElementMetadata& fieldInfo) :
                _fieldInfo(fieldInfo), _repeatedString(google::protobuf::RepeatedPtrField<std::string>()) {}
        };

        struct RepeatedBytesValue {
            const MessageElementMetadata& _fieldInfo;
            google::protobuf::RepeatedPtrField<std::string> _repeatedBytes;

            RepeatedBytesValue(const MessageElementMetadata& fieldInfo) :
                _fieldInfo(fieldInfo), _repeatedBytes(google::protobuf::RepeatedPtrField<std::string>()) {}
        };

    public:
        std::unordered_map<std::string, std::shared_ptr<RepeatedMessageValue>> _repeatedMessageValuesMap;
        std::unordered_map<std::string, std::shared_ptr<RepeatedStringValue>> _repeatedStringValuesMap;
        std::unordered_map<std::string, std::shared_ptr<RepeatedBytesValue>> _repeatedBytesValuesMap;

    protected:
        int8_t* _LVClusterHandle;

        // Override ParseXxxField methods to write directly to LV cluster memory
        bool ParseInt32Field(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo, uint32_t wire_type) override;
        bool ParseUInt32Field(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo, uint32_t wire_type) override;
        bool ParseInt64Field(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo, uint32_t wire_type) override;
        bool ParseUInt64Field(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo, uint32_t wire_type) override;
        bool ParseBoolField(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo, uint32_t wire_type) override;
        bool ParseFloatField(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo, uint32_t wire_type) override;
        bool ParseDoubleField(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo, uint32_t wire_type) override;
        bool ParseSInt32Field(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo, uint32_t wire_type) override;
        bool ParseSInt64Field(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo, uint32_t wire_type) override;
        bool ParseFixed32Field(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo, uint32_t wire_type) override;
        bool ParseFixed64Field(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo, uint32_t wire_type) override;
        bool ParseSFixed32Field(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo, uint32_t wire_type) override;
        bool ParseSFixed64Field(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo, uint32_t wire_type) override;
        bool ParseEnumField(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo, uint32_t wire_type) override;
        bool ParseStringField(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo) override;
        bool ParseBytesField(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo) override;
        bool ParseMessageField(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo) override;

    private:
        bool ParseDouble2DArrayField(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo);
        bool ParseString2DArrayField(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo);
        bool Parse2DArrayField(google::protobuf::io::CodedInputStream* input, uint32_t field_number, const MessageElementMetadata& fieldInfo, wellknown::I2DArray& array);
    };
}