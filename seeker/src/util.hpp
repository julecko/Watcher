#pragma once

#include <string>
#include <vector>

namespace util {
	std::string base64_encode(const std::vector<char>& bytes);
	std::string base64_decode(const std::string& encoded);
}