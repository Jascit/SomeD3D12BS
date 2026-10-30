#include <DirectXMath.h>
#include <Windows.h>
#include <d3d12.h>
#include <dxcapi.h>
#include <dxgi1_6.h>
#include <filesystem>
#include <memory>
#include <wrl/client.h>
import core.log;
import core.window;
import utils;
import core.last_error;
import d3d12.commited_resource;
import d3d12.root_signature;
import d3d12.factory;
import d3d12.device;
import d3d12.commands;
import d3d12.swapchain;
import d3d12.descriptor_heap;
import d3d12.pipeline_state_object;
import d3d12.shaders;
import d3d12.input_element_layout;
import d3d12.obj_parser;
using Microsoft::WRL::ComPtr;
using namespace DirectX;

int main()
{
#pragma region initialization
    constexpr std::uint32_t BUFFER_COUNT = 2;
    core::diagnostics::Logger::instance().registerSink(std::make_unique<core::diagnostics::ConsoleSink>());
    d3d12::utils::enable_d3d12_debug_layer();

    ComPtr<IDxcUtils> dxc_utils;
    HRESULT result = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxc_utils));
    if (FAILED(result))
    {
        core::diagnostics::log_error("Failed to create dxc utils! HRESULT: 0x{:08X}",
                                     static_cast<unsigned long>(result));
        return 1;
    }

    ComPtr<IDxcCompiler3> dxc_compiler;
    result = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxc_compiler));
    if (FAILED(result))
    {
        core::diagnostics::log_error("Failed to create dxc compiler! HRESULT: 0x{:08X}",
                                     static_cast<unsigned long>(result));
        return 1;
    }

    auto window = core::Window::create(L"Yay window!!", L"some bs class name for my klein Fenster");
    if (!window)
    {
        uint64_t error = core::diagnostics::get_last_error();
        return 1;
    }

    window.show();

    d3d12::Factory factory = d3d12::Factory::create();
    if (!factory)
    {
        return 1;
    }

    d3d12::Device device = d3d12::Device::create(factory);
    if (!device)
    {
        return 1;
    }
#pragma endregion

#pragma region root_signature
    D3D12_DESCRIPTOR_RANGE1 srv_range{};
    srv_range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    srv_range.NumDescriptors = 1;
    srv_range.RegisterSpace = 0;
    srv_range.BaseShaderRegister = 0;
    srv_range.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
    srv_range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    d3d12::RootParameter parameters[] = {
        d3d12::RootParameter::constants(16, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX),
        d3d12::RootParameter::descriptorTable({&srv_range, 1}, D3D12_SHADER_VISIBILITY_PIXEL),
        d3d12::RootParameter::constants(1, 1, 0, D3D12_SHADER_VISIBILITY_PIXEL)};

    D3D12_STATIC_SAMPLER_DESC1 sampler_desc1{};
    sampler_desc1.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sampler_desc1.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler_desc1.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler_desc1.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler_desc1.MaxAnisotropy = 1;
    sampler_desc1.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    sampler_desc1.MinLOD = 0.0f;
    sampler_desc1.MaxLOD = D3D12_FLOAT32_MAX;
    sampler_desc1.ShaderRegister = 0;
    sampler_desc1.RegisterSpace = 0;
    sampler_desc1.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    d3d12::RootSignature root_signature =
        d3d12::RootSignature::create(device, {parameters}, {&sampler_desc1, 1},
                                     D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
                                         D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
                                         D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
                                         D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS);
    if (!root_signature)
    {
        return 1;
    }

    D3D12_DESCRIPTOR_RANGE1 uav_range{};
    srv_range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
    srv_range.NumDescriptors = 1;
    srv_range.RegisterSpace = 0;
    srv_range.BaseShaderRegister = 0;
    srv_range.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
    srv_range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    d3d12::RootParameter compute_parameters[] = {d3d12::RootParameter::UAV(0)};
    d3d12::RootSignature compute_root_signature =
        d3d12::RootSignature::create(device, {compute_parameters, 1}, {}, D3D12_ROOT_SIGNATURE_FLAG_NONE);
    if (!compute_root_signature)
    {
        return 1;
    }
#pragma endregion

#pragma region shaders
    wchar_t exe_path[MAX_PATH];
    GetModuleFileNameW(nullptr, exe_path, MAX_PATH);
    std::filesystem::path shader_directory(exe_path);
    shader_directory = shader_directory.parent_path().parent_path().parent_path() / L"shaders";
    const std::filesystem::path ps_path = shader_directory / L"ps.hlsl";
    const std::filesystem::path cs_path = shader_directory / L"cs.hlsl";
    const std::filesystem::path vs_path = shader_directory / L"vs.hlsl";

    d3d12::VertexShader vs_shader = d3d12::VertexShader::create(vs_path, dxc_compiler.Get());
    d3d12::PixelShader ps_shader = d3d12::PixelShader::create(ps_path, dxc_compiler.Get());
    d3d12::ComputeShader cs_shader = d3d12::ComputeShader::create(cs_path, dxc_compiler.Get());
#pragma endregion

#pragma region commands_related
    d3d12::CommandAllocator command_allocators[BUFFER_COUNT];
    for (auto &command_allocator : command_allocators)
    {
        command_allocator = d3d12::CommandAllocator::create(device);
        if (!command_allocator)
        {
            return 1;
        }
    }

    d3d12::CommandList command_list = d3d12::CommandList::create(device, command_allocators[0]);
    if (!command_list)
    {
        return 1;
    }

    d3d12::CommandQueue command_queue = d3d12::CommandQueue::create(device);
    if (!command_queue)
    {
        return 1;
    }
#pragma endregion

#pragma region swapchain
    d3d12::RTVDescriptorHeap rtv_descriptor_heap = d3d12::RTVDescriptorHeap::create(device, BUFFER_COUNT);
    if (!rtv_descriptor_heap)
    {
        return 1;
    }
    const d3d12::SwapChainConfig swap_chain_config =
        d3d12::SwapChainConfig::makeDefault(window.getWidth(), window.getHeight());
    auto swap_chain = d3d12::SwapChain::create(std::move(rtv_descriptor_heap), device, factory, command_queue,
                                               swap_chain_config, window.getSystemHandle());
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
    d3d12::PipelineStateObject pso = d3d12::PipelineStateObject::create(device, pso_config);
    if (!pso)
    {
        return 1;
    }

    D3D12_COMPUTE_PIPELINE_STATE_DESC compute_pso_desc{};
    compute_pso_desc.CS.BytecodeLength = cs_shader->GetBufferSize();
    compute_pso_desc.CS.pShaderBytecode = cs_shader->GetBufferPointer();
    compute_pso_desc.pRootSignature = compute_root_signature.get();
    ComPtr<ID3D12PipelineState> compute_pso;
    result = device->CreateComputePipelineState(&compute_pso_desc, IID_PPV_ARGS(&compute_pso));
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
    auto [vertex_data, index_data] = d3d12::ObjParser::parse(cube_path);
    UINT vertex_buffer_size = vertex_data.size() * sizeof(d3d12::Vertex);
    UINT indexes_count = index_data.size();
    UINT index_buffer_size = indexes_count * sizeof(d3d12::vertex_index);

    d3d12::CommitedResourceConfig vertex_data_config = d3d12::CommitedResourceConfig::buffer(
        D3D12_HEAP_TYPE_UPLOAD, vertex_buffer_size, D3D12_RESOURCE_STATE_GENERIC_READ);
    d3d12::CommitedResource vertex_buffer = d3d12::CommitedResource::create(device, vertex_data_config);

    D3D12_RANGE read_range = {0, 0};
    void *p_void = nullptr;
    vertex_buffer->Map(0, &read_range, &p_void);
    memcpy(p_void, vertex_data.data(), vertex_buffer_size);
    vertex_buffer->Unmap(0, nullptr);

    D3D12_VERTEX_BUFFER_VIEW vb_view;
    vb_view.BufferLocation = vertex_buffer->GetGPUVirtualAddress();
    vb_view.SizeInBytes = vertex_buffer_size;
    vb_view.StrideInBytes = sizeof(d3d12::Vertex);

    d3d12::CommitedResourceConfig index_data_config = d3d12::CommitedResourceConfig::buffer(
        D3D12_HEAP_TYPE_UPLOAD, index_buffer_size, D3D12_RESOURCE_STATE_GENERIC_READ);
    d3d12::CommitedResource index_buffer = d3d12::CommitedResource::create(device, index_data_config);

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

    d3d12::CommitedResourceConfig texture_config = d3d12::CommitedResourceConfig::texture2D(
        texture_height, texture_width, D3D12_RESOURCE_STATE_COMMON, DXGI_FORMAT_R8G8B8A8_UNORM);
    d3d12::CommitedResource rainbow_texture_buffer = d3d12::CommitedResource::create(device, texture_config);

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
    std::uint64_t upload_size = 0;
    D3D12_RESOURCE_DESC rainbow_texture_desc = rainbow_texture_buffer->GetDesc();
    device->GetCopyableFootprints(&rainbow_texture_desc, 0, 1, 0, &footprint, nullptr, nullptr, &upload_size);

    d3d12::SRVDescriptorHeap srv_uav_descriptor_heap =
        d3d12::SRVDescriptorHeap::create(device, 2, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
    srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srv_desc.Texture2D.MipLevels = 1;
    srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    device->CreateShaderResourceView(rainbow_texture_buffer.get(), &srv_desc, srv_uav_descriptor_heap.getCPUHandle(0));

    //D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc{};
    //uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    //uav_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    //uav_desc.Texture2D.MipSlice = 0;
    //uav_desc.Texture2D.PlaneSlice = 0;
    //device->CreateUnorderedAccessView(rainbow_texture_buffer.get(), nullptr, &uav_desc,
    //                                  srv_uav_descriptor_heap.getCPUHandle(1));

    d3d12::CommitedResourceConfig upload_buffer_config =
        d3d12::CommitedResourceConfig::buffer(D3D12_HEAP_TYPE_UPLOAD, upload_size, D3D12_RESOURCE_STATE_GENERIC_READ);
    d3d12::CommitedResource rainbow_texture_upload = d3d12::CommitedResource::create(device, upload_buffer_config);

    rainbow_texture_upload->Map(0, &read_range, &p_void);
    std::uint8_t *dst = reinterpret_cast<std::uint8_t *>(p_void) + footprint.Offset;
    for (std::uint32_t y = 0; y < texture_height; y++)
    {
        memcpy(dst, &texture_data[y * texture_width], texture_width * 4);
        dst += footprint.Footprint.RowPitch;
    }
    rainbow_texture_upload->Unmap(0, nullptr);

    D3D12_TEXTURE_COPY_LOCATION src_location{};
    src_location.pResource = rainbow_texture_upload.get();
    src_location.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    src_location.PlacedFootprint = footprint;

    D3D12_TEXTURE_COPY_LOCATION dst_location{};
    dst_location.pResource = rainbow_texture_buffer.get();
    dst_location.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dst_location.SubresourceIndex = 0;

    D3D12_RESOURCE_BARRIER texture_resource_barrier{};
    texture_resource_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    texture_resource_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    texture_resource_barrier.Transition.pResource = rainbow_texture_buffer.get();
    texture_resource_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
    texture_resource_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
    texture_resource_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    command_list->ResourceBarrier(1, &texture_resource_barrier);
    command_list->CopyTextureRegion(&dst_location, 0, 0, 0, &src_location, nullptr);
    texture_resource_barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    texture_resource_barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    texture_resource_barrier.Transition.pResource = rainbow_texture_buffer.get();
    texture_resource_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    texture_resource_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    texture_resource_barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    command_list->ResourceBarrier(1, &texture_resource_barrier);

    command_list->Close();
    ID3D12CommandList *lists[] = {command_list.get()};
    command_queue->ExecuteCommandLists(1, lists);

#pragma endregion

#pragma region uav

#pragma endregion

#pragma region dsv
    d3d12::CommitedResourceConfig dsb_config = d3d12::CommitedResourceConfig::depthStencil(
        window.getHeight(), window.getWidth(), DXGI_FORMAT_D32_FLOAT, D3D12_RESOURCE_STATE_DEPTH_WRITE,
        D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
    d3d12::CommitedResource depth_stencil_buffer = d3d12::CommitedResource::create(device, dsb_config);

    d3d12::DSVDescriptorHeap dsv_descriptor_heap = d3d12::DSVDescriptorHeap::create(device, 1);

    D3D12_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc{};
    depth_stencil_view_desc.Format = DXGI_FORMAT_D32_FLOAT;
    depth_stencil_view_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

    device->CreateDepthStencilView(depth_stencil_buffer.get(), &depth_stencil_view_desc,
                                   dsv_descriptor_heap.getCPUHandle(0));
#pragma endregion

#pragma region camera

#pragma endregion

    MSG msg;
    bool is_running = true;

    ComPtr<ID3D12Fence1> sync_fence;
    UINT64 fence_value = 0;
    device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&sync_fence));
    HANDLE fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);

    UINT64 frame_fence_values[2] = {0, 0};

    command_queue->Signal(sync_fence.Get(), ++fence_value);
    sync_fence->SetEventOnCompletion(fence_value, fence_event);
    WaitForSingleObject(fence_event, INFINITE);

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
        auto frame_time = std::chrono::high_resolution_clock::now();
        UINT frame_index = swap_chain->GetCurrentBackBufferIndex();

        if (sync_fence->GetCompletedValue() < frame_fence_values[frame_index])
        {
            sync_fence->SetEventOnCompletion(frame_fence_values[frame_index], fence_event);
            WaitForSingleObject(fence_event, INFINITE);
        }

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
        viewport.Width = static_cast<float>(window.getWidth());
        viewport.Height = static_cast<float>(window.getHeight());
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        command_list->RSSetViewports(1, &viewport);

        D3D12_RECT scissor_rect = {};
        scissor_rect.left = 0;
        scissor_rect.top = 0;
        scissor_rect.right = window.getWidth();
        scissor_rect.bottom = window.getHeight();
        command_list->RSSetScissorRects(1, &scissor_rect);

        float angle = std::chrono::duration<float>(frame_time - start_time).count();
        XMMATRIX rotation_y = XMMatrixRotationY(angle);
        XMMATRIX rotation_x = XMMatrixRotationX(angle);
        XMMATRIX world = rotation_x * rotation_y;

        XMVECTOR eye = XMVectorSet(3.0f, 0.0f, 5.0f, 1.0f);
        XMVECTOR target = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
        XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
        XMMATRIX view = XMMatrixLookAtLH(eye, target, up);

        float aspect = static_cast<float>(window.getWidth()) / static_cast<float>(window.getHeight());
        XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(60), aspect, 0.1f, 100.0f);
        XMFLOAT4X4 wvp;
        XMStoreFloat4x4(&wvp, XMMatrixTranspose(world * view * proj));

        ID3D12DescriptorHeap *pp_descriptors_heap[] = {srv_uav_descriptor_heap.get()};
        command_list->SetDescriptorHeaps(1, pp_descriptors_heap);

        // command_list->SetPipelineState(pso.get());
        // command_list->SetComputeRootDescriptorTable(0, srv_uav_descriptor_heap.getGPUHandle(1));
        // command_list->SetComputeRootSignature(compute_root_signature.get());
        //
        // command_list->Dispatch((texture_width + 7) / 8, (texture_height + 7) / 8, 1);

        command_list->SetPipelineState(pso.get());
        command_list->SetGraphicsRootSignature(root_signature.get());
        command_list->SetGraphicsRoot32BitConstants(0, 16, &wvp, 0);
        command_list->SetGraphicsRootDescriptorTable(1, srv_uav_descriptor_heap.getGPUHandle(0));
        command_list->SetGraphicsRoot32BitConstants(2, 1, &angle, 0);

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
        command_queue->ExecuteCommandLists(1, command_lists);

        swap_chain->Present(1, 0);

        fence_value++;
        command_queue->Signal(sync_fence.Get(), fence_value);
        frame_fence_values[frame_index] = fence_value;
    }
    command_queue->Signal(sync_fence.Get(), ++fence_value);
    sync_fence->SetEventOnCompletion(fence_value, fence_event);
    WaitForSingleObject(fence_event, INFINITE);
    CloseHandle(fence_event);
}
