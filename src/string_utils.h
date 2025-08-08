#pragma once

#include <google/protobuf/wire_format_lite.h>
#include <string_view>

namespace grpc_labview
{
    // Returns true if valid. Logs and returns false if invalid.
    // Disabled when verifyStringEncoding feature toggle is false.
    bool VerifyAsciiString(std::string_view str);

    // Returns true if valid. Logs and returns false if invalid.
    // Disabled when verifyStringEncoding feature toggle is false.
    bool VerifyUtf8String(
        std::string_view str,
        google::protobuf::internal::WireFormatLite::Operation operation = google::protobuf::internal::WireFormatLite::PARSE,
        const char* field_name = nullptr);
}