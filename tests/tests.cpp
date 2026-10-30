/* SPDX-License-Identifier: Apache-2.0 */

/**
 * @file    tests.cpp
 * @brief
 *
 * @date    10/10/2026
 *
 * @copyright Copyright (c) 2026 The Project Contributors
 */
#include <catch2/catch_test_macros.hpp>
#include <iostream>
#include <windows.h>
#include <filesystem>

TEST_CASE("Checking GetModuleFileNameW", "[windows]") {
  wchar_t path_buffer[MAX_PATH];
  GetModuleFileNameW(nullptr, path_buffer, MAX_PATH);
  std::filesystem::path test_path(path_buffer);
  INFO(test_path.string() << "\n" << test_path.parent_path().parent_path().parent_path().string());
}
