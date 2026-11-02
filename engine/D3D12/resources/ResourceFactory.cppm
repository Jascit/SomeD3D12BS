/* SPDX-License-Identifier: Apache-2.0 */

/**
 * @file    ResourceFactory.cppm
 * @brief
 *
 * @date    31/10/2026
 *
 * @copyright Copyright (c) 2026 The Project Contributors
 */
module;
#include <d3d12.h>
export module d3d12.resources:resource_factory;
import :texture_2d;
import :render_target;
import :depth_stencil;
import :upload_buffer;

export namespace d3d12
{
class ResourceFactory
{
  public:
    static [[nodiscard]] ResourceFactory create(ID3D12Device *device) noexcept;
    [[nodiscard]] DepthStencil create_depth_stencil(const DepthStencilDesc &desc) noexcept;
    [[nodiscard]] UploadBuffer create_upload_buffer(const UploadBufferDesc &desc) noexcept;
    [[nodiscard]] RenderTarget create_render_target(const RenderTargetDesc &desc) noexcept;
    [[nodiscard]] Texture2D create_texture_2d(const Texture2DDesc &desc) noexcept;

  private:
    ID3D12Device *device_;
};
} // namespace d3d12