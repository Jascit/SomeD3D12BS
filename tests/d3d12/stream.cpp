/* SPDX-License-Identifier: Apache-2.0 */

/**
 * @file    stream.cpp
 * @brief
 *
 * @date    29/10/2026
 *
 * @copyright Copyright (c) 2026 The Project Contributors
 */

#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <windows.h>
import core.log;
import d3d12.obj_parser;

TEST_CASE("Checking sstream", "[fstream]")
{
    char exe_path[MAX_PATH];
    GetModuleFileNameA(nullptr, exe_path, MAX_PATH);
    std::filesystem::path cube_path(exe_path);
    cube_path = cube_path.parent_path().parent_path().parent_path() / "models\\Cube.obj";
    std::ifstream file(cube_path, std::ios::binary);
    if (!file.is_open())
    {
        core::diagnostics::log_error("Failed to open the File! File path: {}", cube_path.string().c_str());
    }

    REQUIRE(file.is_open());
}
