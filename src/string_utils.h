#pragma once

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
        const char* field_name = nullptr);
}