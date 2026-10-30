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
#include <dxgi1_6.h>
#include <optional>
#include <wrl/client.h>
export module d3d12.device;
import core.log;
import d3d12.factory;
import d3d12.com_resource;
using namespace Microsoft::WRL;

namespace
{

[[nodiscard]] auto create_high_perf_device(const d3d12::Factory &factory) noexcept -> ID3D12Device *
{
    ComPtr<IDXGIAdapter1> adapter;
    ComPtr<ID3D12Device> device;
    for (UINT adapter_index = 0;
         factory->EnumAdapterByGpuPreference(adapter_index, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
                                             IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND;
         ++adapter_index)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if ((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0U)
        {
            continue;
        }

        HRESULT result = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device));
        if (SUCCEEDED(result))
        {
            return device.Detach();
        }
    }
    core::diagnostics::log_error("Failed to create device!");
    return nullptr;
}

} // namespace

export namespace d3d12
{
class Device : public ComResource<ID3D12Device>
{
    using base = ComResource;

  public:
    Device() noexcept = default;
#pragma region factory
    static auto create(const Factory &factory) noexcept -> Device
    {
        ComPtr<ID3D12Device> device = create_high_perf_device(factory);
        if (device.Get() == nullptr)
        {
            return {nullptr};
        }
        return {std::move(device)};
    }
#pragma endregion

  private:
    Device(ComPtr<interface_type> &&other) noexcept : base(std::move(other)) {};
};
}; // namespace d3d12