#include <DirectXMath.h>
#include <Windows.h>
#include <d3d12.h>
#include <dxcapi.h>
#include <dxgi1_6.h>
#include <filesystem>
#include <fstream>
#include <memory>
#include <wrl/client.h>
import core.log;
import platform.window;
import core.last_error;
import d3d12.context;
import d3d12.resources;
import d3d12.descriptor_heap;
import d3d12.frame_manager;
import d3d12.utils;
import d3d12.pipeline;
import d3d12.swapchain;
import assets.obj_parser;
using Microsoft::WRL::ComPtr;
using namespace DirectX;

[[nodiscard]] auto read_file(const std::filesystem::path &source_path) noexcept -> std::vector<BYTE>
{
    std::ifstream file(source_path, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        core::diagnostics::log_error("Failed to open file! File path: {}", source_path.string().c_str());
        return {};
    }
    std::int64_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<BYTE> buffer(size);
    if (!file.read(reinterpret_cast<char *>(buffer.data()), size))
    {
        core::diagnostics::log_error("Failed to read from file! File path: {}", source_path.string().c_str());
    }
    return buffer;
}

constexpr std::uint32_t buffer_count = 2;

int main()
{
#pragma region initialization
    const d3d12::utils::Reporter reporter;
    core::diagnostics::Logger::instance().register_sink(std::make_unique<core::diagnostics::ConsoleSink>());
    d3d12::utils::enable_d3d12_debug_layer();
    auto window = core::Window::create(L"Yay window!!", L"some bs class name for my klein Fenster");
    window.show();

    ComPtr<IDxcUtils> dxc_utils;
    HRESULT result = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxc_utils));
    if (FAILED(result))
    {
        core::diagnostics::log_error("Failed to create dxc utils! HRESULT: 0x{:08X}",
                                     static_cast<unsigned long>(result));
        return 1;
    }

    d3d12::ShaderCompiler shader_compiler = d3d12::ShaderCompiler::create();
    d3d12::Context renderer_context = d3d12::Context::create();
    d3d12::ResourceFactory resource_factory = d3d12::ResourceFactory::create(renderer_context.device());
#pragma endregion

#pragma region root_signature
    d3d12::DescriptorRanges descriptor_ranges = {d3d12::DescriptorRangeDesc::srv(0, 0, 1)};
    d3d12::RootParameters root_parameters = {
        d3d12::RootParameterDesc::constants(16, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX),
        d3d12::RootParameterDesc::descriptor_table(descriptor_ranges, D3D12_SHADER_VISIBILITY_PIXEL),
        d3d12::RootParameterDesc::constants(1, 1, 0, D3D12_SHADER_VISIBILITY_PIXEL)};

    d3d12::RootSamplers root_samplers = {d3d12::RootSamplerDesc::linear(0)};

    d3d12::RootSignature root_signature = d3d12::RootSignature::create(
        renderer_context.device(), "graphics_root_signature", root_parameters, root_samplers,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS);

    d3d12::RootParameters compute_parameters = {d3d12::RootParameterDesc::uav(0)};
    d3d12::RootSignature compute_root_signature = d3d12::RootSignature::create(
        renderer_context.device(), "compute_root_signature", compute_parameters, {}, D3D12_ROOT_SIGNATURE_FLAG_NONE);
#pragma endregion

#pragma region shaders
    wchar_t exe_path[MAX_PATH];
    GetModuleFileNameW(nullptr, exe_path, MAX_PATH);
    std::filesystem::path shader_directory(exe_path);
    shader_directory = shader_directory.parent_path().parent_path().parent_path() / L"engine\\shaders";
    const std::filesystem::path ps_path = shader_directory / L"ps.hlsl";
    const std::filesystem::path cs_path = shader_directory / L"cs.hlsl";
    const std::filesystem::path vs_path = shader_directory / L"vs.hlsl";

    std::vector<BYTE> vs_data = read_file(vs_path);
    std::vector<BYTE> ps_data = read_file(ps_path);
    std::vector<BYTE> cs_data = read_file(cs_path);
    d3d12::VertexShader vs_shader = shader_compiler.compile_shader(vs_data, d3d12::SHADER_TYPE::Vertex);
    d3d12::PixelShader ps_shader = shader_compiler.compile_shader(ps_data, d3d12::SHADER_TYPE::Pixel);
    d3d12::ComputeShader cs_shader = shader_compiler.compile_shader(cs_data, d3d12::SHADER_TYPE::Compute);
#pragma endregion

#pragma region commands_related
    d3d12::CommandAllocator command_allocators[buffer_count];
    char name[] = "graphics_command_allocator  ";
    for (uint32_t i = 0; i < buffer_count; i++)
    {
        name[sizeof(name) - 2] = static_cast<char>('0' + i);
        command_allocators[i] = d3d12::CommandAllocator::create(renderer_context.device(), name);
        if (!command_allocators[i])
        {
            return 1;
        }
    }

    d3d12::CommandList command_list =
        d3d12::CommandList::create(renderer_context.device(), command_allocators[0], "graphics_command_list");
    if (!command_list)
    {
        return 1;
    }

#pragma endregion

#pragma region swapchain
    d3d12::RTVDescriptorHeap rtv_descriptor_heap =
        d3d12::RTVDescriptorHeap::create(renderer_context.device(), "rtv_descriptor_heap", buffer_count);
    if (!rtv_descriptor_heap)
    {
        return 1;
    }
    const d3d12::SwapChainConfig swap_chain_config =
        d3d12::SwapChainConfig::makeDefault(window.get_width(), window.get_height());
    auto swap_chain = d3d12::SwapChain::create(std::move(rtv_descriptor_heap), &renderer_context, "swap_chain",
                                               swap_chain_config, window.get_system_handle());
    if (!swap_chain)
    {
        return 1;
    }
#pragma endregion swapchain

#pragma region pso
    d3d12::InputElementLayout layout = d3d12::InputElementLayout::create(vs_shader, dxc_utils.Get());
    if (!layout)
    {
        return 1;
    }

    d3d12::PipelineStateConfig pso_config =
        d3d12::PipelineStateConfig::defaults(layout, root_signature, vs_shader, ps_shader);
    d3d12::PipelineStateObject pso =
        d3d12::PipelineStateObject::create(renderer_context.device(), "graphics_pso", pso_config);
    if (!pso)
    {
        return 1;
    }

    D3D12_COMPUTE_PIPELINE_STATE_DESC compute_pso_desc{};
    compute_pso_desc.CS.BytecodeLength = cs_shader->GetBufferSize();
    compute_pso_desc.CS.pShaderBytecode = cs_shader->GetBufferPointer();
    compute_pso_desc.pRootSignature = compute_root_signature.get();
    ComPtr<ID3D12PipelineState> compute_pso;
    result = renderer_context.device()->CreateComputePipelineState(&compute_pso_desc, IID_PPV_ARGS(&compute_pso));
    if (FAILED(result))
    {
        core::diagnostics::log_error("Failed to create compute pipeline state! HRESULT: ",
                                     static_cast<std::uint64_t>(result));
        return 1;
    }
#pragma endregion

#pragma region cube
    std::filesystem::path cube_path(exe_path);
    cube_path = cube_path.parent_path().parent_path().parent_path() / "models\\Cube.obj";
    auto [vertex_data, index_data] = assets::ObjParser::parse(cube_path);
    std::uint32_t vertex_buffer_size = static_cast<std::uint32_t>(vertex_data.size() * sizeof(assets::Vertex));
    std::uint32_t indexes_count = static_cast<std::uint32_t>(index_data.size());
    std::uint32_t index_buffer_size = static_cast<std::uint32_t>(indexes_count * sizeof(assets::vertex_index));

    d3d12::UploadBuffer vertex_buffer =
        resource_factory.create_upload_buffer({.total_bytes = vertex_buffer_size, .name = "vertex_upload_buffer"});

    D3D12_RANGE read_range = {0, 0};
    void *p_void = nullptr;
    vertex_buffer->Map(0, &read_range, &p_void);
    memcpy(p_void, vertex_data.data(), vertex_buffer_size);
    vertex_buffer->Unmap(0, nullptr);

    D3D12_VERTEX_BUFFER_VIEW vb_view;
    vb_view.BufferLocation = vertex_buffer->GetGPUVirtualAddress();
    vb_view.SizeInBytes = vertex_buffer_size;
    vb_view.StrideInBytes = sizeof(assets::Vertex);

    d3d12::UploadBuffer index_buffer =
        resource_factory.create_upload_buffer({.total_bytes = index_buffer_size, .name = "index_upload_buffer"});

    index_buffer->Map(0, &read_range, &p_void);
    memcpy(p_void, index_data.data(), index_buffer_size);
    index_buffer->Unmap(0, nullptr);

    D3D12_INDEX_BUFFER_VIEW ib_view;
    ib_view.BufferLocation = index_buffer->GetGPUVirtualAddress();
    ib_view.SizeInBytes = index_buffer_size;
    ib_view.Format = DXGI_FORMAT_R32_UINT;

#pragma endregion

#pragma region srv
    std::uint32_t texture_width = 256;
    std::uint32_t texture_height = 256;
    std::vector<std::uint32_t> texture_data = d3d12::utils::generate_rainbow_texture(texture_width, texture_height);

    d3d12::Texture2D rainbow_texture = resource_factory.create_texture_2d({.width = texture_width,
                                                                           .height = texture_height,
                                                                           .format = DXGI_FORMAT_R8G8B8A8_UNORM,
                                                                           .mip_levels = 1,
                                                                           .array_size = 1,
                                                                           .name = "rainbow_texture"});

    d3d12::SRVDescriptorHeap srv_uav_descriptor_heap = d3d12::SRVDescriptorHeap::create(
        renderer_context.device(), "srv_uav_descriptor_heap", 2, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
    srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srv_desc.Texture2D.MipLevels = 1;
    srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    renderer_context.device()->CreateShaderResourceView(rainbow_texture.get(), &srv_desc,
                                                        srv_uav_descriptor_heap.getCPUHandle(0));

    auto footprint = d3d12::get_copyable_footprints(renderer_context.device(), rainbow_texture->GetDesc(), 0, 1, 0);

    d3d12::UploadBuffer texture_upload_buffer =
        resource_factory.create_upload_buffer({.total_bytes = footprint.total_bytes, .name = "rainbow_upload_buffer"});

    texture_upload_buffer->Map(0, &read_range, &p_void);
    std::uint8_t *dst = static_cast<std::uint8_t *>(p_void) + footprint.layouts[0].Offset;
    for (std::uint32_t y = 0; y < texture_height; y++)
    {
        memcpy(dst, &texture_data[y * texture_width], texture_width * 4);
        dst += footprint.layouts[0].Footprint.RowPitch;
    }
    texture_upload_buffer->Unmap(0, nullptr);

    D3D12_TEXTURE_COPY_LOCATION src_location{};
    src_location.pResource = texture_upload_buffer.get();
    src_location.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    src_location.PlacedFootprint = footprint.layouts[0];

    D3D12_TEXTURE_COPY_LOCATION dst_location{};
    dst_location.pResource = rainbow_texture.get();
    dst_location.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dst_location.SubresourceIndex = 0;

    D3D12_RESOURCE_BARRIER texture_resource_barrier{};
    texture_resource_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    texture_resource_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    texture_resource_barrier.Transition.pResource = rainbow_texture.get();
    texture_resource_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
    texture_resource_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
    texture_resource_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    command_list->ResourceBarrier(1, &texture_resource_barrier);
    command_list->CopyTextureRegion(&dst_location, 0, 0, 0, &src_location, nullptr);
    texture_resource_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    texture_resource_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    texture_resource_barrier.Transition.pResource = rainbow_texture.get();
    texture_resource_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    texture_resource_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    texture_resource_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    command_list->ResourceBarrier(1, &texture_resource_barrier);

    command_list->Close();
    ID3D12CommandList *lists[] = {command_list.get()};
    renderer_context.queue()->ExecuteCommandLists(1, lists);

#pragma endregion

#pragma region dsv

    d3d12::DepthStencil depth_stencil = resource_factory.create_depth_stencil({.width = window.get_width(),
                                                                               .height = window.get_height(),
                                                                               .format = DXGI_FORMAT_D32_FLOAT,
                                                                               .clear_depth = 1.0f,
                                                                               .name = "depth_stencil_buffer"});

    d3d12::DSVDescriptorHeap dsv_descriptor_heap =
        d3d12::DSVDescriptorHeap::create(renderer_context.device(), "dsv_descriptor_heap", 1);

    D3D12_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc{};
    depth_stencil_view_desc.Format = DXGI_FORMAT_D32_FLOAT;
    depth_stencil_view_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

    renderer_context.device()->CreateDepthStencilView(depth_stencil.get(), &depth_stencil_view_desc,
                                                      dsv_descriptor_heap.getCPUHandle(0));
#pragma endregion

    MSG msg;
    bool is_running = true;
    d3d12::Fence sync_fence = d3d12::Fence::create(renderer_context.device(), "sync_fence");
    UINT64 frame_fence_values[2] = {0, 0};

    renderer_context.queue()->Signal(sync_fence.get(), sync_fence.add_fetch());
    sync_fence->SetEventOnCompletion(sync_fence.get_value(), sync_fence.get_event());
    WaitForSingleObject(sync_fence.get_event(), INFINITE);

    static auto start_time = std::chrono::high_resolution_clock::now();

    while (is_running)
    {
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                is_running = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        if (!is_running)
        {
            break;
        }
        UINT frame_index = swap_chain->GetCurrentBackBufferIndex();
        if (sync_fence->GetCompletedValue() < frame_fence_values[frame_index])
        {
            sync_fence->SetEventOnCompletion(frame_fence_values[frame_index], sync_fence.get_event());
            WaitForSingleObject(sync_fence.get_event(), INFINITE);
        }

        auto frame_time = std::chrono::high_resolution_clock::now();

        command_allocators[frame_index]->Reset();
        command_list->Reset(command_allocators[frame_index].get(), nullptr);

        D3D12_RESOURCE_BARRIER barrier_to_rtv{};
        barrier_to_rtv.Transition.pResource = swap_chain.getBuffer(frame_index);
        barrier_to_rtv.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier_to_rtv.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier_to_rtv.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier_to_rtv.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

        command_list->ResourceBarrier(1, &barrier_to_rtv);

        D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = swap_chain.getRTVDescriptorHandle(frame_index);
        D3D12_CPU_DESCRIPTOR_HANDLE dsv_handle = dsv_descriptor_heap.getCPUHandle(0);

        command_list->OMSetRenderTargets(1, &rtv_handle, FALSE, &dsv_handle);

        float clear_color[4] = {1.0f, 0.5f, 0.5f, 1.0f};
        command_list->ClearRenderTargetView(rtv_handle, clear_color, 0, nullptr);
        command_list->ClearDepthStencilView(dsv_descriptor_heap.getCPUHandle(0), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0,
                                            nullptr);

        D3D12_VIEWPORT viewport = {};
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = static_cast<float>(window.get_width());
        viewport.Height = static_cast<float>(window.get_height());
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        command_list->RSSetViewports(1, &viewport);

        D3D12_RECT scissor_rect = {};
        scissor_rect.left = 0;
        scissor_rect.top = 0;
        scissor_rect.right = window.get_width();
        scissor_rect.bottom = window.get_height();
        command_list->RSSetScissorRects(1, &scissor_rect);

        float angle = std::chrono::duration<float>(frame_time - start_time).count();
        XMMATRIX rotation_y = XMMatrixRotationY(angle);
        XMMATRIX rotation_x = XMMatrixRotationX(angle);
        XMMATRIX world = rotation_x * rotation_y;

        XMVECTOR eye = XMVectorSet(3.0f, 0.0f, 5.0f, 1.0f);
        XMVECTOR target = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
        XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        XMMATRIX view = XMMatrixLookAtLH(eye, target, up);

        float aspect = static_cast<float>(window.get_width()) / static_cast<float>(window.get_height());
        XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(60), aspect, 0.1f, 100.0f);
        XMFLOAT4X4 wvp;
        XMStoreFloat4x4(&wvp, XMMatrixTranspose(world * view * proj));


        ID3D12DescriptorHeap *pp_descriptors_heap[] = {srv_uav_descriptor_heap.get()};
        command_list->SetPipelineState(pso.get());
        command_list->SetGraphicsRootSignature(root_signature.get());
        command_list->SetDescriptorHeaps(1, pp_descriptors_heap);
        command_list->SetGraphicsRoot32BitConstants(0, 16, &wvp, 0);
        command_list->SetGraphicsRoot32BitConstants(2, 1, &angle, 0);
        command_list->SetGraphicsRootDescriptorTable(1, srv_uav_descriptor_heap.getGPUHandle(0));

        command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        command_list->IASetVertexBuffers(0, 1, &vb_view);
        command_list->IASetIndexBuffer(&ib_view);
        command_list->DrawIndexedInstanced(indexes_count, 1, 0, 0, 0);

        D3D12_RESOURCE_BARRIER barrier_to_present{};
        barrier_to_present.Transition.pResource = swap_chain.getBuffer(frame_index);
        barrier_to_present.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier_to_present.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        barrier_to_present.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier_to_present.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

        command_list->ResourceBarrier(1, &barrier_to_present);

        command_list->Close();
        ID3D12CommandList *command_lists[] = {command_list.get()};
        renderer_context.queue()->ExecuteCommandLists(1, command_lists);

        swap_chain->Present(1, 0);

        uint64_t temp_val = sync_fence.add_fetch();
        renderer_context.queue()->Signal(sync_fence.get(), temp_val);
        frame_fence_values[frame_index] = temp_val;
    }
    renderer_context.queue()->Signal(sync_fence.get(), sync_fence.add_fetch());
    sync_fence->SetEventOnCompletion(sync_fence.get_value(), sync_fence.get_event());
    WaitForSingleObject(sync_fence.get_event(), INFINITE);
}
