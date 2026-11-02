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
#include <dxgi1_6.h>
#include <memory>
#include <string>
#include <wrl/client.h>
export module d3d12.swapchain;
import d3d12.descriptor_heap;
import d3d12.context;
import d3d12.resources;
import d3d12.utils;
import d3d12.com_resource;
import core.log;
using Microsoft::WRL::ComPtr;

export namespace d3d12
{

class SwapChainDesc
{
  public:
    [[nodiscard]] static auto defaults(std::uint32_t width, std::uint32_t height) noexcept -> SwapChainDesc
    {
        SwapChainDesc swapchain_desc{};
        swapchain_desc.swapchain_desc_.Height = height;
        swapchain_desc.swapchain_desc_.Width = width;
        swapchain_desc.swapchain_desc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapchain_desc.swapchain_desc_.BufferCount = 2;
        swapchain_desc.swapchain_desc_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapchain_desc.swapchain_desc_.Stereo = static_cast<BOOL>(false);
        swapchain_desc.swapchain_desc_.SampleDesc.Count = 1;
        swapchain_desc.swapchain_desc_.SampleDesc.Quality = 0;
        swapchain_desc.swapchain_desc_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapchain_desc.swapchain_desc_.Scaling = DXGI_SCALING_STRETCH;
        swapchain_desc.swapchain_desc_.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        swapchain_desc.swapchain_desc_.Flags =
            DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
        return swapchain_desc;
    }

    [[nodiscard]] auto format(DXGI_FORMAT format) noexcept -> SwapChainDesc &
    {
        swapchain_desc_.Format = format;
        return *this;
    };

    [[nodiscard]] auto buffer_count(UINT buffer_count) noexcept -> SwapChainDesc &
    {
        swapchain_desc_.BufferCount = buffer_count;
        return *this;
    };

    [[nodiscard]] auto flags(UINT flags) noexcept -> SwapChainDesc &
    {
        swapchain_desc_.Flags = flags;
        return *this;
    };

    [[nodiscard]] auto get() const noexcept -> DXGI_SWAP_CHAIN_DESC1
    {
        return swapchain_desc_;
    };

  private:
    SwapChainDesc() noexcept = default;

    DXGI_SWAP_CHAIN_DESC1 swapchain_desc_ = {};
};

class SwapChain : public ComResource<IDXGISwapChain4>
{
    using base = ComResource;

  public:
    SwapChain() noexcept = default;
    [[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE current_rtv() const noexcept
    {
        std::uint32_t index = interface_->GetCurrentBackBufferIndex();
        return descriptor_heap_.cpu_handle(index);
    }

    [[nodiscard]] ID3D12Resource *current_back_buffer() const noexcept
    {
        std::uint32_t index = interface_->GetCurrentBackBufferIndex();
        return rtv_buffers_[index].get();
    }

#pragma region factory
    static auto create(const Context *context, const char *name, SwapChainDesc config, HWND hwnd) noexcept -> SwapChain
    {
        const DXGI_SWAP_CHAIN_DESC1 swapchain_desc = config.get();
        ComPtr<IDXGISwapChain1> temp_swap_chain;
        HRESULT result = context->factory()->CreateSwapChainForHwnd(context->queue(), hwnd, &swapchain_desc, nullptr,
                                                                    nullptr, temp_swap_chain.GetAddressOf());
        if (FAILED(result))
        {
            core::diagnostics::log_error("Failed to create swap chain! Name: {}, HRESULT: {}", name, result);
            return {};
        }

        SwapChain swap_chain;
        result = temp_swap_chain->QueryInterface(IID_PPV_ARGS(swap_chain.put()));
        if (FAILED(result))
        {
            core::diagnostics::log_error("Failed to translate interface! Name: {}, HRESULT: {}", name, result);
            return {};
        }

        std::uint32_t buffer_count = swapchain_desc.BufferCount;
        swap_chain.descriptor_heap_ =
            RTVDescriptorHeap::create(context->device(), "swap_chain_rtv_descriptors", swapchain_desc.BufferCount);
        swap_chain.buffer_count_ = buffer_count;

        swap_chain.rtv_buffers_ = std::make_unique<RenderTarget[]>(buffer_count);
        const char temp_name[] = "swap_chain_rtv_buffer  ";
        std::string buffer_name = temp_name;
        for (UINT i = 0; i < buffer_count; i++)
        {
            buffer_name[buffer_name.size() - 1] = static_cast<char>('0' + i);

            swap_chain->GetBuffer(i, IID_PPV_ARGS(swap_chain.rtv_buffers_[i].put()));
            context->device()->CreateRenderTargetView(swap_chain.rtv_buffers_[i].get(), nullptr,
                                                      swap_chain.descriptor_heap_.cpu_handle(i));
            swap_chain.rtv_buffers_[i].set_name(buffer_name.c_str());
        }
        swap_chain.set_name(name);
        return swap_chain;
    }
#pragma endregion

  private:
    RTVDescriptorHeap descriptor_heap_;
    std::unique_ptr<RenderTarget[]> rtv_buffers_;
    std::uint32_t buffer_count_{};
};
} // namespace d3d12