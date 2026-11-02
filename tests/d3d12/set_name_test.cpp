/* SPDX-License-Identifier: Apache-2.0 */

/**
 * @file    set_name_test.cpp
 * @brief
 *
 * @date    31/10/2026
 *
 * @copyright Copyright (c) 2026 The Project Contributors
 */

#include <Windows.h>
#include <catch2/catch_test_macros.hpp>
#include <iostream>

[[nodiscard]] auto to_wide(const char *utf8) noexcept -> std::wstring
{
    std::uint32_t length = std::strlen(utf8);
    std::uint32_t wlength = MultiByteToWideChar(CP_UTF8, 0, utf8, length, nullptr, 0);
    std::unique_ptr<wchar_t[]> wide_name = std::make_unique<wchar_t[]>(wlength + 1);
    MultiByteToWideChar(CP_UTF8, 0, utf8, length, wide_name.get(), wlength);
    wide_name[wlength] = L'\0';
    return {};
}

TEST_CASE("set_name", "[utils]")
{
    const char fuck[] = "fuck";
    to_wide(fuck);
}