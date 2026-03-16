//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <message_value.h>
#include <message_metadata.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/unknown_field_set.h>
#include <grpcpp/support/byte_buffer.h>

namespace grpc_labview
{
    //---------------------------------------------------------------------
    // LVMessage - Custom message class for LabVIEW gRPC integration
    // 
    // This class does NOT inherit from google::protobuf::Message.
    // gRPC integration is provided via grpc::SerializationTraits
    // specialization in lv_serialization_traits.h.
    //---------------------------------------------------------------------
    class LVMessage : public gRPCid
    {
    public:
        LVMessage(std::shared_ptr<MessageMetadata> metadata);
        ~LVMessage();

        void Clear();
        bool IsInitialized() const;
        size_t ByteSizeLong() const;
        virtual void PostInteralParseAction() {};

        google::protobuf::UnknownFieldSet& UnknownFields();
        void CopyOneofIndicesToCluster(int8_t* cluster) const;

        //---------------------------------------------------------------------
        // ByteBuffer serialization methods - used by SerializationTraits
        //---------------------------------------------------------------------
        bool ParseFromByteBuffer(const grpc::ByteBuffer& buffer);
        bool ParseFromString(const std::string& data);
        bool ParseFromCodedStream(google::protobuf::io::CodedInputStream* input);
        std::unique_ptr<grpc::ByteBuffer> SerializeToByteBuffer() const;
        bool SerializeToString(std::string* output) const;
        void SerializeToCodedStream(google::protobuf::io::CodedOutputStream* output) const;

        std::map<int, std::shared_ptr<LVMessageValue>> _values;
        std::shared_ptr<MessageMetadata> _metadata;
        std::map<std::string, int> _oneofContainerToSelectedIndexMap;

    protected:
        google::protobuf::UnknownFieldSet _unknownFields;

        // Field parse helpers - virtual so LVMessageEfficient can override for direct-to-cluster writes
        virtual bool ParseFieldFromCodedStream(google::protobuf::io::CodedInputStream* input, uint32_t tag, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo);
        virtual bool ParseInt32Field(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo, uint32_t wireType);
        virtual bool ParseInt64Field(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo, uint32_t wireType);
        virtual bool ParseUInt32Field(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo, uint32_t wireType);
        virtual bool ParseUInt64Field(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo, uint32_t wireType);
        virtual bool ParseSInt32Field(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo, uint32_t wireType);
        virtual bool ParseSInt64Field(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo, uint32_t wireType);
        virtual bool ParseFixed32Field(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo, uint32_t wireType);
        virtual bool ParseFixed64Field(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo, uint32_t wireType);
        virtual bool ParseSFixed32Field(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo, uint32_t wireType);
        virtual bool ParseSFixed64Field(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo, uint32_t wireType);
        virtual bool ParseFloatField(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo, uint32_t wireType);
        virtual bool ParseDoubleField(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo, uint32_t wireType);
        virtual bool ParseBoolField(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo, uint32_t wireType);
        virtual bool ParseEnumField(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo, uint32_t wireType);
        virtual bool ParseStringField(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo);
        virtual bool ParseBytesField(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo);
        virtual bool ParseMessageField(google::protobuf::io::CodedInputStream* input, uint32_t fieldNumber, const MessageElementMetadata& fieldInfo);
    };
}
