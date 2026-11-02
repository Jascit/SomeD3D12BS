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
        PipelineStateConfig config{};
        config.config_.pRootSignature = root_signature.get();
        config.config_.VS.BytecodeLength = vs_shader->GetBufferSize();
        config.config_.VS.pShaderBytecode = vs_shader->GetBufferPointer();
        config.config_.PS.BytecodeLength = ps_shader->GetBufferSize();
        config.config_.PS.pShaderBytecode = ps_shader->GetBufferPointer();
        config.config_.InputLayout = input_layout.desc();
        config.config_.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        config.config_.SampleMask = UINT_MAX;
        config.config_.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        config.config_.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
        config.config_.RasterizerState.FrontCounterClockwise = TRUE;
        config.config_.RasterizerState.DepthClipEnable = TRUE;
        config.config_.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        config.config_.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        config.config_.NumRenderTargets = 1;
        config.config_.SampleDesc.Count = 1;
        config.config_.SampleDesc.Quality = 0;
        config.config_.DepthStencilState.DepthEnable = TRUE;
        config.config_.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        config.config_.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
        config.config_.DepthStencilState.StencilEnable = FALSE;
        config.config_.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        config.config_.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
        config.config_.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        return config;
    }

    [[nodiscard]] D3D12_GRAPHICS_PIPELINE_STATE_DESC get() const noexcept
    {
        return config_;
    }

  private:
    D3D12_GRAPHICS_PIPELINE_STATE_DESC config_;
};

class PipelineStateObject : public ComResource<ID3D12PipelineState>
{
    using base = ComResource;

  public:
    [[nodiscard]] static auto create(ID3D12Device *device, const char *name, const PipelineStateConfig &config) noexcept
        -> PipelineStateObject
    {
        auto desc = config.get();
        PipelineStateObject pso;
        HRESULT result = device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(pso.put()));
        if (FAILED(result))
        {
            core::diagnostics::log_error("Failed to create pso! Name: {}, HRESULT: 0x{:08X}", name,
                                         static_cast<unsigned long>(result));
            return {};
        }
        pso.set_name(name);
        return pso;
    }
};
} // namespace d3d12