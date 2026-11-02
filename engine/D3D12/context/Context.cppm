/* SPDX-License-Identifier: Apache-2.0 */

/**
 * @file    Context.cppm
 * @brief
 *
 * @date    31/10/2026
 *
 * @copyright Copyright (c) 2026 The Project Contributors
 */
module;
#include <d3d12.h>
#include <dxgi1_6.h>
export module d3d12.context;
export import :command_queue;
export import :device;
export import :factory;
export import :fence;

export namespace d3d12
{
class Context
{
  public:
    static [[nodiscard]] auto create() noexcept -> Context;
    [[nodiscard]] ID3D12Device *device() const noexcept;
    [[nodiscard]] IDXGIFactory6 *factory() const noexcept;
    [[nodiscard]] ID3D12CommandQueue *queue() const noexcept;
    explicit operator bool() const noexcept;

  private:
    Context(Factory &&factory, Device &&device, CommandQueue &&command_queue) noexcept;

    Factory factory_;
    Device device_;
    CommandQueue command_queue_;
};
} // namespace d3d12
