#include "file_transfer.hpp"
#include "util.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <vector>
#include <stdexcept>

namespace file_transfer {
    std::string save_file(const nlohmann::json& file_data) {
        try {
            std::string filename = file_data.value("filename", "");
            std::string content = file_data.value("content", "");
            std::string path = file_data.value("path", "");

            if (filename.empty() || content.empty()) {
                return "[Error] Missing filename or content";
            }

            std::string file_content = util::base64_decode(content);

            std::filesystem::path save_path;
            if (path.ends_with(std::filesystem::path::preferred_separator) ||
                std::filesystem::is_directory(path)) {
                save_path = std::filesystem::path(path) / filename;
            }
            else {
                std::filesystem::path path_obj(path);
                if (path_obj.filename().string() != filename) {
                    save_path = path_obj / filename;
                }
                else {
                    save_path = path_obj;
                }
            }

            save_path = std::filesystem::absolute(save_path);

            std::filesystem::create_directories(save_path.parent_path());

            std::ofstream file(save_path, std::ios::binary);
            if (!file) {
                return "[Error] Failed to open file for writing: " + save_path.string();
            }
            file.write(file_content.data(), file_content.size());
            file.close();

            return "[Success] File " + filename + " saved to " + save_path.string();
        }
        catch (const std::exception& e) {
            return "[Error] Failed to save file: " + std::string(e.what());
        }
    }

    std::string send_file(const std::string& file_path) {
        try {
            std::filesystem::path abs_path = std::filesystem::absolute(file_path);
            if (!std::filesystem::exists(abs_path)) {
                return "[Error] File not found: " + abs_path.string();
            }

            std::ifstream file(abs_path, std::ios::binary);
            if (!file) {
                return "[Error] Failed to open file: " + abs_path.string();
            }

            std::vector<char> content((std::istreambuf_iterator<char>(file)),
                std::istreambuf_iterator<char>());
            file.close();

            std::string base64_content = util::base64_encode(content);

            nlohmann::json response;
            response["filename"] = abs_path.filename().string();
            response["content"] = base64_content;

            return response.dump();
        }
        catch (const std::exception& e) {
            return "[Error] Failed to send file: " + std::string(e.what());
        }
    }

}