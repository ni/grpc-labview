//---------------------------------------------------------------------
//---------------------------------------------------------------------
#pragma once

#include <cstdint>
#include <grpcpp/support/status.h>
#include <lv_interop.h>
#include <stdexcept>
#include <string>

namespace grpc_labview
{
    class GrpcException : public std::runtime_error {
    public:
        GrpcException(grpc::StatusCode code, const std::string& message) : std::runtime_error(message), _code(code) {}
        GrpcException(grpc::StatusCode code, const char* message) : std::runtime_error(message), _code(code) {}
        GrpcException(const GrpcException& other) : std::runtime_error(other.what()), _code(other._code) {}

        grpc::Status GetStatus() const { return grpc::Status(_code, what()); }
        grpc::StatusCode GetStatusCode() const { return _code; }

    private:
        grpc::StatusCode _code;
    };

    class LabVIEWException : public std::runtime_error {
    public:
        LabVIEWException(int32_t code, const std::string& message) : std::runtime_error(message), _code(code) {}
        LabVIEWException(int32_t code, const char* message) : std::runtime_error(message), _code(code) {}
        LabVIEWException(const LabVIEWException& other) : std::runtime_error(other.what()), _code(other._code) {}

        int32_t GetCode() const { return _code; }

    private:
        int32_t _code;
    };

    class InvalidEnumValueException : public LabVIEWException {
    public:
        InvalidEnumValueException(const char* message) : LabVIEWException(-2003, message) {}
    };

    int32_t TranslateException();

    void SetErrorMessage(LStrHandle* errorMessageOut, const char* message);
}
