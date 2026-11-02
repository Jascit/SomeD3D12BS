/* SPDX-License-Identifier: Apache-2.0 */

/**
 * @file    InputElementLayout.cppm
 * @brief
 *
 * @date    27/10/2026
 *
 * @copyright Copyright (c) 2026 The Project Contributors
 */
module;
#include <bit>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxcapi.h>
#include <memory>
#include <string>
#include <vector>
#include <wrl/client.h>
export module d3d12.pipeline:input_element_layout;
import :shaders;
import core.log;

using Microsoft::WRL::ComPtr;
namespace
{
constexpr auto select_format(std::uint32_t count, DXGI_FORMAT format1, DXGI_FORMAT format2, DXGI_FORMAT format3,
                             DXGI_FORMAT format4) noexcept -> DXGI_FORMAT
{
    switch (count)
    {
    case 1:
        return format1;
    case 2:
        return format2;
    case 3:
        return format3;
    case 4:
        return format4;
    default:
        return DXGI_FORMAT_UNKNOWN;
    }
}

constexpr [[nodiscard]] auto determine_format(D3D_REGISTER_COMPONENT_TYPE type, BYTE mask) noexcept -> DXGI_FORMAT
{
    std::int32_t component_count = std::popcount(mask);

    switch (type)
    {
    case D3D_REGISTER_COMPONENT_UNKNOWN:
        return DXGI_FORMAT_UNKNOWN;
    case D3D_REGISTER_COMPONENT_UINT32:
        return select_format(component_count, DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32G32_UINT, DXGI_FORMAT_R32G32B32_UINT,
                             DXGI_FORMAT_R32G32B32A32_UINT);
    case D3D_REGISTER_COMPONENT_SINT32:
        return select_format(component_count, DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32G32_SINT, DXGI_FORMAT_R32G32B32_SINT,
                             DXGI_FORMAT_R32G32B32A32_SINT);
    case D3D_REGISTER_COMPONENT_FLOAT32:
        return select_format(component_count, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32G32_FLOAT,
                             DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT);
    case D3D_REGISTER_COMPONENT_UINT16:
        return select_format(component_count, DXGI_FORMAT_R16_UINT, DXGI_FORMAT_R16G16_UINT,
                             DXGI_FORMAT_R16G16B16A16_UINT, DXGI_FORMAT_R16G16B16A16_UINT);
    case D3D_REGISTER_COMPONENT_SINT16:
        return select_format(component_count, DXGI_FORMAT_R16_SINT, DXGI_FORMAT_R16G16_SINT,
                             DXGI_FORMAT_R16G16B16A16_SINT, DXGI_FORMAT_R16G16B16A16_SINT);
    case D3D_REGISTER_COMPONENT_FLOAT16:
        return select_format(component_count, DXGI_FORMAT_R16_FLOAT, DXGI_FORMAT_R16G16_FLOAT,
                             DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R16G16B16A16_FLOAT);
    case D3D_REGISTER_COMPONENT_UINT64:
        return select_format(component_count, DXGI_FORMAT_R32G32_UINT, DXGI_FORMAT_R32G32B32A32_UINT,
                             DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN);
    case D3D_REGISTER_COMPONENT_SINT64:
        return select_format(component_count, DXGI_FORMAT_R32G32_SINT, DXGI_FORMAT_R32G32B32A32_SINT,
                             DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN);
    case D3D_REGISTER_COMPONENT_FLOAT64:
        return select_format(component_count, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT,
                             DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN);
    }
    return DXGI_FORMAT_UNKNOWN;
}
} // namespace

export namespace d3d12
{
class InputElementLayout
{
    using semantic_names_t = std::unique_ptr<std::unique_ptr<char[]>[]>;
    using data_t = std::unique_ptr<D3D12_INPUT_ELEMENT_DESC[]>;

  public:
    static auto create(const VertexShader &shader, IDxcUtils *dxc_utils) noexcept -> InputElementLayout
    {
        DxcBuffer reflection_buffer;
        reflection_buffer.Ptr = shader->GetBufferPointer();
        reflection_buffer.Size = shader->GetBufferSize();
        reflection_buffer.Encoding = DXC_CP_ACP;

        ComPtr<ID3D12ShaderReflection> reflection;
        HRESULT error = dxc_utils->CreateReflection(&reflection_buffer, IID_PPV_ARGS(&reflection));

        if (FAILED(error))
        {
            core::diagnostics::log_error("Failed to create reflection of shader! HRESULT: 0x{:08X}",
                                         static_cast<unsigned long>(error));
            return {};
        }

        D3D12_SHADER_DESC shader_desc{};
        error = reflection->GetDesc(&shader_desc);
        if (FAILED(error))
        {
            core::diagnostics::log_error("Failed to retrieve description from shader! HRESULT: 0x{:08X}",
                                         static_cast<unsigned long>(error));
            return {};
        }

        InputElementLayout result;
        result.names_.reserve(shader_desc.InputParameters);
        result.elements_.reserve(shader_desc.InputParameters);

        for (UINT i = 0; i < shader_desc.InputParameters; ++i)
        {
            D3D12_SIGNATURE_PARAMETER_DESC param{};
            if (FAILED(reflection->GetInputParameterDesc(i, &param)))
                return {};

            if (param.SystemValueType != D3D_NAME_UNDEFINED)
            {
                continue;
            }

            result.names_.emplace_back(param.SemanticName);

            D3D12_INPUT_ELEMENT_DESC element{};
            element.SemanticIndex = param.SemanticIndex;
            element.Format = determine_format(param.ComponentType, param.Mask);
            element.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
            element.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
            result.elements_.push_back(element);
        }

        for (std::size_t i = 0; i < result.elements_.size(); ++i)
            result.elements_[i].SemanticName = result.names_[i].c_str();

        return result;
    }

    [[nodiscard]] auto desc() const noexcept -> D3D12_INPUT_LAYOUT_DESC
    {
        return {.pInputElementDescs = elements_.data(), .NumElements = static_cast<UINT>(elements_.size())};
    }

    explicit operator bool() const noexcept
    {
        return !elements_.empty();
    }

  private:
    std::vector<std::string> names_;
    std::vector<D3D12_INPUT_ELEMENT_DESC> elements_;
};
} // namespace d3d12