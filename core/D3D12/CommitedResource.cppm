/* SPDX-License-Identifier: Apache-2.0 */

/**
 * @file    CommitedResource.cppm
 * @brief
 *
 * @date    27/10/2026
 *
 * @copyright Copyright (c) 2026 The Project Contributors
 */
module;
#include <cstdint>
#include <d3d12.h>
#include <type_traits>
#include <wrl/client.h>
export module d3d12.commited_resource;
import d3d12.com_resource;
import d3d12.device;
import core.log;
using Microsoft::WRL::ComPtr;
export namespace d3d12
{
class CommitedResourceConfig
{
    struct Context
    {
        D3D12_HEAP_PROPERTIES heap_properties_;
        D3D12_RESOURCE_DESC resource_descriptor_;
        D3D12_CLEAR_VALUE clear_value;
        D3D12_RESOURCE_STATES states;
    };

  public:
    CommitedResourceConfig() noexcept : context_{}
    {
    }

    static [[nodiscard]] CommitedResourceConfig defaults(D3D12_HEAP_TYPE heap_type, D3D12_RESOURCE_DIMENSION dimension,
                                                         std::uint16_t depth, std::uint32_t height, std::uint32_t width,
                                                         D3D12_RESOURCE_STATES state,
                                                         DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN) noexcept
    {
        D3D12_HEAP_PROPERTIES heap_properties{};
        heap_properties.Type = heap_type;
        heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heap_properties.CreationNodeMask = 0;
        heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heap_properties.VisibleNodeMask = 0;

        D3D12_RESOURCE_DESC resource_descriptor;
        resource_descriptor.Dimension = dimension;
        resource_descriptor.Format = format;
        resource_descriptor.DepthOrArraySize = depth;
        resource_descriptor.Height = height;
        resource_descriptor.Width = width;
        resource_descriptor.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
        resource_descriptor.Flags = D3D12_RESOURCE_FLAG_NONE;
        resource_descriptor.MipLevels = 1;
        resource_descriptor.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resource_descriptor.SampleDesc.Count = 1;
        resource_descriptor.SampleDesc.Quality = 0;
        return {heap_properties, resource_descriptor, {}, state};
    }
    static [[nodiscard]] CommitedResourceConfig texture2D(
        std::uint32_t height, std::uint32_t width, D3D12_RESOURCE_STATES state,
        DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE) noexcept
    {
        D3D12_HEAP_PROPERTIES heap_properties{};
        heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;
        heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heap_properties.CreationNodeMask = 0;
        heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heap_properties.VisibleNodeMask = 0;

        D3D12_RESOURCE_DESC resource_descriptor;
        resource_descriptor.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        resource_descriptor.Format = format;
        resource_descriptor.DepthOrArraySize = 1;
        resource_descriptor.Height = height;
        resource_descriptor.Width = width;
        resource_descriptor.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
        resource_descriptor.Flags = flags;
        resource_descriptor.MipLevels = 1;
        resource_descriptor.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        resource_descriptor.SampleDesc.Count = 1;
        resource_descriptor.SampleDesc.Quality = 0;
        return {heap_properties, resource_descriptor, {}, state};
    }

    static [[nodiscard]] CommitedResourceConfig depthStencil(
        std::uint32_t height, std::uint32_t width, DXGI_FORMAT format, D3D12_RESOURCE_STATES state,
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) noexcept
    {
        D3D12_HEAP_PROPERTIES heap_properties{};
        heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;
        heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heap_properties.CreationNodeMask = 0;
        heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heap_properties.VisibleNodeMask = 0;

        D3D12_CLEAR_VALUE clear_value{};
        clear_value.Format = format;
        clear_value.DepthStencil.Depth = 1.0f;
        clear_value.DepthStencil.Stencil = 0.0f;

        D3D12_RESOURCE_DESC resource_descriptor;
        resource_descriptor.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        resource_descriptor.Format = format;
        resource_descriptor.DepthOrArraySize = 1;
        resource_descriptor.Height = height;
        resource_descriptor.Width = width;
        resource_descriptor.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
        resource_descriptor.Flags = flags;
        resource_descriptor.MipLevels = 1;
        resource_descriptor.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        resource_descriptor.SampleDesc.Count = 1;
        resource_descriptor.SampleDesc.Quality = 0;
        return {heap_properties, resource_descriptor, clear_value, state};
    }

    static [[nodiscard]] CommitedResourceConfig buffer(D3D12_HEAP_TYPE heap_type, std::uint32_t bytes,
                                                       D3D12_RESOURCE_STATES state,
                                                       D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE) noexcept
    {
        D3D12_HEAP_PROPERTIES heap_properties{};
        heap_properties.Type = heap_type;
        heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heap_properties.CreationNodeMask = 0;
        heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heap_properties.VisibleNodeMask = 0;

        D3D12_RESOURCE_DESC resource_descriptor;
        resource_descriptor.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resource_descriptor.Format = DXGI_FORMAT_UNKNOWN;
        resource_descriptor.DepthOrArraySize = 1;
        resource_descriptor.Height = 1;
        resource_descriptor.Width = bytes;
        resource_descriptor.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
        resource_descriptor.Flags = flags;
        resource_descriptor.MipLevels = 1;
        resource_descriptor.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resource_descriptor.SampleDesc.Count = 1;
        resource_descriptor.SampleDesc.Quality = 0;
        return {heap_properties, resource_descriptor, {}, state};
    }

    [[nodiscard]] auto heapType(D3D12_HEAP_TYPE heap_type) noexcept -> CommitedResourceConfig &
    {
        context_.heap_properties_.Type = heap_type;
        return *this;
    }
    [[nodiscard]] auto dimension(D3D12_RESOURCE_DIMENSION dimension) noexcept -> CommitedResourceConfig &
    {
        context_.resource_descriptor_.Dimension = dimension;
        return *this;
    }
    [[nodiscard]] auto format(DXGI_FORMAT format) noexcept -> CommitedResourceConfig &
    {
        context_.resource_descriptor_.Format = format;
        return *this;
    }
    [[nodiscard]] auto size(std::uint16_t depth, std::uint32_t height, std::uint32_t width) noexcept
        -> CommitedResourceConfig &
    {
        context_.resource_descriptor_.DepthOrArraySize = depth;
        context_.resource_descriptor_.Height = height;
        context_.resource_descriptor_.Width = width;
        return *this;
    }
    [[nodiscard]] auto alignment(std::uint64_t alignment) noexcept -> CommitedResourceConfig &
    {
        context_.resource_descriptor_.Alignment = alignment;
        return *this;
    }
    [[nodiscard]] auto flags(D3D12_RESOURCE_FLAGS flags) noexcept -> CommitedResourceConfig &
    {
        context_.resource_descriptor_.Flags = flags;
        return *this;
    }
    [[nodiscard]] auto mipLevels(std::uint16_t mip_levels) noexcept -> CommitedResourceConfig &
    {
        context_.resource_descriptor_.MipLevels = mip_levels;
        return *this;
    }
    [[nodiscard]] auto layout(D3D12_TEXTURE_LAYOUT layout) noexcept -> CommitedResourceConfig &
    {
        context_.resource_descriptor_.Layout = layout;
        return *this;
    }
    [[nodiscard]] auto sample(std::uint32_t count, std::uint32_t quality) noexcept -> CommitedResourceConfig &
    {
        context_.resource_descriptor_.SampleDesc.Count = count;
        context_.resource_descriptor_.SampleDesc.Quality = quality;
        return *this;
    }

    [[nodiscard]] auto get() const noexcept -> Context
    {
        return context_;
    };

  private:
    CommitedResourceConfig(D3D12_HEAP_PROPERTIES properties, D3D12_RESOURCE_DESC resource_desc, D3D12_CLEAR_VALUE value,
                           D3D12_RESOURCE_STATES state) noexcept
        : context_({.heap_properties_ = properties,
                    .resource_descriptor_ = resource_desc,
                    .clear_value = value,
                    .states = state})
    {
    }

    Context context_;
};

class CommitedResource : public ComResource<ID3D12Resource>
{
    using base = ComResource;

  public:
    static [[nodiscard]] CommitedResource create(const Device &device, const CommitedResourceConfig &config) noexcept
    {
        auto context = config.get();
        D3D12_CLEAR_VALUE *clear_value = nullptr;
        D3D12_CLEAR_VALUE zeroed_value{};
        std::uint32_t cmp = memcmp(&context.clear_value, &zeroed_value, sizeof(D3D12_CLEAR_VALUE));
        if (cmp != 0)
        {
            clear_value = &context.clear_value;
        }

        ComPtr<ID3D12Resource> resource;
        HRESULT result = device->CreateCommittedResource(&context.heap_properties_, D3D12_HEAP_FLAG_NONE,
                                                         &context.resource_descriptor_, context.states, clear_value,
                                                         IID_PPV_ARGS(&resource));
        if (FAILED(result))
        {
            core::diagnostics::log_error("Failed to create committed resource! HRESULT: 0x{:08X}",
                                         static_cast<unsigned long>(result));
            return {nullptr};
        }
        return {std::move(resource)};
    }

  private:
    CommitedResource(ComPtr<interface_type> &&other) noexcept : base(std::move(other))
    {
    }
};
} // namespace d3d12