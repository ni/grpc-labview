#include "string_utils.h"

#include <feature_toggles.h>
#include <iostream>
#include <stdexcept>

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

    // Returns true if str is valid UTF-8.
    static bool IsValidUtf8(std::string_view str)
    {
        const unsigned char* s = reinterpret_cast<const unsigned char*>(str.data());
        const unsigned char* end = s + str.size();
        while (s < end) {
            unsigned char c = *s++;
            int extra = 0;
            if (c < 0x80) {
                continue;  // ASCII
            } else if ((c & 0xE0) == 0xC0) {
                extra = 1;
                if ((c & 0xFE) == 0xC0) return false;  // overlong
            } else if ((c & 0xF0) == 0xE0) {
                extra = 2;
            } else if ((c & 0xF8) == 0xF0) {
                extra = 3;
                if (c > 0xF4) return false;  // beyond U+10FFFF
            } else {
                return false;  // invalid lead byte
            }
            if (s + extra > end) return false;
            while (extra-- > 0) {
                if ((*s++ & 0xC0) != 0x80) return false;
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

        if (!IsValidUtf8(str)) {
#ifndef NDEBUG
            const char* safe_field_name = field_name ? field_name : "";
            std::cerr << "ERROR: String field '" << safe_field_name << "' contains invalid UTF-8.";
#endif
            return false;
        }
        return true;
    }
}