/* SPDX-License-Identifier: Apache-2.0 */

/**
 * @file    ShaderCompiler.cppm
 * @brief
 *
 * @date    01/11/2026
 *
 * @copyright Copyright (c) 2026 The Project Contributors
 */
module;
#include <combaseapi.h>
#include <dxcapi.h>
#include <filesystem>
#include <type_traits>
#include <wrl/client.h>
export module d3d12.pipeline:shader_compiler;
import core.log;
import :shaders;

using Microsoft::WRL::ComPtr;
export namespace d3d12
{
enum class SHADER_TYPE
{
    Vertex,
    Pixel,
    Compute,
    Geometry,
    Hull,
    Domain,
    Library
};
} // namespace d3d12
namespace
{

constexpr auto shader_type_to_profile(d3d12::SHADER_TYPE shader_type) noexcept -> const wchar_t *
{
    switch (shader_type)
    {
    case d3d12::SHADER_TYPE::Vertex:
        return L"vs_6_0";
    case d3d12::SHADER_TYPE::Pixel:
        return L"ps_6_0";
    case d3d12::SHADER_TYPE::Compute:
        return L"cs_6_0";
    case d3d12::SHADER_TYPE::Geometry:
        return L"gs_6_0";
    case d3d12::SHADER_TYPE::Hull:
        return L"hs_6_0";
    case d3d12::SHADER_TYPE::Domain:
        return L"ds_6_0";
    case d3d12::SHADER_TYPE::Library:
        return L"lib_6_0";
    }
    return L"vs_6_0"; // fallback
}
constexpr auto shader_type_to_entry(d3d12::SHADER_TYPE shader_type) noexcept -> const wchar_t *
{
    switch (shader_type)
    {
    case d3d12::SHADER_TYPE::Vertex:
        return L"VSMain";
    case d3d12::SHADER_TYPE::Pixel:
        return L"PSMain";
    case d3d12::SHADER_TYPE::Compute:
        return L"CSMain";
    case d3d12::SHADER_TYPE::Geometry:
        return L"GSMain";
    case d3d12::SHADER_TYPE::Hull:
        return L"HSMain";
    case d3d12::SHADER_TYPE::Domain:
        return L"DSMain";
    case d3d12::SHADER_TYPE::Library:
        return L"LSMain";
    }
    return L"VSMain";
}
} // namespace

export namespace d3d12
{

class ShaderCompiler
{
  public:
    static [[nodiscard]] ShaderCompiler create() noexcept
    {
        ShaderCompiler compiler;
        HRESULT result =
            DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(compiler.compiler_.ReleaseAndGetAddressOf()));
        if (FAILED(result))
        {
            core::diagnostics::log_error("Failed to create dxc compiler! HRESULT: 0x{:08X}",
                                         static_cast<unsigned long>(result));
            return {};
        }
        return compiler;
    }

    [[nodiscard]] auto compile_shader(const std::vector<BYTE> &data, SHADER_TYPE shader_type) -> Shader
    {
        LPCWSTR args[] = {
            L"",    L"-E",           shader_type_to_entry(shader_type), L"-T", shader_type_to_profile(shader_type),
#ifdef DEBUG
            L"-Zi", L"-Qembed_debug"
#endif
        };

        DxcBuffer source_buffer{};
        source_buffer.Ptr = data.data();
        source_buffer.Size = data.size();
        source_buffer.Encoding = 0U;
        ComPtr<IDxcResult> result;
        compiler_->Compile(&source_buffer, args, _countof(args), nullptr, IID_PPV_ARGS(&result));

        ComPtr<IDxcBlobUtf8> errors;
        result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
        if ((errors != nullptr) && errors->GetStringLength() > 0)
        {
            auto error_msg = static_cast<const char *>(errors->GetStringPointer());
            core::diagnostics::log_error("Couldn't compile shader (Type: {}), with error:\n{}",
                                         static_cast<std::uint32_t>(shader_type), error_msg);
            return {nullptr};
        }

        ComPtr<IDxcBlob> code;
        result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&code), nullptr);
        return code;
    }

  private:
    ComPtr<IDxcCompiler3> compiler_;
};
} // namespace d3d12
