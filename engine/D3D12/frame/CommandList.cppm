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
#pragma region factory
    static [[nodiscard]] auto create(ID3D12Device *device, const char *name,
                                     D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT) noexcept
        -> CommandAllocator
    {
        CommandAllocator allocator;
        HRESULT result = device->CreateCommandAllocator(type, IID_PPV_ARGS(allocator.put()));
        if (FAILED(result))
        {
            core::diagnostics::log_error("Failed to create command allocator! Name: {}, HRESULT: {}", name, result);
            return {};
        }
        allocator.set_name(name);
        return allocator;
    }
#pragma endregion
};

class CommandList : public ComResource<ID3D12GraphicsCommandList10>
{
    using base = ComResource;

  public:
    static [[nodiscard]] auto create(ID3D12Device *device, const CommandAllocator &allocator, const char *name,
                                     D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT,
                                     ID3D12PipelineState *state = nullptr) noexcept -> CommandList
    {
        CommandList command_list;
        HRESULT result = device->CreateCommandList(0, type, allocator.get(), state, IID_PPV_ARGS(command_list.put()));
        if (FAILED(result))
        {
            core::diagnostics::log_error("Failed to create command list! Name: {}, HRESULT: {}", name, result);
            return {};
        }
        command_list.set_name(name);
        return command_list;
    }
};
} // namespace d3d12
