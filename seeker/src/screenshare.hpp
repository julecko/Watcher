#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include <thread>
#include <atomic>

class WebSocketClient;

class ScreenShare {
public:
    explicit ScreenShare(WebSocketClient& ws_client);
    ~ScreenShare();

    void start_screenshare();

    void stop_screenshare();

private:
    std::atomic<bool> running_;
    std::shared_ptr<std::thread> thread_;
    WebSocketClient& ws_client_;

    std::string capture_frame();

    void screenshare_loop();
};