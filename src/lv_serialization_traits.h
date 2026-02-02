//---------------------------------------------------------------------
// Custom gRPC SerializationTraits for LVMessage
// 
// This header provides a custom SerializationTraits specialization that
// allows gRPC to serialize/deserialize LVMessage without requiring
// inheritance from google::protobuf::Message.
//
// IMPORTANT: This header MUST be included BEFORE any gRPC headers
// in files that use LVMessage with gRPC calls.
//---------------------------------------------------------------------
#pragma once

#include <grpcpp/support/byte_buffer.h>
#include <grpcpp/support/status.h>
#include <grpcpp/impl/codegen/serialization_traits.h>

// Forward declaration - full definition in lv_message.h
namespace grpc_labview {
    class LVMessage;
}

namespace grpc {

//---------------------------------------------------------------------
// SerializationTraits specialization for LVMessage
// This tells gRPC how to serialize/deserialize our custom message type
// without requiring inheritance from google::protobuf::Message
//---------------------------------------------------------------------
template <>
class SerializationTraits<grpc_labview::LVMessage, void> {
public:
    //---------------------------------------------------------------------
    // Serialize an LVMessage to a ByteBuffer for sending over the wire
    //---------------------------------------------------------------------
    static Status Serialize(const grpc_labview::LVMessage& msg,
                           ByteBuffer* bb,
                           bool* own_buffer);

    //---------------------------------------------------------------------
    // Deserialize a ByteBuffer into an LVMessage
    //---------------------------------------------------------------------
    static Status Deserialize(ByteBuffer* bb,
                             grpc_labview::LVMessage* msg);
};

}  // namespace grpc
