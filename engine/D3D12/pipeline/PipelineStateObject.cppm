/* SPDX-License-Identifier: Apache-2.0 */

/**
 * @file    PipelineStateObject.cppm
 * @brief
 *
 * @date    26/10/2026
 *
 * @copyright Copyright (c) 2026 The Project Contributors
 */
module;
#include <cstdint>
#include <d3d12.h>
#include <type_traits>
#include <wrl/client.h>
export module d3d12.pipeline:pipeline_state_object;
import :root_signature;
import :shaders;
import :input_element_layout;
import d3d12.com_resource;
import core.log;
using Microsoft::WRL::ComPtr;

export namespace d3d12
{
class PipelineStateConfig
{
  public:
    PipelineStateConfig() noexcept = default;

    [[nodiscard]] static PipelineStateConfig defaults(const InputElementLayout &input_layout,
                                                      const RootSignature &root_signature,
                                                      const VertexShader &vs_shader,
                                                      const PixelShader &ps_shader) noexcept
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc{};
        pso_desc.pRootSignature = root_signature.get();

        pso_desc.VS.BytecodeLength = vs_shader->GetBufferSize();
        pso_desc.VS.pShaderBytecode = vs_shader->GetBufferPointer();
        pso_desc.PS.BytecodeLength = ps_shader->GetBufferSize();
        pso_desc.PS.pShaderBytecode = ps_shader->GetBufferPointer();

        pso_desc.InputLayout = input_layout.desc();

        pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

        pso_desc.SampleMask = UINT_MAX;

        pso_desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        pso_desc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
        pso_desc.RasterizerState.FrontCounterClockwise = TRUE;
        pso_desc.RasterizerState.DepthClipEnable = TRUE;

        pso_desc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

        pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        pso_desc.NumRenderTargets = 1;

        pso_desc.SampleDesc.Count = 1;
        pso_desc.SampleDesc.Quality = 0;

        pso_desc.DepthStencilState.DepthEnable = TRUE;
        pso_desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        pso_desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
        pso_desc.DepthStencilState.StencilEnable = FALSE;
        pso_desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

        pso_desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
        pso_desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        return {pso_desc};
    }

    [[nodiscard]] D3D12_GRAPHICS_PIPELINE_STATE_DESC get() const noexcept
    {
        return config_;
    }

  private:
    PipelineStateConfig(const D3D12_GRAPHICS_PIPELINE_STATE_DESC &desc) noexcept : config_(desc) {};
    D3D12_GRAPHICS_PIPELINE_STATE_DESC config_;
};

class PipelineStateObject : public ComResource<ID3D12PipelineState>
{
    using base = ComResource;

  public:
    [[nodiscard]] static auto create(ID3D12Device *device, const char *name,
                                     const PipelineStateConfig &config) noexcept -> PipelineStateObject
    {
        auto desc = config.get();
        ComPtr<ID3D12PipelineState> pso;
        HRESULT result = device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&pso));
        if (FAILED(result))
        {
            core::diagnostics::log_error("Failed to create pso! HRESULT: 0x{:08X}", static_cast<unsigned long>(result));
            return {nullptr, nullptr};
        }

        return {std::move(pso), name};
    }

  private:
    PipelineStateObject(ComPtr<interface_type> &&other, const char *name) noexcept : base(std::move(other), name) {};
};
} // namespace d3d12