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
    core::diagnostics::Logger::instance().registerSink(std::make_unique<core::diagnostics::ConsoleSink>());
    core::diagnostics::Logger::instance().registerSink(std::make_unique<core::diagnostics::FileSink>("log.txt"));
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

    REQUIRE(vertices.size() == 24);
    REQUIRE(indexes.size() == 36);
    for (auto &vertex : vertices)
    {
        core::diagnostics::log_info("Pos: ({}, {}, {}), Normal: ({}, {}, {}), UV: ({}, {})", vertex.position.x,
                                    vertex.position.y, vertex.position.z, vertex.normal.x, vertex.normal.y,
                                    vertex.normal.z, vertex.textcoord.x, vertex.textcoord.y);
    }
    for (std::uint32_t i = 0; i < indexes.size(); i += 3)
    {
        core::diagnostics::log_info("Triangle n.{}: {}, {}, {}", i / 3, indexes[i], indexes[i + 1], indexes[i + 2]);
    }
}
