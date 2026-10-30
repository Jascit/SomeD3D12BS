/* SPDX-License-Identifier: Apache-2.0 */

/**
 * @file    SwapChain.cppm
 * @brief
 *
 * @date    25/10/2026
 *
 * @copyright Copyright (c) 2026 The Project Contributors
 */
module;
#include <cstdint>
#include <d3d12.h>
#include <dxgi1_5.h>
#include <dxgi1_6.h>
#include <memory>
#include <type_traits>
#include <wrl/client.h>
export module d3d12.swapchain;
import core.log;
import d3d12.device;
import d3d12.commands;
import d3d12.factory;
import d3d12.descriptor_heap;
import d3d12.com_resource;
using Microsoft::WRL::ComPtr;

export namespace d3d12
{

class SwapChainConfig
{
  public:
    SwapChainConfig() noexcept = default;

    [[nodiscard]] static auto makeDefault(std::uint32_t width, std::uint32_t height) noexcept -> SwapChainConfig
    {
        DXGI_SWAP_CHAIN_DESC1 swapchain_desc{};
        swapchain_desc.Height = height;
        swapchain_desc.Width = width;
        swapchain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapchain_desc.BufferCount = 2;
        swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapchain_desc.Stereo = static_cast<BOOL>(false);
        swapchain_desc.SampleDesc.Count = 1;
        swapchain_desc.SampleDesc.Quality = 0;
        swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapchain_desc.Scaling = DXGI_SCALING_STRETCH;
        swapchain_desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        swapchain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
        return {swapchain_desc};
    }

    [[nodiscard]] auto withFormat(DXGI_FORMAT format) noexcept -> SwapChainConfig &
    {
        swapchain_desc_.Format = format;
        return *this;
    };

    [[nodiscard]] auto withBufferCount(UINT buffer_count) noexcept -> SwapChainConfig &
    {
        swapchain_desc_.BufferCount = buffer_count;
        return *this;
    };

    [[nodiscard]] auto withFlags(UINT flags) noexcept -> SwapChainConfig &
    {
        swapchain_desc_.Flags = flags;
        return *this;
    };

    [[nodiscard]] auto get() const noexcept -> DXGI_SWAP_CHAIN_DESC1
    {
        return swapchain_desc_;
    };

  private:
    SwapChainConfig(DXGI_SWAP_CHAIN_DESC1 &swapchain_desc) noexcept : swapchain_desc_(swapchain_desc) {};
    DXGI_SWAP_CHAIN_DESC1 swapchain_desc_ = {};
};

class SwapChain : public ComResource<IDXGISwapChain4>
{
    using base = ComResource;

  public:
    SwapChain() noexcept = default;
#pragma region accessors
    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE getRTVDescriptorHandle(std::uint32_t frame_index) const noexcept
    {
        return descriptor_heap_.getCPUHandle(frame_index);
    }

    [[nodiscard]] ID3D12Resource *getBuffer(std::uint32_t frame_index) const noexcept
    {
        return buffers_[frame_index].Get();
    }
#pragma endregion

#pragma region factory
    static auto create(RTVDescriptorHeap &&descriptor_heap, const Device &device, const Factory &factory,
                       const CommandQueue &command_queue, const SwapChainConfig &config, HWND hwnd) noexcept
        -> SwapChain
    {
        const DXGI_SWAP_CHAIN_DESC1 swapchain_desc = config.get();
        IDXGISwapChain1 *temp_swap_chain = nullptr;
        HRESULT result = factory->CreateSwapChainForHwnd(command_queue.get(), hwnd, &swapchain_desc, nullptr, nullptr,
                                                         &temp_swap_chain);
        if (FAILED(result))
        {
            core::diagnostics::log_error("Failed to create swap chain! HRESULT: {}", result);
            return {nullptr, {}, nullptr};
        }
        ComPtr<IDXGISwapChain4> swap_chain;
        result = temp_swap_chain->QueryInterface(IID_PPV_ARGS(&swap_chain));
        if (FAILED(result))
        {
            core::diagnostics::log_error("Failed to translate interface! HRESULT: {}", result);
            return {nullptr, {}, nullptr};
        }

        std::unique_ptr<ComPtr<ID3D12Resource>[]> buffers =
            std::make_unique<ComPtr<ID3D12Resource>[]>(swapchain_desc.BufferCount);
        for (UINT i = 0; i < swapchain_desc.BufferCount; i++)
        {
            swap_chain->GetBuffer(i, IID_PPV_ARGS(&buffers[i]));
            device->CreateRenderTargetView(buffers[i].Get(), nullptr, descriptor_heap.getCPUHandle(i));
        }
        return {std::move(swap_chain), std::move(descriptor_heap), std::move(buffers)};
    }
#pragma endregion

  private:
    SwapChain(ComPtr<interface_type> &&other, RTVDescriptorHeap &&rtv_heap,
              std::unique_ptr<ComPtr<ID3D12Resource>[]> &&buffers) noexcept
        : base(std::move(other)), descriptor_heap_(std::move(rtv_heap)), buffers_(std::move(buffers)) {};

    ComPtr<interface_type> interface_;
    RTVDescriptorHeap descriptor_heap_;
    std::unique_ptr<ComPtr<ID3D12Resource>[]> buffers_;
};
} // namespace d3d12