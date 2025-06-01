#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace file_transfer {
	std::string save_file(const nlohmann::json& file_data);

	std::string send_file(const std::string& file_path);
}