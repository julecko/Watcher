#define _CRT_SECURE_NO_WARNINGS
#include "screenshare.hpp"
#include "websocket_client.hpp"
#include "util.hpp"
#include <windows.h> // Suppress sprintf warning
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <nlohmann/json.hpp>
#include <vector>
#include <stdexcept>
#include <chrono>
#include <thread>

ScreenShare::ScreenShare(WebSocketClient& ws_client)
    : running_(false), thread_(nullptr), ws_client_(ws_client) {
}

ScreenShare::~ScreenShare() {
    stop_screenshare();
}

std::string ScreenShare::capture_frame() {
    try {
        // Get desktop window and DC
        HWND desktop = GetDesktopWindow();
        HDC screen_dc = GetDC(desktop);
        if (!screen_dc) throw std::runtime_error("Failed to get desktop DC");

        // Get screen dimensions
        int width = GetSystemMetrics(SM_CXSCREEN);
        int height = GetSystemMetrics(SM_CYSCREEN);

        // Resize to a lower resolution (e.g., 960x540)
        int target_width = 960;  // Fixed resolution
        int target_height = 540;

        // Create compatible DC and bitmap for original size
        HDC mem_dc = CreateCompatibleDC(screen_dc);
        HBITMAP bitmap = CreateCompatibleBitmap(screen_dc, width, height);
        if (!mem_dc || !bitmap) {
            ReleaseDC(desktop, screen_dc);
            if (mem_dc) DeleteDC(mem_dc);
            if (bitmap) DeleteObject(bitmap);
            throw std::runtime_error("Failed to create DC or bitmap");
        }

        // Select bitmap into memory DC
        HGDIOBJ old_bitmap = SelectObject(mem_dc, bitmap);

        // Copy screen to bitmap
        if (!BitBlt(mem_dc, 0, 0, width, height, screen_dc, 0, 0, SRCCOPY)) {
            SelectObject(mem_dc, old_bitmap);
            DeleteDC(mem_dc);
            DeleteObject(bitmap);
            ReleaseDC(desktop, screen_dc);
            throw std::runtime_error("BitBlt failed");
        }

        // Create a resized bitmap
        HDC resize_dc = CreateCompatibleDC(screen_dc);
        HBITMAP resize_bitmap = CreateCompatibleBitmap(screen_dc, target_width, target_height);
        if (!resize_dc || !resize_bitmap) {
            SelectObject(mem_dc, old_bitmap);
            DeleteDC(mem_dc);
            DeleteObject(bitmap);
            if (resize_dc) DeleteDC(resize_dc);
            if (resize_bitmap) DeleteObject(resize_bitmap);
            ReleaseDC(desktop, screen_dc);
            throw std::runtime_error("Failed to create resize DC or bitmap");
        }

        HGDIOBJ old_resize_bitmap = SelectObject(resize_dc, resize_bitmap);
        SetStretchBltMode(resize_dc, HALFTONE);
        if (!StretchBlt(resize_dc, 0, 0, target_width, target_height, mem_dc, 0, 0, width, height, SRCCOPY)) {
            SelectObject(resize_dc, old_resize_bitmap);
            SelectObject(mem_dc, old_bitmap);
            DeleteDC(mem_dc);
            DeleteDC(resize_dc);
            DeleteObject(bitmap);
            DeleteObject(resize_bitmap);
            ReleaseDC(desktop, screen_dc);
            throw std::runtime_error("StretchBlt failed");
        }

        // Get resized bitmap info
        BITMAPINFO bmi = { 0 };
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = target_width;
        bmi.bmiHeader.biHeight = -target_height;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 24;
        bmi.bmiHeader.biCompression = BI_RGB;

        // Allocate buffer for pixel data
        std::vector<unsigned char> pixels(target_width * target_height * 3);
        if (!GetDIBits(resize_dc, resize_bitmap, 0, target_height, pixels.data(), &bmi, DIB_RGB_COLORS)) {
            SelectObject(resize_dc, old_resize_bitmap);
            SelectObject(mem_dc, old_bitmap);
            DeleteDC(mem_dc);
            DeleteDC(resize_dc);
            DeleteObject(bitmap);
            DeleteObject(resize_bitmap);
            ReleaseDC(desktop, screen_dc);
            throw std::runtime_error("GetDIBits failed");
        }

        // Clean up GDI resources
        SelectObject(resize_dc, old_resize_bitmap);
        SelectObject(mem_dc, old_bitmap);
        DeleteDC(mem_dc);
        DeleteDC(resize_dc);
        DeleteObject(bitmap);
        DeleteObject(resize_bitmap);
        ReleaseDC(desktop, screen_dc);

        // Convert to JPEG with lower quality (e.g., 20)
        std::vector<unsigned char> jpeg_buffer;
        int stride = target_width * 3;
        stbi_write_jpg_to_func(
            [](void* context, void* data, int size) {
                auto* buffer = static_cast<std::vector<unsigned char>*>(context);
                buffer->insert(buffer->end(), static_cast<unsigned char*>(data), static_cast<unsigned char*>(data) + size);
            },
            &jpeg_buffer, target_width, target_height, 3, pixels.data(), 20 // Lower quality
        );

        if (jpeg_buffer.empty()) {
            throw std::runtime_error("Failed to encode JPEG");
        }

        std::vector<char> jpeg_buffer_char(jpeg_buffer.begin(), jpeg_buffer.end());
        return util::base64_encode(jpeg_buffer_char);
    }
    catch (const std::exception& e) {
        return "[Error] Failed to capture frame: " + std::string(e.what());
    }
}

void ScreenShare::screenshare_loop() {
    while (running_) {
        std::string frame_data = capture_frame();
        nlohmann::json message;
        message["type"] = "screenshare_frame";
        message["data"] = frame_data;
        if (!ws_client_.sendMessage(message.dump())) {
            nlohmann::json error_message;
            error_message["type"] = "screenshare_output";
            error_message["data"] = "[Error] Failed to send frame: Not connected";
            ws_client_.sendMessage(error_message.dump());
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200)); // 5 FPS
    }
}

void ScreenShare::start_screenshare() {
    if (!running_) {
        running_ = true;
        thread_ = std::make_shared<std::thread>(&ScreenShare::screenshare_loop, this);
    }
}

void ScreenShare::stop_screenshare() {
    running_ = false;
    if (thread_) {
        if (thread_->joinable()) {
            thread_->join();
        }
        thread_ = nullptr;
    }
}