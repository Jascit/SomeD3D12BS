/* SPDX-License-Identifier: Apache-2.0 */

/**
 * @file    RootSIgnature.cppm
 * @brief
 *
 * @date    24/10/2026
 *
 * @copyright Copyright (c) 2026 The Project Contributors
 */
module;
#include <cassert>
#include <d3d12.h>
#include <wrl/client.h>

#include <optional>
#include <span>

export module d3d12.root_signature;
import core.log;
import d3d12.device;
import d3d12.com_resource;
using namespace Microsoft::WRL;

export namespace d3d12
{

struct RootParameter : D3D12_ROOT_PARAMETER1
{
    RootParameter() noexcept : D3D12_ROOT_PARAMETER1{} {};

    static auto constants(UINT num_32bit_values, UINT shader_register, UINT space = 0,
                          D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept -> RootParameter
    {
        RootParameter p;
        p.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        p.Constants.ShaderRegister = shader_register;
        p.Constants.RegisterSpace = space;
        p.Constants.Num32BitValues = num_32bit_values;
        p.ShaderVisibility = visibility;
        return p;
    };

    static auto descriptorTable(std::span<const D3D12_DESCRIPTOR_RANGE1> ranges,
                                D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept
        -> RootParameter
    {
        RootParameter p;
        p.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        p.DescriptorTable.NumDescriptorRanges = static_cast<UINT>(ranges.size());
        p.DescriptorTable.pDescriptorRanges = ranges.data();
        p.ShaderVisibility = visibility;
        return p;
    };

    static auto descriptor(D3D12_ROOT_PARAMETER_TYPE type, UINT shader_register, UINT space = 0,
                           D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
                           D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept -> RootParameter
    {
        assert(type == D3D12_ROOT_PARAMETER_TYPE_CBV || type == D3D12_ROOT_PARAMETER_TYPE_SRV ||
               type == D3D12_ROOT_PARAMETER_TYPE_UAV);

        RootParameter p;
        p.ParameterType = type;
        p.Descriptor.ShaderRegister = shader_register;
        p.Descriptor.RegisterSpace = space;
        p.Descriptor.Flags = flags;
        p.ShaderVisibility = visibility;

        return p;
    };

    static auto CBV(UINT reg, UINT space = 0, D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
                    D3D12_SHADER_VISIBILITY vis = D3D12_SHADER_VISIBILITY_ALL) noexcept -> RootParameter
    {
        return descriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, reg, space, flags, vis);
    };

    static auto SRV(UINT reg, UINT space = 0, D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
                    D3D12_SHADER_VISIBILITY vis = D3D12_SHADER_VISIBILITY_ALL) noexcept -> RootParameter
    {
        return descriptor(D3D12_ROOT_PARAMETER_TYPE_SRV, reg, space, flags, vis);
    };

    static auto UAV(UINT reg, UINT space = 0, D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
                    D3D12_SHADER_VISIBILITY vis = D3D12_SHADER_VISIBILITY_ALL) noexcept -> RootParameter
    {
        return descriptor(D3D12_ROOT_PARAMETER_TYPE_UAV, reg, space, flags, vis);
    };
};

class RootSignature : public ComResource<ID3D12RootSignature>
{
    using base = ComResource;

  public:
    RootSignature() noexcept = default;
#pragma region factory
    [[nodiscard]] static auto create(const Device &device, const std::span<RootParameter> parameters,
                                     const std::span<D3D12_STATIC_SAMPLER_DESC1> samplers,
                                     D3D12_ROOT_SIGNATURE_FLAGS flags) noexcept -> RootSignature
    {
        D3D12_ROOT_SIGNATURE_DESC2 const descriptor{.NumParameters = static_cast<UINT>(parameters.size()),
                                                    .pParameters = parameters.data(),
                                                    .NumStaticSamplers = static_cast<UINT>(samplers.size()),
                                                    .pStaticSamplers = samplers.data(),
                                                    .Flags = flags};

        D3D12_VERSIONED_ROOT_SIGNATURE_DESC const desc{.Version = D3D_ROOT_SIGNATURE_VERSION_1_2,
                                                       .Desc_1_2 = descriptor};
        ComPtr<ID3DBlob> root_signature_blob;
        ComPtr<ID3DBlob> error_blob;
        if (FAILED(D3D12SerializeVersionedRootSignature(&desc, root_signature_blob.GetAddressOf(),
                                                        error_blob.GetAddressOf())))
        {
            core::diagnostics::log_error("Couldn't serialize root signature. Error message: {}",
                                         reinterpret_cast<const char *>(error_blob->GetBufferPointer()));
            return {nullptr};
        }

        ComPtr<ID3D12RootSignature> root_signature;
        HRESULT result =
            device->CreateRootSignature(0, root_signature_blob->GetBufferPointer(),
                                        root_signature_blob->GetBufferSize(), IID_PPV_ARGS(&root_signature));
        if (FAILED(result))
        {
            core::diagnostics::log_error("Failed to create RootSignature! HRESULT: {}", result);
            return {nullptr};
        }
        return {std::move(root_signature)};
    }
#pragma endregion

  private:
    RootSignature(ComPtr<interface_type> &&other) noexcept : base(std::move(other)) {};
};

} // namespace d3d12
