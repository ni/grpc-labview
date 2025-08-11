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

    bool VerifyAsciiString(std::string_view str)
    {
        if (!FeatureConfig::getInstance().IsVerifyStringEncodingEnabled()) {
            return true;
        }

        if (!IsAscii(str)) {
#ifndef NDEBUG
            std::cerr << "ERROR: String contains non-ASCII characters.";
#endif
            return false;
        }
        return true;
    }

    bool VerifyUtf8String(std::string_view str, WireFormatLite::Operation operation, const char* field_name)
    {
        if (!FeatureConfig::getInstance().IsVerifyStringEncodingEnabled()) {
            return true;
        }

        // WireFormatLite::VerifyUtf8String logs the failure.
        return WireFormatLite::VerifyUtf8String(str.data(), str.size(), operation, field_name);
    }
}