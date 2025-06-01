#pragma once

#include <libwebsockets.h>
#include <string>
#include <functional>

class WebSocketClient {
public:
    using MessageCallback = std::function<void(const std::string& message, size_t len)>;
    using EventCallback = std::function<void(enum lws_callback_reasons reason)>;

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
    struct lws_context* context_;
    struct lws* client_wsi_;
    std::string address_;
    int port_;
    std::string path_;
    MessageCallback message_callback_;
    EventCallback event_callback_;
    bool interrupted_;
    static WebSocketClient* instance_;
};