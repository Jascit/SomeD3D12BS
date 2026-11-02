/* SPDX-License-Identifier: Apache-2.0 */

/**
 * @file    last_error.cppm
 * @brief
 *
 * @date    24/10/2026
 *
 * @copyright Copyright (c) 2026 The Project Contributors
 */
module;
#include <cstdint>
export module core.last_error;

export namespace core::diagnostics
{

enum class Facility : std::uint16_t
{
    Core = 1,
    Memory = 2,
    D3D12 = 3,
    Platform = 4
};

constexpr std::uint64_t make_error(bool is_fatal, Facility facility, std::uint64_t error_code) noexcept
{
    return (static_cast<std::uint64_t>(is_fatal) << 63) |
           (static_cast<std::uint64_t>(static_cast<std::uint16_t>(facility) & 0x7FFF) << 48) |
           (error_code & 0xFFFFFFFFFFFFULL);
};

constexpr bool is_fatal(std::uint64_t error) noexcept
{
    return static_cast<bool>(error >> 63);
}

constexpr Facility get_facility(std::uint64_t error) noexcept
{
    return static_cast<Facility>((error >> 48) & 0x7FFF);
}

constexpr std::uint64_t get_error_code(std::uint64_t error) noexcept
{
    return (error & 0xFFFFFFFFFFFFULL);
}

thread_local std::uint64_t last_error = 0;

__forceinline [[nodiscard]] uint64_t get_last_error() noexcept
{
    return last_error;
}

__forceinline void set_last_error(uint64_t error) noexcept
{
    last_error = error;
}
}; // namespace core::diagnostics
