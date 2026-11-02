/* SPDX-License-Identifier: Apache-2.0 */

/**
 * @file    ObjParser.cppm
 * @brief
 *
 * @date    29/10/2026
 *
 * @copyright Copyright (c) 2026 The Project Contributors
 */
module;
#include <filesystem>
#include <fstream>
#include <sstream>
#include <utility>
#include <vector>
export module assets.obj_parser;
import core.log;

export namespace assets
{

struct Float3
{
    float x;
    float y;
    float z;
};
struct Float2
{
    float x;
    float y;
};

using position_t = Float3;
using normal_t = Float3;
using textcoord_t = Float2;

struct Vertex
{
    position_t position;
    normal_t normal;
    textcoord_t textcoord;
};
using vertex_index = std::uint32_t;
class ObjParser
{
  public:
    static [[nodiscard]] std::pair<std::vector<Vertex>, std::vector<vertex_index>> parse(
        const std::filesystem::path &path)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open())
        {
            core::diagnostics::log_error("Failed to open the File! File path: {}", path.string().c_str());
            return {};
        }
        std::vector<Vertex> vertices;
        std::vector<vertex_index> indexes;

        std::vector<position_t> positions;
        std::vector<normal_t> normals;
        std::vector<textcoord_t> textcoords;

        std::string line;
        while (std::getline(file, line))
        {
            std::istringstream stream(line);
            std::string tag;
            stream >> tag;
            if (tag == "v")
            {
                position_t pos;
                stream >> pos.x;
                stream >> pos.y;
                stream >> pos.z;
                positions.push_back(pos);
            }
            if (tag == "vn")
            {
                normal_t normal;
                stream >> normal.x;
                stream >> normal.y;
                stream >> normal.z;
                normals.push_back(normal);
            }
            if (tag == "vt")
            {
                textcoord_t textcoord;
                float buffer_y = 0;
                stream >> textcoord.x;
                stream >> buffer_y;
                textcoord.y = 1 - buffer_y;
                textcoords.push_back(textcoord);
            }
            if (tag == "f")
            {
                Vertex quad[4];
                for (int i = 0; i < 4; i++)
                {
                    std::uint32_t triangle_indexes[3];
                    std::string triangle_description;
                    stream >> triangle_description;
                    triangle_description.push_back('/');

                    std::uint32_t current_pos = 0; // not /; its either posof('/')+1 or start or end("\0")
                    for (int j = 0; j < 3; j++)
                    {
                        std::uint32_t char_count = 0; // how many chars is there until / symbol
                        for (; triangle_description[current_pos + char_count] != '/'; char_count++)
                        {
                        }
                        std::string num_string = triangle_description.substr(current_pos, char_count);
                        triangle_indexes[j] = std::stoi(num_string);
                        current_pos += char_count + 1;
                    }
                    quad[i].position = positions[triangle_indexes[0] - 1];
                    quad[i].textcoord = textcoords[triangle_indexes[1] - 1];
                    quad[i].normal = normals[triangle_indexes[2] - 1];
                }

                // CCW: 0->3->2->1 CW: 0->1->2->3
                std::uint32_t start_index = static_cast<std::uint32_t>(vertices.size());
                vertices.push_back(quad[0]);
                vertices.push_back(quad[3]);
                vertices.push_back(quad[2]);
                vertices.push_back(quad[1]);
                indexes.push_back(start_index);
                indexes.push_back(start_index + 1);
                indexes.push_back(start_index + 2);
                indexes.push_back(start_index);
                indexes.push_back(start_index + 2);
                indexes.push_back(start_index + 3);
            }
        }
        return std::make_pair(std::move(vertices), std::move(indexes));
    }
};
} // namespace assets