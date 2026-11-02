/* SPDX-License-Identifier: Apache-2.0 */

/**
 * @file    UploadBuffer.cppm
 * @brief
 *
 * @date    31/10/2026
 *
 * @copyright Copyright (c) 2026 The Project Contributors
 */
module;
#include <cstdint>
#include <d3d12.h>
export module d3d12.resources:upload_buffer;
import d3d12.com_resource;

export namespace d3d12
{
struct UploadBufferDesc
{
    std::uint64_t total_bytes;
    const char *name;
};

using UploadBuffer = ComResource<ID3D12Resource>;
} // namespace d3d12