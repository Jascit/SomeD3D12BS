/* SPDX-License-Identifier: Apache-2.0 */

/**
 * @file    commands.cppm
 * @brief
 *
 * @date    25/10/2026
 *
 * @copyright Copyright (c) 2026 The Project Contributors
 */
module;
#include <d3d12.h>
#include <type_traits>
#include <wrl/client.h>
export module d3d12.commands;
import d3d12.device;
import core.log;
import d3d12.com_resource;

using Microsoft::WRL::ComPtr;
export namespace d3d12
{

class CommandQueue : public ComResource<ID3D12CommandQueue>
{
    using base = ComResource;

  public:
    CommandQueue() noexcept = default;
#pragma region factory
    static
        [[nodiscard]] auto create(const Device &device, D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT,
                                  D3D12_COMMAND_QUEUE_PRIORITY priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
                                  D3D12_COMMAND_QUEUE_FLAGS flags = D3D12_COMMAND_QUEUE_FLAG_NONE) noexcept
        -> CommandQueue
    {
        D3D12_COMMAND_QUEUE_DESC desc{};
        desc.Type = type;
        desc.Flags = flags;
        desc.Priority = priority;
        desc.NodeMask = 0;
        ComPtr<ID3D12CommandQueue> command_queue;
        HRESULT result = device->CreateCommandQueue(&desc, IID_PPV_ARGS(&command_queue));
        if (FAILED(result))
        {
            core::diagnostics::log_error("Failed to create command queue! HRESULT: {}", result);
            return {nullptr};
        }
        return {std::move(command_queue)};
    }
#pragma endregion
  private:
    CommandQueue(ComPtr<interface_type> &&other) noexcept : base(std::move(other)) {};
};

class CommandAllocator : public ComResource<ID3D12CommandAllocator>
{
    using base = ComResource;

  public:
    CommandAllocator() = default;
#pragma region factory
    static [[nodiscard]] auto create(const Device &device,
                                     D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT) noexcept
        -> CommandAllocator
    {
        ComPtr<ID3D12CommandAllocator> command_allocator;
        HRESULT result = device->CreateCommandAllocator(type, IID_PPV_ARGS(&command_allocator));
        if (FAILED(result))
        {
            core::diagnostics::log_error("Failed to create command allocator! HRESULT: {}", result);
            return {nullptr};
        }
        return {std::move(command_allocator)};
    }
#pragma endregion
  private:
    CommandAllocator(ComPtr<interface_type> &&other) noexcept : base(std::move(other)) {};
};

class CommandList : public ComResource<ID3D12GraphicsCommandList10>
{
    using base = ComResource;

  public:
    CommandList() noexcept = default;
#pragma region factory
    static [[nodiscard]] auto create(const Device &device, const CommandAllocator &allocator,
                                     D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT,
                                     ID3D12PipelineState *state = nullptr) noexcept -> CommandList
    {
        ComPtr<ID3D12GraphicsCommandList10> command_list;
        HRESULT result = device->CreateCommandList(0, type, allocator.get(), state, IID_PPV_ARGS(&command_list));
        if (FAILED(result))
        {
            core::diagnostics::log_error("Failed to create command list! HRESULT: {}", result);
            return {nullptr};
        }
        return {std::move(command_list)};
    }
#pragma endregion

  private:
    CommandList(ComPtr<interface_type> &&other) noexcept : base(std::move(other)) {};
};
}; // namespace d3d12