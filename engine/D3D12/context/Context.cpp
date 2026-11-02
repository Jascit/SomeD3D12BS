/* SPDX-License-Identifier: Apache-2.0 */

/**
 * @file    Context.cpp
 * @brief
 *
 * @date    31/10/2026
 *
 * @copyright Copyright (c) 2026 The Project Contributors
 */
module;
#include <d3d12.h>
#include <dxgi1_6.h>
#include <memory>
#include <type_traits>
#include <wrl/client.h>
module d3d12.context;
import d3d12.com_resource;
import core.log;

using Microsoft::WRL::ComPtr;
using namespace d3d12;

namespace
{

[[nodiscard]] auto create_high_perf_device(IDXGIFactory6 *factory, ID3D12Device **device) noexcept -> HRESULT
{
    ComPtr<IDXGIAdapter1> adapter;
    HRESULT result = S_OK;
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

        result = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(device));
        if (SUCCEEDED(result))
        {
            return S_OK;
        }
    }
    return result;
}
} // namespace

// :device
auto Device::create(IDXGIFactory6 *factory, const char *name) noexcept -> Device
{
    base device;
    HRESULT result = create_high_perf_device(factory, device.put());
    if (FAILED(result))
    {
        core::diagnostics::log_error("Failed to create device! Name: {}, HRESULT: {}", name,
                                     static_cast<std::uint32_t>(result));
        return {{}};
    }
    return {std::move(device)};
}

Device::Device(base &&other) noexcept : base(std::move(other))
{
}

//: CommandQueue

auto CommandQueue::create(ID3D12Device *device, const char *name, D3D12_COMMAND_LIST_TYPE type,
                          D3D12_COMMAND_QUEUE_PRIORITY priority, D3D12_COMMAND_QUEUE_FLAGS flags) noexcept
    -> CommandQueue
{
    D3D12_COMMAND_QUEUE_DESC desc{};
    desc.Type = type;
    desc.Flags = flags;
    desc.Priority = priority;
    desc.NodeMask = 0;

    base command_queue;
    HRESULT result = device->CreateCommandQueue(&desc, IID_PPV_ARGS(command_queue.put()));
    if (FAILED(result))
    {
        core::diagnostics::log_error("Failed to create command queue! HRESULT: {}", result);
        return {{}};
    }
    command_queue.set_name(name);
    return {std::move(command_queue)};
}

CommandQueue::CommandQueue(base &&other) noexcept : base(std::move(other))
{
}

// :Factory
auto Factory::create(const char *name) noexcept -> Factory
{
    base factory;
    HRESULT result = CreateDXGIFactory(IID_PPV_ARGS(factory.put()));
    if (FAILED(result))
    {
        core::diagnostics::log_error("Failed to create DXGI Factory! Name: {}, HRESULT: {}", name, result);
        return {{}};
    }
    return {std::move(factory)};
}
Factory::Factory(base &&other) noexcept : base(std::move(other))
{
}

Fence::~Fence() noexcept
{
    if (fence_event_)
    {
        CloseHandle(fence_event_);
    }
}

Fence::Fence(Fence &&other) noexcept
    : fence_event_(std::exchange(other.fence_event_, nullptr)), fence_value_(other.fence_value_)
{
}
auto Fence::operator=(Fence &&other) noexcept -> Fence &
{
    if (this != &other)
    {
        if (fence_event_)
            CloseHandle(fence_event_);
        ComResource::operator=(std::move(other));
        fence_event_ = std::exchange(other.fence_event_, nullptr);
    }
    return *this;
}

Fence Fence::create(ID3D12Device *device, const char *name) noexcept
{
    base fence;
    HRESULT result = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence.put()));
    if (FAILED(result))
    {
        core::diagnostics::log_error("Failed to create fence! Name: {}, HRESULT: 0x{:08X}", name,
                                     static_cast<std::uint32_t>(result));
        return Fence({}, nullptr);
    }
    HANDLE event = CreateEventA(nullptr, 0, 0, nullptr);
    fence.set_name(name);
    return {std::move(fence), event};
}

std::uint64_t Fence::add_fetch() noexcept
{
    return ++fence_value_;
}

std::uint64_t Fence::fetch_add() noexcept
{
    return fence_value_++;
}

std::uint64_t Fence::get_value() noexcept
{
    return fence_value_;
}

HANDLE Fence::get_event() noexcept
{
    return fence_event_;
}

Fence::Fence(base &&other, HANDLE event) noexcept : base(std::move(other)), fence_event_(event)
{
}

auto Context::create() noexcept -> Context
{
    Factory factory = Factory::create("main_factory");
    Device device = Device::create(factory.get(), "main_device");
    CommandQueue graphics_cmdq = CommandQueue::create(device.get(), "graphics_command_queue");
    return {std::move(factory), std::move(device), std::move(graphics_cmdq)};
}

ID3D12Device *Context::device() const noexcept
{
    return device_.get();
}
IDXGIFactory6 *Context::factory() const noexcept
{
    return factory_.get();
}

ID3D12CommandQueue *Context::queue() const noexcept
{
    return command_queue_.get();
}

Context::operator bool() const noexcept
{
    if (!device_ || !command_queue_ || !factory_)
    {
        return false;
    }
    return true;
}

Context::Context(Factory &&factory, Device &&device, CommandQueue &&command_queue) noexcept
    : factory_(std::move(factory)), device_(std::move(device)), command_queue_(std::move(command_queue))
{
}