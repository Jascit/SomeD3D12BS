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
#include <d3d12.h>
#include <d3d12shader.h>
#include <d3dcompiler.h>
#include <dxcapi.h>
#include <filesystem>
#include <fstream>
#include <vector>
#include <wrl/client.h>
export module d3d12.shaders;
import core.log;
import d3d12.com_resource;
using namespace Microsoft::WRL;

export namespace d3d12
{
enum class TARGET_PROFILE
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
[[nodiscard]] auto read_file(const std::filesystem::path &source_path) noexcept -> std::vector<char>
{
    std::ifstream file(source_path, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        core::diagnostics::log_error("Failed to open file! File path: {}", source_path.string().c_str());
        return {};
    }
    std::int64_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size))
    {
        core::diagnostics::log_error("Failed to read from file! File path: {}", source_path.string().c_str());
    }
    return buffer;
}

[[nodiscard]] auto compile_shader(DxcBuffer source_buffer, const std::filesystem::path &source_path,
                                  IDxcCompiler3 *compiler, LPCWSTR entry_point, LPCWSTR target_profile)
    -> ComPtr<IDxcBlob>
{
    DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));

    LPCWSTR args[] = {source_path.c_str(),
                      L"-E",
                      entry_point,
                      L"-T",
                      target_profile,
#ifdef DEBUG
                      L"-Zi",
                      L"-Qembed_debug"
#endif
    };

    ComPtr<IDxcResult> result;
    compiler->Compile(&source_buffer, args, _countof(args), nullptr, IID_PPV_ARGS(&result));

    ComPtr<IDxcBlobUtf8> errors;
    result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
    if ((errors != nullptr) && errors->GetStringLength() > 0)
    {
        const auto *error_msg = static_cast<const char *>(errors->GetStringPointer());
        core::diagnostics::log_error("Couldn't compile shader {}, with error:\n{}", source_path.string().c_str(),
                                     error_msg);
        return {nullptr};
    }

    ComPtr<IDxcBlob> code;
    result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&code), nullptr);
    return code;
}
constexpr auto shader_profile_to_wstring(d3d12::TARGET_PROFILE profile) noexcept -> const wchar_t *
{
    switch (profile)
    {
    case d3d12::TARGET_PROFILE::Vertex:
        return L"vs_6_0";
    case d3d12::TARGET_PROFILE::Pixel:
        return L"ps_6_0";
    case d3d12::TARGET_PROFILE::Compute:
        return L"cs_6_0";
    case d3d12::TARGET_PROFILE::Geometry:
        return L"gs_6_0";
    case d3d12::TARGET_PROFILE::Hull:
        return L"hs_6_0";
    case d3d12::TARGET_PROFILE::Domain:
        return L"ds_6_0";
    case d3d12::TARGET_PROFILE::Library:
        return L"lib_6_0";
    }
    return L"vs_6_0"; // fallback
}
constexpr auto shader_profile_to_entry(d3d12::TARGET_PROFILE profile) noexcept -> const wchar_t *
{
    switch (profile)
    {
    case d3d12::TARGET_PROFILE::Vertex:
        return L"VSMain";
    case d3d12::TARGET_PROFILE::Pixel:
        return L"PSMain";
    case d3d12::TARGET_PROFILE::Compute:
        return L"CSMain";
    case d3d12::TARGET_PROFILE::Geometry:
        return L"GSMain";
    case d3d12::TARGET_PROFILE::Hull:
        return L"HSMain";
    case d3d12::TARGET_PROFILE::Domain:
        return L"DSMain";
    case d3d12::TARGET_PROFILE::Library:
        return L"LSMain";
    }
    return L"VSMain";
}
} // namespace
auto compile_shader_impl(const std::filesystem::path &source_path, IDxcCompiler3 *compiler, LPCWSTR entry_point,
                         LPCWSTR profile) -> ComPtr<IDxcBlob>
{
    std::vector<char> buffer = read_file(source_path);
    DxcBuffer source_buffer{};
    source_buffer.Encoding = DXC_CP_UTF8;
    source_buffer.Ptr = buffer.data();
    source_buffer.Size = buffer.size();

    return compile_shader(source_buffer, source_path, compiler, entry_point, profile);
}

export namespace d3d12
{
template <TARGET_PROFILE TP> class Shader : public ComResource<IDxcBlob>
{
    using base = ComResource;

  public:
#pragma region rule_of_5
    ~Shader() noexcept = default;
    Shader() noexcept = default;
    Shader(Shader &&other) noexcept = default;
    Shader(const Shader &other) noexcept = delete;
    auto operator=(const Shader &other) noexcept -> Shader & = delete;
    auto operator=(Shader &&other) noexcept -> Shader & = default;
#pragma endregion

    static [[nodiscard]] auto create(const std::filesystem::path &source_path, IDxcCompiler3 *compiler) noexcept
        -> Shader
    {
        return {compile_shader_impl(source_path, compiler, shader_profile_to_entry(TP), shader_profile_to_wstring(TP))};
    }

  private:
    Shader(ComPtr<interface_type> &&other) : base(std::move(other))
    {
    }
};

using VertexShader = Shader<TARGET_PROFILE::Vertex>;
using PixelShader = Shader<TARGET_PROFILE::Pixel>;
using ComputeShader = Shader<TARGET_PROFILE::Compute>;
}; // namespace d3d12