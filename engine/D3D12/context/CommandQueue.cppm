/* SPDX-License-Identifier: Apache-2.0 */

/**
 * @file    CommandQueue.cppm
 * @brief
 *
 * @date    31/10/2026
 *
 * @copyright Copyright (c) 2026 The Project Contributors
 */
module;
#include <d3d12.h>
export module d3d12.context:command_queue;
import d3d12.com_resource;
import :device;

export namespace d3d12
{
class CommandQueue : public ComResource<ID3D12CommandQueue>
{
    using base = ComResource;

  public:
    CommandQueue() noexcept = default;
#pragma region factory
    static [[nodiscard]] auto create(ID3D12Device *device, const char *name,
                                     D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT,
                                     D3D12_COMMAND_QUEUE_PRIORITY priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
                                     D3D12_COMMAND_QUEUE_FLAGS flags = D3D12_COMMAND_QUEUE_FLAG_NONE) noexcept
        -> CommandQueue;
#pragma endregion
  private:
    CommandQueue(base &&other) noexcept;
};
} // namespace d3d12