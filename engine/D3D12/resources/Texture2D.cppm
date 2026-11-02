/* SPDX-License-Identifier: Apache-2.0 */

/**
 * @file    Texture2D.cppm
 * @brief
 *
 * @date    31/10/2026
 *
 * @copyright Copyright (c) 2026 The Project Contributors
 */
module;
#include <cstdint>
#include <d3d12.h>
export module d3d12.resources:texture_2d;
import d3d12.com_resource;

export namespace d3d12
{
struct Texture2DDesc
{
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
    std::uint16_t mip_levels = 1;
    std::uint16_t array_size = 1;
    const char *name = nullptr;
};

class Texture2D : public ComResource<ID3D12Resource>
{
};
} // namespace d3d12