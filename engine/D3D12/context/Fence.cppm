/* SPDX-License-Identifier: Apache-2.0 */

/**
 * @file    Fence.cppm
 * @brief
 *
 * @date    31/10/2026
 *
 * @copyright Copyright (c) 2026 The Project Contributors
 */
module;
#include <d3d12.h>
#include <memory>
export module d3d12.context:fence;
import d3d12.com_resource;
import core.log;

export namespace d3d12
{
class Fence : public ComResource<ID3D12Fence1>
{
    using base = ComResource;

  public:
    ~Fence() noexcept;
    Fence() noexcept = default;
    Fence(Fence &&other) noexcept;
    auto operator=(Fence &&other) noexcept -> Fence &;
    Fence(const Fence &other) noexcept = delete;
    auto operator=(const Fence &other) noexcept -> Fence & = delete;

    static [[nodiscard]] Fence create(ID3D12Device *device, const char *name) noexcept;
    [[nodiscard]] std::uint64_t add_fetch() noexcept;
    [[nodiscard]] std::uint64_t fetch_add() noexcept;
    [[nodiscard]] std::uint64_t get_value() noexcept;
    [[nodiscard]] HANDLE get_event() noexcept;

private:
    std::uint64_t fence_value_{};
    HANDLE fence_event_{};
};
} // namespace d3d12