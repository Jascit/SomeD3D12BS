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
#include <optional>
#include <vector>
#include <wrl/client.h>

export module d3d12.pipeline:root_signature;
import core.log;
import d3d12.com_resource;
using namespace Microsoft::WRL;

export namespace d3d12
{
template <typename T> class DescList
{
    using descriptor_type = T::descriptor_type;

  public:
    DescList(std::initializer_list<T> descs) noexcept
    {
        parameters_.reserve(descs.size());
        for (const T &d : descs)
        {
            parameters_.push_back(d.desc());
        }
    }

    [[nodiscard]] descriptor_type &operator[](std::uint32_t index) noexcept
    {
        return parameters_[index];
    }

    void push(const T &parameter) noexcept
    {
        parameters_.push_back(parameter.desc());
    }

    [[nodiscard]] const std::vector<descriptor_type> &get() const noexcept
    {
        return parameters_;
    }

  private:
    std::vector<descriptor_type> parameters_;
};

class DescriptorRangeDesc
{
  public:
    using descriptor_type = D3D12_DESCRIPTOR_RANGE1;

    [[nodiscard]] static DescriptorRangeDesc srv(
        std::uint32_t reg, std::uint32_t space, std::uint32_t count,
        std::uint32_t offset = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND,
        D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE) noexcept
    {
        return create(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, reg, space, count, offset, flags);
    }
    [[nodiscard]] static DescriptorRangeDesc uav(
        std::uint32_t reg, std::uint32_t space, std::uint32_t count,
        std::uint32_t offset = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND,
        D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE) noexcept
    {
        return create(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, reg, space, count, offset, flags);
    }
    [[nodiscard]] static DescriptorRangeDesc cbv(
        std::uint32_t reg, std::uint32_t space, std::uint32_t count,
        std::uint32_t offset = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND,
        D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE) noexcept
    {
        return create(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, reg, space, count, offset, flags);
    }
    [[nodiscard]] static DescriptorRangeDesc sampler(
        std::uint32_t reg, std::uint32_t space, std::uint32_t count,
        std::uint32_t offset = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND,
        D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE) noexcept
    {
        return create(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, reg, space, count, offset, flags);
    }

    [[nodiscard]] D3D12_DESCRIPTOR_RANGE1 desc() const noexcept
    {
        return desc_;
    }

  private:
    static DescriptorRangeDesc create(D3D12_DESCRIPTOR_RANGE_TYPE type, std::uint32_t reg, std::uint32_t space,
                                      std::uint32_t count, std::uint32_t offset,
                                      D3D12_DESCRIPTOR_RANGE_FLAGS flags) noexcept
    {
        DescriptorRangeDesc range{};
        range.desc_.Flags = flags;
        range.desc_.RegisterSpace = space;
        range.desc_.RangeType = type;
        range.desc_.BaseShaderRegister = reg;
        range.desc_.NumDescriptors = count;
        range.desc_.OffsetInDescriptorsFromTableStart = offset;
        return range;
    }

    DescriptorRangeDesc() noexcept = default;

    D3D12_DESCRIPTOR_RANGE1 desc_;
};

using DescriptorRanges = DescList<DescriptorRangeDesc>;

class RootParameterDesc
{
  public:
    using descriptor_type = D3D12_ROOT_PARAMETER1;

    static auto constants(std::uint32_t num_32bit_values, std::uint32_t shader_register, std::uint32_t space = 0,
                          D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept
        -> RootParameterDesc
    {
        RootParameterDesc p;
        p.desc_.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        p.desc_.Constants.ShaderRegister = shader_register;
        p.desc_.Constants.RegisterSpace = space;
        p.desc_.Constants.Num32BitValues = num_32bit_values;
        p.desc_.ShaderVisibility = visibility;
        return p;
    }

    static auto descriptor_table(const DescriptorRanges &ranges,
                                 D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept
        -> RootParameterDesc
    {
        RootParameterDesc p;
        p.desc_.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        p.desc_.DescriptorTable.NumDescriptorRanges = static_cast<std::uint32_t>(ranges.get().size());
        p.desc_.DescriptorTable.pDescriptorRanges = ranges.get().data();
        p.desc_.ShaderVisibility = visibility;
        return p;
    };

    static auto descriptor(D3D12_ROOT_PARAMETER_TYPE type, std::uint32_t shader_register, std::uint32_t space = 0,
                           D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
                           D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept
        -> RootParameterDesc
    {
        assert(type == D3D12_ROOT_PARAMETER_TYPE_CBV || type == D3D12_ROOT_PARAMETER_TYPE_SRV ||
               type == D3D12_ROOT_PARAMETER_TYPE_UAV);

        RootParameterDesc p;
        p.desc_.ParameterType = type;
        p.desc_.Descriptor.ShaderRegister = shader_register;
        p.desc_.Descriptor.RegisterSpace = space;
        p.desc_.Descriptor.Flags = flags;
        p.desc_.ShaderVisibility = visibility;

        return p;
    }

    static auto cbv(std::uint32_t reg, std::uint32_t space = 0,
                    D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
                    D3D12_SHADER_VISIBILITY vis = D3D12_SHADER_VISIBILITY_ALL) noexcept -> RootParameterDesc
    {
        return descriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, reg, space, flags, vis);
    }

    static auto srv(std::uint32_t reg, std::uint32_t space = 0,
                    D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
                    D3D12_SHADER_VISIBILITY vis = D3D12_SHADER_VISIBILITY_ALL) noexcept -> RootParameterDesc
    {
        return descriptor(D3D12_ROOT_PARAMETER_TYPE_SRV, reg, space, flags, vis);
    }

    static auto uav(std::uint32_t reg, std::uint32_t space = 0,
                    D3D12_ROOT_DESCRIPTOR_FLAGS flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
                    D3D12_SHADER_VISIBILITY vis = D3D12_SHADER_VISIBILITY_ALL) noexcept -> RootParameterDesc
    {
        return descriptor(D3D12_ROOT_PARAMETER_TYPE_UAV, reg, space, flags, vis);
    }

    [[nodiscard]] D3D12_ROOT_PARAMETER1 desc() const noexcept
    {
        return desc_;
    }

  private:
    RootParameterDesc() noexcept = default;
    D3D12_ROOT_PARAMETER1 desc_{};
};

class RootSamplerDesc
{

  public:
    using descriptor_type = D3D12_STATIC_SAMPLER_DESC1;
    [[nodiscard]] static RootSamplerDesc point(std::uint32_t reg, std::uint32_t space = 0,
                                               D3D12_TEXTURE_ADDRESS_MODE mode = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                               D3D12_SHADER_VISIBILITY vis = D3D12_SHADER_VISIBILITY_ALL) noexcept
    {
        return create(D3D12_FILTER_MIN_MAG_MIP_POINT, reg, space, 1, mode, mode, mode, 0.0f,
                      D3D12_COMPARISON_FUNC_NEVER, D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK, 0, D3D12_FLOAT32_MAX, vis,
                      D3D12_SAMPLER_FLAG_NONE);
    }
    [[nodiscard]] static RootSamplerDesc linear(std::uint32_t reg, std::uint32_t space = 0,
                                                D3D12_TEXTURE_ADDRESS_MODE mode = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                                D3D12_SHADER_VISIBILITY vis = D3D12_SHADER_VISIBILITY_ALL) noexcept
    {

        return create(D3D12_FILTER_MIN_MAG_MIP_LINEAR, reg, space, 1, mode, mode, mode, 0.0f,
                      D3D12_COMPARISON_FUNC_NEVER, D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK, 0, D3D12_FLOAT32_MAX, vis,
                      D3D12_SAMPLER_FLAG_NONE);
        ;
    }

    [[nodiscard]] static RootSamplerDesc anisotropic(std::uint32_t reg, std::uint32_t space = 0,
                                                     std::uint32_t max_anisotropy = 16,
                                                     D3D12_TEXTURE_ADDRESS_MODE mode = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                                     D3D12_SHADER_VISIBILITY vis = D3D12_SHADER_VISIBILITY_ALL) noexcept
    {
        return create(D3D12_FILTER_ANISOTROPIC, reg, space, max_anisotropy, mode, mode, mode, 0.0f,
                      D3D12_COMPARISON_FUNC_NEVER, D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK, 0, D3D12_FLOAT32_MAX, vis,
                      D3D12_SAMPLER_FLAG_NONE);
    }

    [[nodiscard]] D3D12_STATIC_SAMPLER_DESC1 desc() const noexcept
    {
        return desc_;
    }

  private:
    static RootSamplerDesc create(D3D12_FILTER filter, std::uint32_t reg, std::uint32_t space,
                                  std::uint32_t max_anisotropy, D3D12_TEXTURE_ADDRESS_MODE u,
                                  D3D12_TEXTURE_ADDRESS_MODE v, D3D12_TEXTURE_ADDRESS_MODE w, float mip_lod_bias,
                                  D3D12_COMPARISON_FUNC compar, D3D12_STATIC_BORDER_COLOR border, float min_lod,
                                  float max_lod, D3D12_SHADER_VISIBILITY vis, D3D12_SAMPLER_FLAGS flags) noexcept
    {
        RootSamplerDesc sampler_desc;
        sampler_desc.desc_.AddressU = u;
        sampler_desc.desc_.AddressV = v;
        sampler_desc.desc_.AddressW = w;
        sampler_desc.desc_.ShaderRegister = reg;
        sampler_desc.desc_.RegisterSpace = space;
        sampler_desc.desc_.Filter = filter;
        sampler_desc.desc_.BorderColor = border;
        sampler_desc.desc_.MinLOD = min_lod;
        sampler_desc.desc_.MaxLOD = max_lod;
        sampler_desc.desc_.ComparisonFunc = compar;
        sampler_desc.desc_.MaxAnisotropy = max_anisotropy;
        sampler_desc.desc_.MipLODBias = mip_lod_bias;
        sampler_desc.desc_.Flags = flags;
        sampler_desc.desc_.ShaderVisibility = vis;
        return sampler_desc;
    }
    RootSamplerDesc() noexcept = default;
    D3D12_STATIC_SAMPLER_DESC1 desc_{.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                     .AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                     .AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP};
};

using RootParameters = DescList<RootParameterDesc>;
using RootSamplers = DescList<RootSamplerDesc>;

class RootSignature : public ComResource<ID3D12RootSignature>
{
    using base = ComResource;

  public:
    [[nodiscard]] static auto create(ID3D12Device *device, const char *name, const RootParameters &root_parameters,
                                     const RootSamplers &root_samplers, D3D12_ROOT_SIGNATURE_FLAGS flags) noexcept
        -> RootSignature
    {
        D3D12_ROOT_SIGNATURE_DESC2 const descriptor{
            .NumParameters = static_cast<std::uint32_t>(root_parameters.get().size()),
            .pParameters = root_parameters.get().data(),
            .NumStaticSamplers = static_cast<std::uint32_t>(root_samplers.get().size()),
            .pStaticSamplers = root_samplers.get().data(),
            .Flags = flags};

        D3D12_VERSIONED_ROOT_SIGNATURE_DESC const desc{.Version = D3D_ROOT_SIGNATURE_VERSION_1_2,
                                                       .Desc_1_2 = descriptor};
        ComPtr<ID3DBlob> root_signature_blob;
        ComPtr<ID3DBlob> error_blob;
        if (FAILED(D3D12SerializeVersionedRootSignature(&desc, root_signature_blob.GetAddressOf(),
                                                        error_blob.GetAddressOf())))
        {
            core::diagnostics::log_error(
                "Failed to serialize root signature! Name: {}, Error message: {}", name,
                reinterpret_cast<const char *>(error_blob ? error_blob->GetBufferPointer() : "Unknown"));
            return {};
        }

        base root_signature;
        HRESULT result =
            device->CreateRootSignature(0, root_signature_blob->GetBufferPointer(),
                                        root_signature_blob->GetBufferSize(), IID_PPV_ARGS(root_signature.put()));
        if (FAILED(result))
        {
            core::diagnostics::log_error("Failed to create RootSignature! Name: {}, HRESULT: 0x{:08X}", name,
                                         static_cast<std::uint32_t>(result));
            return {};
        }
        root_signature.set_name(name);
        return {std::move(root_signature)};
    }
};

} // namespace d3d12
