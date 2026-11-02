/* SPDX-License-Identifier: Apache-2.0 */

/**
 * @file    Device.cppm
 * @brief
 *
 * @date    25/10/2026
 *
 * @copyright Copyright (c) 2026 The Project Contributors
 */
module;
#include <d3d12.h>
#include <wrl/client.h>
export module d3d12.context:device;
import d3d12.com_resource;
import :factory;
using Microsoft::WRL::ComPtr;

export namespace d3d12
{
class Device : public ComResource<ID3D12Device>
{
    using base = ComResource;

  public:
    static auto create(IDXGIFactory6 *factory, const char *name) noexcept -> Device;

};
}; // namespace d3d12