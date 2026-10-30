/* SPDX-License-Identifier: Apache-2.0 */

/**
 * @file    ComResource.cppm
 * @brief
 *
 * @date    26/10/2026
 *
 * @copyright Copyright (c) 2026 The Project Contributors
 */
module;
#include <type_traits>
#include <wrl/client.h>
export module d3d12.com_resource;
using Microsoft::WRL::ComPtr;

export namespace d3d12
{
template <typename ComObj> class ComResource
{
  protected:
    using interface_type = ComObj;
    using interface_pointer = interface_type *;

    explicit ComResource(ComPtr<ComObj> &&obj) noexcept : interface_(std::move(obj)) {};

  public:
#pragma region rule_of_5
    ~ComResource() noexcept = default;
    ComResource() noexcept = default;
    ComResource(ComResource &&other) noexcept = default;
    ComResource(const ComResource &other) noexcept = delete;
    auto operator=(const ComResource &other) noexcept -> ComResource & = delete;
    auto operator=(ComResource &&other) noexcept -> ComResource & = default;
#pragma endregion

#pragma region accessors
    [[nodiscard]] auto get() const noexcept -> interface_type *
    {
        return interface_.Get();
    };

    [[nodiscard]] auto getAddressOf() noexcept -> interface_type **
    {
        return interface_.GetAddressOf();
    };

    [[nodiscard]] auto getAddressOf() const noexcept -> interface_type *const *
    {
        return interface_.GetAddressOf();
    };

    auto operator->() const noexcept
    {
        return interface_.operator->();
    };

    explicit operator bool() const noexcept
    {
        return interface_ != nullptr;
    };

#pragma endregion
  protected:
    ComPtr<interface_type> interface_;
};
} // namespace d3d12