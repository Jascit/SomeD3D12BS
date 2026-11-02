/* SPDX-License-Identifier: Apache-2.0 */

/**
 * @file    shaders.cppm
 * @brief
 *
 * @date    26/10/2026
 *
 * @copyright Copyright (c) 2026 The Project Contributors
 */
module;
#include <windows.h>
#include <dxcapi.h>
#include <wrl/client.h>
export module d3d12.pipeline:shaders;
import core.log;
import d3d12.com_resource;
using namespace Microsoft::WRL;

export namespace d3d12
{
using Shader = ComPtr<IDxcBlob>;
using VertexShader = Shader;
using PixelShader = Shader;
using ComputeShader = Shader;
}; // namespace d3d12