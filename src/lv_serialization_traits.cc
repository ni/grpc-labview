//---------------------------------------------------------------------
// Implementation of gRPC SerializationTraits for LVMessage
//---------------------------------------------------------------------
#include "lv_serialization_traits.h"
#include "lv_message.h"

namespace grpc {

//---------------------------------------------------------------------
// Serialize an LVMessage to a ByteBuffer
//---------------------------------------------------------------------
Status SerializationTraits<grpc_labview::LVMessage, void>::Serialize(
    const grpc_labview::LVMessage& msg,
    ByteBuffer* bb,
    bool* own_buffer) 
{
    // Use the existing SerializeToByteBuffer method
    auto buffer = msg.SerializeToByteBuffer();
    if (!buffer) {
        return Status(StatusCode::INTERNAL, "LVMessage serialization failed");
    }
    *bb = std::move(*buffer);
    *own_buffer = true;
    return Status::OK;
}

//---------------------------------------------------------------------
// Deserialize a ByteBuffer into an LVMessage
//---------------------------------------------------------------------
Status SerializationTraits<grpc_labview::LVMessage, void>::Deserialize(
    ByteBuffer* bb,
    grpc_labview::LVMessage* msg) 
{
    if (!msg) {
        return Status(StatusCode::INTERNAL, "Null message pointer");
    }
    
    if (!msg->ParseFromByteBuffer(*bb)) {
        return Status(StatusCode::INTERNAL, "LVMessage deserialization failed");
    }
    
    bb->Clear();
    return Status::OK;
}

}  // namespace grpc
