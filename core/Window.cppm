/* SPDX-License-Identifier: Apache-2.0 */

/**
 * @file    Window.cppm
 * @brief
 *
 * @date    07/10/2026
 *
 * @copyright Copyright (c) 2026 The Project Contributors
 */
module;
#include <cstdint>
#include <regex>

#include "Windows.h"
export module core.window;
import core.log;

namespace
{
auto CALLBACK window_proc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) -> LRESULT
{
    switch (umsg)
    {
    case WM_CREATE:
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_KEYDOWN:
        core::diagnostics::log_info("Key down: {}", wparam);
        break;
    default:;
    }
    return DefWindowProcW(hwnd, umsg, wparam, lparam);
}
} // namespace

export namespace core
{
class Window
{
  public:
#pragma region rule_of_5
    Window() noexcept = default;
    Window(const Window &) = delete;

    Window(Window &&window) noexcept
    {
        hwnd_ = std::exchange(window.hwnd_, nullptr);
        window_class_name_ = std::exchange(window.window_class_name_, nullptr);
        width_ = std::exchange(window.width_, 0);
        height_ = std::exchange(window.height_, 0);
    }

    auto operator=(const Window &window) -> Window & = delete;
    auto operator=(Window &&window) -> Window & = delete;
#pragma endregion

    static auto create(const wchar_t *window_name, const wchar_t *window_class_name, int32_t width = 800,
                       int32_t height = 600) noexcept -> Window
    {
        HINSTANCE h_instance = GetModuleHandleA(nullptr);
        WNDCLASSEXW window_class = {};
        window_class = {};
        window_class.cbSize = sizeof(WNDCLASSEXW);
        window_class.lpfnWndProc = &window_proc;
        window_class.hInstance = h_instance;
        window_class.lpszClassName = window_class_name;

        auto res = RegisterClassExW(&window_class);
        if (res == 0)
        {
            DWORD error_code = GetLastError();
            diagnostics::log_error("RegisterClassExW failed! Win32 Error Code: {}", error_code);
            return {};
        }

        HWND hwnd = CreateWindowExW(0, window_class_name, window_name, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                                    CW_USEDEFAULT, width, height, nullptr, nullptr, h_instance, nullptr);

        if (hwnd == nullptr)
        {
            DWORD error_code = GetLastError();
            diagnostics::log_error("CreateWindowExW failed! Win32 Error Code: {}", error_code);
            return {};
        }

        return {hwnd, window_class_name, width, height};
    };

    ~Window()
    {
        DestroyWindow(hwnd_);
    }

    void show()
    {
        ShowWindow(hwnd_, SW_SHOW);
    }

    [[nodiscard]] auto getSystemHandle() const -> HWND
    {
        return hwnd_;
    }

    [[nodiscard]] auto getHeight() const -> int
    {
        return height_;
    }

    [[nodiscard]] auto getWidth() const -> int
    {
        return width_;
    }

    explicit operator bool() noexcept
    {
        return hwnd_ != nullptr;
    }

  private:
    Window(HWND hwnd, const wchar_t *window_class_name, int32_t width, int32_t height) noexcept
        : hwnd_(hwnd), window_class_name_(window_class_name), width_(width), height_(height) {};

    HWND hwnd_;
    const wchar_t *window_class_name_;
    int32_t width_;
    int32_t height_;
};
} // namespace core
