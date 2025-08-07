#pragma once

#include <google/protobuf/wire_format_lite.h>
#include <string_view>

namespace grpc_labview
{
    void VerifyAsciiString(std::string_view str);

    void VerifyUtf8String(
        std::string_view str,
        google::protobuf::internal::WireFormatLite::Operation operation = google::protobuf::internal::WireFormatLite::Operation::PARSE,
        std::string_view field_name = std::string_view());
}