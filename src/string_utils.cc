#include "string_utils.h"

#include <feature_toggles.h>
#include <iostream>
#include <stdexcept>
#include "third_party/utf8_range/utf8_validity.h"

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

    bool VerifyUtf8String(std::string_view str, const char* field_name)
    {
        if (!FeatureConfig::getInstance().IsVerifyStringEncodingEnabled()) {
            return true;
        }

        if (!utf8_range::IsStructurallyValid(str)) {
#ifndef NDEBUG
            const char* safe_field_name = field_name ? field_name : "";
            std::cerr << "ERROR: String field '" << safe_field_name << "' contains invalid UTF-8.";
#endif
            return false;
        }
        return true;
    }
}