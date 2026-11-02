/* SPDX-License-Identifier: Apache-2.0 */

/**
 * @file    DepthStencil.cppm
 * @brief
 *
 * @date    31/10/2026
 *
 * @copyright Copyright (c) 2026 The Project Contributors
 */
module;
#include <cstdint>
export module d3d12.resources:depth_stencil;
import :texture_2d;

export namespace d3d12
{
struct DepthStencilDesc
{
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    DXGI_FORMAT format = DXGI_FORMAT_D32_FLOAT;
    float clear_depth = 1.0f;
    const char *name = nullptr;
};
using DepthStencil = Texture2D;
} // namespace d3d12
