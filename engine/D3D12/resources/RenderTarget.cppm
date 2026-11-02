/* SPDX-License-Identifier: Apache-2.0 */

/**
 * @file    RenderTarget.cppm
 * @brief
 *
 * @date    31/10/2026
 *
 * @copyright Copyright (c) 2026 The Project Contributors
 */
module;
#include <array>
#include <cstdint>
export module d3d12.resources:render_target;
import :texture_2d;

export namespace d3d12
{
struct RenderTargetDesc
{
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    DXGI_FORMAT format = DXGI_FORMAT_D32_FLOAT;
    std::array<float, 4> clear_color{0.0f, 0.0f, 0.0f, 0.0f};
    const char *name = nullptr;
};
using RenderTarget = Texture2D;
} // namespace d3d12
