/* SPDX-License-Identifier: Apache-2.0 */

/**
 * @file    resources.cppm
 * @brief
 *
 * @date    31/10/2026
 *
 * @copyright Copyright (c) 2026 The Project Contributors
 */

module;
#include <cassert>
#include <cstdint>
#include <d3d12.h>
#include <vector>
export module d3d12.resources;
export import :render_target;
export import :resource_factory;
export import :upload_buffer;
export import :depth_stencil;
export import :texture_2d;

export namespace d3d12
{
struct PlacedSubresourceFootprint
{
    std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> layouts;
    std::vector<std::uint32_t> row_counts;
    std::vector<std::uint64_t> row_sizes;
    std::uint64_t total_bytes = 0;
};

[[nodiscard]] PlacedSubresourceFootprint get_copyable_footprints(ID3D12Device *device,
                                                                 const D3D12_RESOURCE_DESC &resource_desc,
                                                                 std::uint32_t subresource_index,
                                                                 std::uint32_t num_subresources,
                                                                 std::uint64_t buffer_offset) noexcept
{
    const std::uint32_t total_subresources =
        static_cast<std::uint32_t>(resource_desc.MipLevels) * resource_desc.DepthOrArraySize;
    assert((subresource_index + num_subresources) <= total_subresources);
    PlacedSubresourceFootprint result;
    if (num_subresources == 0)
    {
        return {};
    }

    result.layouts.resize(num_subresources);
    result.row_counts.resize(num_subresources);
    result.row_sizes.resize(num_subresources);

    device->GetCopyableFootprints(&resource_desc, subresource_index, num_subresources, buffer_offset,
                                  result.layouts.data(), result.row_counts.data(), result.row_sizes.data(),
                                  &result.total_bytes);
    return result;
}
} // namespace d3d12