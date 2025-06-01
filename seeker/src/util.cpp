#include "util.hpp"

static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

namespace util {
     std::string base64_decode(const std::string& encoded) {
        std::vector<char> decoded;
        int val = 0, valb = -8;
        for (unsigned char c : encoded) {
            if (c == '=') break;
            if (base64_chars.find(c) == std::string::npos) continue;
            val = (static_cast<unsigned long long>(val) << 6) + base64_chars.find(c);
            valb += 6;
            if (valb >= 0) {
                decoded.push_back(char((val >> valb) & 0xFF));
                valb -= 8;
            }
        }
        return std::string(decoded.begin(), decoded.end());
    }

    std::string base64_encode(const std::vector<char>& bytes) {
        std::string encoded;
        int val = 0, valb = -6;
        for (unsigned char c : bytes) {
            val = (val << 8) + c;
            valb += 8;
            while (valb >= 0) {
                encoded.push_back(base64_chars[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6) {
            encoded.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
        }
        while (encoded.size() % 4) {
            encoded.push_back('=');
        }
        return encoded;
    }
}