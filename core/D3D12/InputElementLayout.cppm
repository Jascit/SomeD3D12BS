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
#include <wrl/client.h>
export module d3d12.input_element_layout;
import d3d12.shaders;
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
#pragma region rule_of_5
    ~InputElementLayout() noexcept = default;
    InputElementLayout() noexcept = default;
    InputElementLayout(InputElementLayout &&other) noexcept = default;
    InputElementLayout(const InputElementLayout &other) noexcept = delete;
    auto operator=(const InputElementLayout &other) noexcept -> InputElementLayout & = delete;
    auto operator=(InputElementLayout &&other) noexcept -> InputElementLayout & = default;
#pragma endregion

    static auto create(const VertexShader &shader, IDxcUtils *dxc_utils) noexcept -> InputElementLayout
    {
        DxcBuffer reflection_buffer;
        reflection_buffer.Ptr = shader->GetBufferPointer();
        reflection_buffer.Size = shader->GetBufferSize();
        reflection_buffer.Encoding = DXC_CP_ACP;

        ComPtr<ID3D12ShaderReflection> shader_reflection;
        HRESULT result = dxc_utils->CreateReflection(&reflection_buffer, IID_PPV_ARGS(&shader_reflection));

        if (FAILED(result))
        {
            core::diagnostics::log_error("Failed to create reflection of shader! HRESULT: 0x{:08X}",
                                         static_cast<unsigned long>(result));
            return {};
        }

        D3D12_SHADER_DESC shader_desc{};
        result = shader_reflection->GetDesc(&shader_desc);
        if (FAILED(result))
        {
            core::diagnostics::log_error("Failed to retrieve description from shader! HRESULT: 0x{:08X}",
                                         static_cast<unsigned long>(result));
            return {};
        }

        std::uint32_t count_of_parameters = shader_desc.InputParameters;
        auto layout = std::make_unique<D3D12_INPUT_ELEMENT_DESC[]>(count_of_parameters);
        auto semantic_names = std::make_unique<std::unique_ptr<char[]>[]>(count_of_parameters);
        for (std::uint32_t parameter_index = 0; parameter_index < count_of_parameters; parameter_index++)
        {
            D3D12_SIGNATURE_PARAMETER_DESC parameter_signature{};
            result = shader_reflection->GetInputParameterDesc(parameter_index, &parameter_signature);
            if (FAILED(result))
            {
                core::diagnostics::log_error(
                    "Failed to retrieve parameter signature(parameter_index: {}) from shader! HRESULT: 0x{:08X}",
                    parameter_index, static_cast<unsigned long>(result));
                return {};
            }

            D3D12_INPUT_ELEMENT_DESC &input_element = layout[parameter_index];
            input_element.SemanticIndex = parameter_signature.SemanticIndex;
            input_element.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
            input_element.Format = determine_format(parameter_signature.ComponentType, parameter_signature.Mask);
            input_element.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
            input_element.InstanceDataStepRate = 0;
            input_element.InputSlot = 0;

            std::uint64_t name_length = strlen(parameter_signature.SemanticName) + 1;
            semantic_names[parameter_index] = std::make_unique<char[]>(name_length);
            memcpy(semantic_names[parameter_index].get(), parameter_signature.SemanticName, name_length);
            input_element.SemanticName = semantic_names[parameter_index].get();
        }
        return {std::move(layout), std::move(semantic_names), count_of_parameters};
    }
#pragma region accessors
    [[nodiscard]] auto data() const noexcept -> D3D12_INPUT_ELEMENT_DESC *
    {
        return data_.get();
    }

    [[nodiscard]] auto count() const noexcept -> std::uint32_t
    {
        return count_of_elements_;
    }

    explicit operator bool() noexcept
    {
        return data_ != nullptr;
    }

#pragma endregion
  private:
    InputElementLayout(data_t &&other_data, semantic_names_t &&other_names, std::uint32_t count) noexcept
        : data_(std::move(other_data)), semantic_names_(std::move(other_names)), count_of_elements_(count) {};

    semantic_names_t semantic_names_;
    data_t data_;
    std::uint32_t count_of_elements_{};
};
} // namespace d3d12