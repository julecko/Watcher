#include "./src/websocket_client.hpp"
#include "./src/user_info.hpp"

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
    WebSocketClient client("localhost", 8080, "/ws/seeker");

    client.setMessageCallback([&client](const std::string& message, size_t len) {
        std::cout << "Received from server: " << message << std::endl;
        nlohmann::json json = nlohmann::json::parse(message);
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