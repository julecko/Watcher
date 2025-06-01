#pragma once

#include <libwebsockets.h>
#include <functional>
#include <string>
#include <mutex>

class WebSocketClient {
public:
    using MessageCallback = std::function<void(const std::string&, size_t)>;
    using EventCallback = std::function<void(enum lws_callback_reasons)>;

    WebSocketClient(const std::string& address, int port, const std::string& path);
    ~WebSocketClient();

    bool initialize();
    bool connect();
    void run();
    bool sendMessage(const std::string& message);
    void setMessageCallback(MessageCallback callback);
    void setEventCallback(EventCallback callback);
    void stop();

    static int callback_client(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len);

private:
    static WebSocketClient* instance_;
    std::string address_;
    int port_;
    std::string path_;
    struct lws_context* context_;
    struct lws* client_wsi_;
    bool interrupted_;
    MessageCallback message_callback_;
    EventCallback event_callback_;
    std::string message_buffer_;
    std::mutex buffer_mutex_;
};