/* SPDX-License-Identifier: Apache-2.0 */

/**
 * @file    CommandList.cppm
 * @brief
 *
 * @date    31/10/2026
 *
 * @copyright Copyright (c) 2026 The Project Contributors
 */
module;
#include <d3d12.h>
#include <type_traits>
#include <wrl/client.h>
export module d3d12.frame_manager:command_list;
import d3d12.com_resource;
import core.log;

using Microsoft::WRL::ComPtr;
export namespace d3d12
{

class CommandAllocator : public ComResource<ID3D12CommandAllocator>
{
    using base = ComResource;

  public:
    CommandAllocator() = default;
#pragma region factory
    static [[nodiscard]] auto create(ID3D12Device *device, const char *name,
                                     D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT) noexcept
        -> CommandAllocator
    {
        ComPtr<ID3D12CommandAllocator> command_allocator;
        HRESULT result = device->CreateCommandAllocator(type, IID_PPV_ARGS(&command_allocator));
        if (FAILED(result))
        {
            core::diagnostics::log_error("Failed to create command allocator! HRESULT: {}", result);
            return {nullptr, nullptr};
        }
        return {std::move(command_allocator), name};
    }
#pragma endregion
  private:
    CommandAllocator(ComPtr<interface_type> &&other, const char *name) noexcept : base(std::move(other), name) {};
};

class CommandList : public ComResource<ID3D12GraphicsCommandList10>
{
    using base = ComResource;

  public:
    CommandList() noexcept = default;
#pragma region factory
    static [[nodiscard]] auto create(ID3D12Device *device, const CommandAllocator &allocator, const char *name,
                                     D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT,
                                     ID3D12PipelineState *state = nullptr) noexcept -> CommandList
    {
        ComPtr<ID3D12GraphicsCommandList10> command_list;
        HRESULT result = device->CreateCommandList(0, type, allocator.get(), state, IID_PPV_ARGS(&command_list));
        if (FAILED(result))
        {
            core::diagnostics::log_error("Failed to create command list! HRESULT: {}", result);
            return {nullptr, nullptr};
        }
        return {std::move(command_list), name};
    }
#pragma endregion

  private:
    CommandList(ComPtr<interface_type> &&other, const char *name) noexcept : base(std::move(other), name) {};
};
} // namespace d3d12
