//---------------------------------------------------------------------
//---------------------------------------------------------------------
#include <lv_interop.h>
#include <google/protobuf/any.pb.h>
#include <lv_message.h>
#include <message_metadata.h>
#include <math.h>

namespace grpc_labview
{
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    class UnpackedFields : public gRPCid
    {
    public:
        UnpackedFields(LVMessage* message);    
        int32_t GetField(int protobufIndex, LVMessageMetadataType valueType, int isRepeated, int8_t* buffer);

    private:
        std::shared_ptr<LVMessage> _message;
    };

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    UnpackedFields::UnpackedFields(grpc_labview::LVMessage* message)
    {
        _message = std::shared_ptr<LVMessage>(message);
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void Copy64BitField(bool isRepeated, int typeCode, const google::protobuf::UnknownField* field, int8_t* buffer)
    {
        if (isRepeated)
        {
            auto destArray = (grpc_labview::LV1DArrayHandle*)buffer;
            auto value = field->length_delimited();
            auto count = value.size() / sizeof(double_t);
            if (count > 0)
            {
                grpc_labview::NumericArrayResize(typeCode, 1, destArray, count);
                (**destArray)->cnt = count;
                memcpy((**destArray)->bytes<double_t>(), value.c_str(), value.size());
            }
        }
        else
        {
            *(uint64_t*)buffer = field->fixed64();
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void CopyFixed32BitValueField(bool isRepeated, int typeCode, const google::protobuf::UnknownField* field, int8_t* buffer)
    {
        if (isRepeated)
        {
            auto destArray = (grpc_labview::LV1DArrayHandle*)buffer;
            auto value = field->length_delimited();
            auto count = value.size() / sizeof(uint32_t);
            grpc_labview::NumericArrayResize(typeCode, 1, destArray, count);
            (**destArray)->cnt = count;
            memcpy((**destArray)->bytes<uint32_t>(), value.c_str(), value.size());
        }
        else
        {
            *(uint32_t*)buffer = field->fixed32();
        }
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    int32_t UnpackedFields::GetField(int protobufIndex, LVMessageMetadataType valueType, int isRepeated, int8_t* buffer)
    {
        const google::protobuf::UnknownField* field = nullptr;
        for (int x = 0; x < _message->UnknownFields().field_count(); ++x)
        {
            field = &_message->UnknownFields().field(x);
            if (field->number() == protobufIndex)
            {
                break;
            }
        }
        if (field == nullptr)
        {
            return -1;
        }
        if (field->number() != protobufIndex)
        {
            return -1;
        }
        switch (valueType)
        {
            case LVMessageMetadataType::MessageValue:
                return -1;
            case LVMessageMetadataType::BoolValue:
                *(uint8_t*)buffer = field->varint();
                break;
            case LVMessageMetadataType::EnumValue:
            case LVMessageMetadataType::Int32Value:
            case LVMessageMetadataType::UInt32Value:
            case LVMessageMetadataType::SInt32Value:
                *(uint32_t*)buffer = field->varint();
                break;
            case LVMessageMetadataType::Int64Value:
            case LVMessageMetadataType::UInt64Value:
            case LVMessageMetadataType::SInt64Value:
                *(uint64_t*)buffer = field->varint();
                break;
            case LVMessageMetadataType::FloatValue:
                CopyFixed32BitValueField(isRepeated, 0x09, field, buffer);
                break;
            case LVMessageMetadataType::Fixed32Value:
            case LVMessageMetadataType::SFixed32Value:
                CopyFixed32BitValueField(isRepeated, 0x03, field, buffer);
                break;
            case LVMessageMetadataType::DoubleValue:
                Copy64BitField(isRepeated, 0x0A, field, buffer);
                break;
            case LVMessageMetadataType::Fixed64Value:
            case LVMessageMetadataType::SFixed64Value:
                Copy64BitField(isRepeated, 0x04, field, buffer);
                break;
            case LVMessageMetadataType::StringValue:
                if (isRepeated)
                {
                }
                else
                {
                    SetLVString((LStrHandle*)buffer, field->length_delimited());
                }
                break;
            case LVMessageMetadataType::BytesValue:
                if (isRepeated)
                {
                }
                else
                {
                    SetLVString((LStrHandle*)buffer, field->length_delimited());
                }
                break;
        }    
        return 0; 
    }
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t UnpackFieldsFromBuffer(grpc_labview::LV1DArrayHandle lvBuffer, grpc_labview::gRPCid** unpackedFieldsRef)
{
    char* elements = (*lvBuffer)->bytes<char>();
    std::string buffer(elements, (*lvBuffer)->cnt);

    auto message = new grpc_labview::LVMessage(nullptr);
    if (message->ParseFromString(buffer))
    {
        auto fields = new grpc_labview::UnpackedFields(message);
        grpc_labview::gClientTokenManager.RegisterPointer(fields);
        *unpackedFieldsRef = fields;
        return 0;
    }
    return -2;
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t UnpackFieldsFromAny(grpc_labview::AnyCluster* anyCluster, grpc_labview::gRPCid** unpackedFieldsRef)
{
    return UnpackFieldsFromBuffer(anyCluster->Bytes, unpackedFieldsRef);
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t GetUnpackedField(grpc_labview::gRPCid* id, int protobufIndex, grpc_labview::LVMessageMetadataType valueType, int isRepeated, int8_t* buffer)
{
    if (id == nullptr)
    {
        return -1;
    }
    auto unpackedFields = id->CastTo<grpc_labview::UnpackedFields>();
    unpackedFields->GetField(protobufIndex, valueType, isRepeated, buffer);
    return 0; 
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t GetUnpackedMessageField(grpc_labview::gRPCid* id, int protobufIndex, int8_t* buffer)
{    
    return -1; 
}

//---------------------------------------------------------------------
//---------------------------------------------------------------------
LIBRARY_EXPORT int32_t FreeUnpackedFields(grpc_labview::gRPCid* id)
{
    grpc_labview::gClientTokenManager.UnregisterPointer(id);
    return 0; 
}
