#include "websocket_client.hpp"
#include <iostream>
#include <cstring>
#include <vector>

WebSocketClient* WebSocketClient::instance_ = nullptr;

static struct lws_protocols protocols[] = {
    { "echo-protocol", WebSocketClient::callback_client, 0, 0, 0, nullptr, 0 },
    { nullptr, nullptr, 0, 0, 0, nullptr, 0 }
};

WebSocketClient::WebSocketClient(const std::string& address, int port, const std::string& path)
    : address_(address), port_(port), path_(path), context_(nullptr), client_wsi_(nullptr),
    interrupted_(false) {
    instance_ = this;
}

WebSocketClient::~WebSocketClient() {
    stop();
    if (context_) {
        lws_context_destroy(context_);
        context_ = nullptr;
    }
    instance_ = nullptr;
}

bool WebSocketClient::initialize() {
    struct lws_context_creation_info info;
    memset(&info, 0, sizeof(info));
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols;
    info.gid = -1;
    info.uid = -1;

    info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;

    context_ = lws_create_context(&info);
    if (!context_) {
        std::cerr << "Failed to create libwebsockets context" << std::endl;
        return false;
    }
    return true;
}

bool WebSocketClient::connect() {
    if (!context_) {
        std::cerr << "Context not initialized" << std::endl;
        return false;
    }

    struct lws_client_connect_info connect_info;
    memset(&connect_info, 0, sizeof(connect_info));
    connect_info.context = context_;
    connect_info.address = address_.c_str();
    connect_info.port = port_;
    connect_info.path = path_.c_str();
    connect_info.host = address_.c_str();
    connect_info.origin = address_.c_str();
    connect_info.protocol = protocols[0].name;

    connect_info.ssl_connection = LCCSCF_USE_SSL;

    client_wsi_ = lws_client_connect_via_info(&connect_info);
    if (!client_wsi_) {
        std::cerr << "Failed to connect to server" << std::endl;
        return false;
    }

    std::cout << "Connecting to server..." << std::endl;
    return true;
}

void WebSocketClient::run() {
    while (!interrupted_) {
        lws_service(context_, 1000);
    }
}

bool WebSocketClient::sendMessage(const std::string& message) {
    if (!client_wsi_) {
        std::cerr << "No WebSocket connection" << std::endl;
        return false;
    }

    size_t buffer_size = message.length() + LWS_PRE;
    std::vector<unsigned char> buffer(buffer_size);
    memcpy(buffer.data() + LWS_PRE, message.c_str(), message.length());

    int written = lws_write(client_wsi_, buffer.data() + LWS_PRE, message.length(), LWS_WRITE_TEXT);
    if (written < 0 || static_cast<size_t>(written) != message.length()) {
        std::cerr << "Failed to send message" << std::endl;
        return false;
    }
    return true;
}

void WebSocketClient::setMessageCallback(MessageCallback callback) {
    message_callback_ = callback;
}

void WebSocketClient::setEventCallback(EventCallback callback) {
    event_callback_ = callback;
}

void WebSocketClient::stop() {
    if (interrupted_) {
        return;
    }
    interrupted_ = true;
    if (context_ && client_wsi_) {
        lws_cancel_service(context_);
    }
}

int WebSocketClient::callback_client(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len) {
    if (!instance_) {
        std::cerr << "Callback: instance_ is null" << std::endl;
        return -1;
    }

    switch (reason) {
    case LWS_CALLBACK_CLIENT_ESTABLISHED:
        std::cout << "Callback: LWS_CALLBACK_CLIENT_ESTABLISHED" << std::endl;
        if (instance_->event_callback_) {
            instance_->event_callback_(reason);
        }
        break;
    case LWS_CALLBACK_CLIENT_RECEIVE:
    {
        std::cout << "Callback: LWS_CALLBACK_CLIENT_RECEIVE, len=" << len << std::endl;
        if (!in) {
            std::cerr << "Received empty or null message" << std::endl;
            return 0;
        }

        std::lock_guard<std::mutex> lock(instance_->buffer_mutex_);

        instance_->message_buffer_.append(static_cast<char*>(in), len);

        if (lws_is_final_fragment(wsi)) {
            if (instance_->message_callback_) {
                instance_->message_callback_(instance_->message_buffer_, instance_->message_buffer_.length());
            }
            instance_->message_buffer_.clear();
        }
        else {
            std::cout << "Received partial message, waiting for final fragment" << std::endl;
        }
    }
    break;
    case LWS_CALLBACK_CLIENT_CLOSED:
        std::cout << "Callback: LWS_CALLBACK_CLIENT_CLOSED" << std::endl;
        if (instance_->event_callback_) {
            instance_->event_callback_(reason);
        }
        instance_->stop();
        break;
    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
        std::cout << "Callback: LWS_CALLBACK_CLIENT_CONNECTION_ERROR, message=" << (in ? (char*)in : "none") << std::endl;
        instance_->stop();
        break;
    default:
        break;
    }
    return 0;
}