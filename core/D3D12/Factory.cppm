/* SPDX-License-Identifier: Apache-2.0 */

/**
 * @file    Factory.cppm
 * @brief
 *
 * @date    25/10/2026
 *
 * @copyright Copyright (c) 2026 The Project Contributors
 */
module;
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_6.h>
#include <optional>
#include <wrl/client.h>
export module d3d12.factory;
import core.log;
import d3d12.com_resource;
using namespace Microsoft::WRL;
export namespace d3d12
{
class Factory : public ComResource<IDXGIFactory6>
{
    using base = ComResource;

  public:
    Factory() noexcept = default;
#pragma region factory
    static auto create() noexcept -> Factory
    {
        ComPtr<interface_type> factory;
        HRESULT result = CreateDXGIFactory(IID_PPV_ARGS(&factory));
        if (FAILED(result))
        {
            core::diagnostics::log_error("Failed to create DXGI Factory! HRESULT: {}", result);
            return {nullptr};
        }
        return {std::move(factory)};
    }
#pragma endregion
  private:
    Factory(ComPtr<interface_type> &&other) noexcept : base(std::move(other)) {};
};
} // namespace d3d12