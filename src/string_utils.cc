#include "string_utils.h"

#include <feature_toggles.h>
#include <iostream>
#include <stdexcept>

using google::protobuf::internal::WireFormatLite;

namespace grpc_labview
{
    static bool IsAscii(std::string_view str)
    {
        for (auto c : str) {
            auto ch = static_cast<int>(c);
            if (ch < 0 || ch > 127) {
                return false;
            }
        }
        return true;
    }

    void VerifyAsciiString(std::string_view str)
    {
        if (!FeatureConfig::getInstance().IsVerifyStringEncodingEnabled()) {
            return;
        }

        if (!IsAscii(str)) {
            throw std::runtime_error("String contains non-ASCII characters.");
        }
    }

    void VerifyUtf8String(std::string_view str, WireFormatLite::Operation operation, std::string_view field_name);
    {
        if (!FeatureConfig::getInstance().IsVerifyStringEncodingEnabled()) {
            return;
        }

        // WireFormatLite::VerifyUtf8String logs the operation and field name.
        if (!WireFormatLite::VerifyUtf8String(str.data(), str.size(), operation, field_name)) {
            throw std::runtime_error("String contains invalid UTF-8 data.");
        }
    }
}