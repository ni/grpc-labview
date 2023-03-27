//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

namespace grpc_labview
{
    class InvalidEnumValueException : public std::exception {
    private:
        char* message;

    public:
        InvalidEnumValueException(char* msg) : message(msg) {}
        char* what()
        {
            return message;
        }
        int code = -2003; // Essentially, -(1000 + grpc::StatusCode::INVALID_ARGUMENT);, but 2000 instead of 1000 because it's not a gRPC eror, more of an LV error.
    };
}