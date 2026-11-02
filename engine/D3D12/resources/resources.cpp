/* SPDX-License-Identifier: Apache-2.0 */

/**
 * @file    resources.cpp
 * @brief
 *
 * @date    31/10/2026
 *
 * @copyright Copyright (c) 2026 The Project Contributors
 */
module;
#include <cstdint>
#include <d3d12.h>
module d3d12.resources;
import core.log;
using namespace d3d12;

ResourceFactory ResourceFactory::create(ID3D12Device *device) noexcept
{
    ResourceFactory resource_factory{};
    resource_factory.device_ = device;
    return resource_factory;
}

DepthStencil ResourceFactory::create_depth_stencil(const DepthStencilDesc &desc) noexcept
{
    D3D12_HEAP_PROPERTIES heap_properties{};
    heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_RESOURCE_DESC resource_desc;
    resource_desc.Width = desc.width;
    resource_desc.Height = desc.height;
    resource_desc.Format = desc.format;
    resource_desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    resource_desc.DepthOrArraySize = 1;
    resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resource_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resource_desc.MipLevels = 1;
    resource_desc.SampleDesc.Count = 1;
    resource_desc.SampleDesc.Quality = 0;
    resource_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clear_value;
    clear_value.Format = desc.format;
    clear_value.DepthStencil.Depth = desc.clear_depth;
    clear_value.DepthStencil.Stencil = 0;

    DepthStencil depth_stencil;
    HRESULT result = device_->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc,
                                                      D3D12_RESOURCE_STATE_DEPTH_WRITE, &clear_value,
                                                      IID_PPV_ARGS(depth_stencil.put()));
    if (FAILED(result))
    {
        core::diagnostics::log_error("Failed to create depth stencil buffer! Name: {}, HRESULT: 0x{:08X}", desc.name,
                                     static_cast<std::uint32_t>(result));
        return {};
    }
    depth_stencil.set_name(desc.name);
    return depth_stencil;
}

UploadBuffer ResourceFactory::create_upload_buffer(const UploadBufferDesc &desc) noexcept
{
    D3D12_HEAP_PROPERTIES heap_properties{};
    heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC resource_desc;
    resource_desc.Width = desc.total_bytes;
    resource_desc.Height = 1;
    resource_desc.Format = DXGI_FORMAT_UNKNOWN;
    resource_desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    resource_desc.DepthOrArraySize = 1;
    resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resource_desc.MipLevels = 1;
    resource_desc.SampleDesc.Count = 1;
    resource_desc.SampleDesc.Quality = 0;
    resource_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    UploadBuffer upload_buffer;
    HRESULT result =
        device_->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc,
                                         D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(upload_buffer.put()));
    if (FAILED(result))
    {
        core::diagnostics::log_error("Failed to create upload buffer! Name: {}, HRESULT: 0x{:08X}", desc.name,
                                     static_cast<std::uint32_t>(result));
        return {};
    }
    upload_buffer.set_name(desc.name);
    return upload_buffer;
}

RenderTarget ResourceFactory::create_render_target(const RenderTargetDesc &desc) noexcept
{
    D3D12_HEAP_PROPERTIES heap_properties{};
    heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_RESOURCE_DESC resource_desc;
    resource_desc.Width = desc.width;
    resource_desc.Height = desc.height;
    resource_desc.Format = desc.format;
    resource_desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    resource_desc.DepthOrArraySize = 1;
    resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resource_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resource_desc.MipLevels = 1;
    resource_desc.SampleDesc.Count = 1;
    resource_desc.SampleDesc.Quality = 0;
    resource_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    D3D12_CLEAR_VALUE clear_value;
    clear_value.Format = desc.format;
    clear_value.Color[0] = desc.clear_color[0];
    clear_value.Color[1] = desc.clear_color[1];
    clear_value.Color[2] = desc.clear_color[2];
    clear_value.Color[3] = desc.clear_color[3];

    RenderTarget render_target;
    HRESULT result = device_->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc,
                                                      D3D12_RESOURCE_STATE_RENDER_TARGET, &clear_value,
                                                      IID_PPV_ARGS(render_target.put()));
    if (FAILED(result))
    {
        core::diagnostics::log_error("Failed to create depth stencil buffer! Name: {}, HRESULT: 0x{:08X}", desc.name,
                                     static_cast<std::uint32_t>(result));
        return {};
    }
    render_target.set_name(desc.name);
    return render_target;
}
Texture2D ResourceFactory::create_texture_2d(const Texture2DDesc &desc) noexcept
{
    D3D12_HEAP_PROPERTIES heap_properties{};
    heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_RESOURCE_DESC resource_desc;
    resource_desc.Width = desc.width;
    resource_desc.Height = desc.height;
    resource_desc.Format = desc.format;
    resource_desc.DepthOrArraySize = desc.array_size;
    resource_desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resource_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resource_desc.MipLevels = desc.mip_levels;
    resource_desc.SampleDesc.Count = 1;
    resource_desc.SampleDesc.Quality = 0;
    resource_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

    Texture2D texture;
    HRESULT result =
        device_->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc,
                                         D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(texture.put()));
    if (FAILED(result))
    {
        core::diagnostics::log_error("Failed to create texture2D! Name: {}, HRESULT: 0x{:08X}", desc.name,
                                     static_cast<std::uint32_t>(result));
        return {};
    }
    texture.set_name(desc.name);
    return texture;
}
