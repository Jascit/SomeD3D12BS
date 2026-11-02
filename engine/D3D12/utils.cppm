/* SPDX-License-Identifier: Apache-2.0 */

/**
 * @file    utils.cppm
 * @brief
 *
 * @date    11/10/2026
 *
 * @copyright Copyright (c) 2026 The Project Contributors
 */
module;
#include <d3d12.h>
#include <dxgi1_3.h>
#include <dxgidebug.h>
#include <filesystem>
#include <wrl/client.h>
export module d3d12.utils;
import core.log;
using namespace Microsoft::WRL;

[[nodiscard]] auto pack_rgba(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a) noexcept -> std::uint32_t
{
    return (static_cast<std::uint32_t>(a) << 24) | (static_cast<std::uint32_t>(b) << 16) |
           (static_cast<std::uint32_t>(g) << 8) | static_cast<std::uint32_t>(r);
}

[[nodiscard]] auto hue_to_pixel(float h, float s, float v) noexcept -> std::uint32_t
{
    // процент до якого кола відтінку відноситься
    h = h - std::floor(h);
    // ціле число до якого кола відтінку відноситься та його внутрішній процент
    float hh = h * 6.0f;
    // цілий індекс кола
    int i = int(hh);
    // процент внутрі кола
    float f = hh - i;

    float minimal = v * (1 - s);
    float grow = v * (1 - s * (1 - f));
    float shrink = v * (1 - s * f);

    float r, g, b;
    switch (i % 6)
    {
    case 0:
        r = v;
        g = grow;
        b = minimal;
        break;
    case 1:
        r = shrink;
        g = v;
        b = minimal;
        break;
    case 2:
        r = minimal;
        g = v;
        b = grow;
        break;
    case 3:
        r = minimal;
        g = shrink;
        b = v;
        break;
    case 4:
        r = grow;
        g = minimal;
        b = v;
        break;
    default:
        r = v;
        g = minimal;
        b = shrink;
        break;
    }
    auto to_uint8 = [](float value) noexcept {
        return static_cast<std::uint8_t>((std::clamp(value, 0.0f, 1.0f) * 255.0f) + 0.5f);
    };
    return pack_rgba(to_uint8(r), to_uint8(g), to_uint8(b), 0xFFU);
}

export namespace d3d12::utils
{
void enable_d3d12_debug_layer()
{
#if defined(_DEBUG)
    ComPtr<ID3D12Debug> debug_controller;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller))))
    {
        debug_controller->EnableDebugLayer();

        ComPtr<ID3D12Debug1> debug_controller1;
        if (SUCCEEDED(debug_controller.As(&debug_controller1)))
        {
            debug_controller1->SetEnableGPUBasedValidation(TRUE);
        }
    }
#endif
}

[[nodiscard]] auto generate_rainbow_texture(std::uint32_t width, std::uint32_t height) noexcept
    -> std::vector<std::uint32_t>
{
    std::vector<std::uint32_t> pixel_data(width * height);
    for (std::uint32_t h = 0; h < height; h++)
    {
        for (std::uint32_t w = 0; w < width; w++)
        {
            float hue = static_cast<float>(w) / static_cast<float>(width);
            pixel_data[h * width + w] = hue_to_pixel(hue, 1.0f, 1.0f);
        }
    }
    return pixel_data;
}

[[nodiscard]] auto to_wide(std::string_view name) noexcept -> std::wstring
{
    std::int32_t wlength = MultiByteToWideChar(CP_UTF8, 0, name.data(), static_cast<int>(name.size()), nullptr, 0);
    if (wlength == 0)
        return {};

    std::wstring wide_name(wlength, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, name.data(), static_cast<int>(name.size()), wide_name.data(), wlength);
    return wide_name;
}

[[nodiscard]] auto to_utf8(std::wstring_view wide_name) noexcept -> std::string
{
    std::int32_t length =
        WideCharToMultiByte(CP_UTF8, 0, wide_name.data(), static_cast<int>(wide_name.size()), nullptr, 0, nullptr, 0);
    if (length == 0)
    {
        return {};
    }

    std::string name(length, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide_name.data(), static_cast<int>(wide_name.size()), name.data(), length, nullptr,
                        nullptr);
    return name;
}

class Reporter
{
  public:
    Reporter() noexcept = default;
    ~Reporter() noexcept
    {
        ComPtr<IDXGIDebug1> dxgi_debug;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgi_debug))))
        {
            dxgi_debug->ReportLiveObjects(DXGI_DEBUG_ALL,
                                          DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
        }
    }
};
} // namespace d3d12::utils
