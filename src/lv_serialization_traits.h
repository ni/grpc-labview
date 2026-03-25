//---------------------------------------------------------------------
// Custom gRPC SerializationTraits for LVMessage
// 
// This header provides a custom SerializationTraits specialization that
// allows gRPC to serialize/deserialize LVMessage without requiring
// inheritance from google::protobuf::Message.
//
// IMPORTANT: In translation units that use LVMessage as a gRPC RPC payload,
// include this header before including other gRPC headers.
//---------------------------------------------------------------------
#pragma once

// Required gRPC headers for SerializationTraits implementation
#include <grpcpp/support/byte_buffer.h>
#include <grpcpp/support/status.h>

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
    static ::grpc::Status Serialize(const grpc_labview::LVMessage& msg,
                                    ::grpc::ByteBuffer* bb,
                                    bool* own_buffer);

    //---------------------------------------------------------------------
    // Deserialize a ByteBuffer into an LVMessage
    //---------------------------------------------------------------------
    static ::grpc::Status Deserialize(::grpc::ByteBuffer* bb,
                                      grpc_labview::LVMessage* msg);
};

}  // namespace grpc
