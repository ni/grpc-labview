//---------------------------------------------------------------------
// Implementation of gRPC SerializationTraits for LVMessage
//---------------------------------------------------------------------
#include "lv_serialization_traits.h"
#include "lv_message.h"

namespace grpc {

//---------------------------------------------------------------------
// Serialize an LVMessage to a ByteBuffer
//---------------------------------------------------------------------
::grpc::Status SerializationTraits<grpc_labview::LVMessage, void>::Serialize(
    const grpc_labview::LVMessage& msg,
    ::grpc::ByteBuffer* bb,
    bool* own_buffer) 
{
    // Use the existing SerializeToByteBuffer method
    auto buffer = msg.SerializeToByteBuffer();
    if (!buffer) {
        return ::grpc::Status(::grpc::StatusCode::INTERNAL, "LVMessage serialization failed");
    }
    *bb = std::move(*buffer);
    *own_buffer = true;
    return ::grpc::Status::OK;
}

//---------------------------------------------------------------------
// Deserialize a ByteBuffer into an LVMessage
//---------------------------------------------------------------------
::grpc::Status SerializationTraits<grpc_labview::LVMessage, void>::Deserialize(
    ::grpc::ByteBuffer* bb,
    grpc_labview::LVMessage* msg) 
{
    if (!msg) {
        return ::grpc::Status(::grpc::StatusCode::INTERNAL, "Null message pointer");
    }
    
    if (!msg->ParseFromByteBuffer(*bb)) {
        return ::grpc::Status(::grpc::StatusCode::INTERNAL, "LVMessage deserialization failed");
    }
    
    bb->Clear();
    return ::grpc::Status::OK;
}

}  // namespace grpc
