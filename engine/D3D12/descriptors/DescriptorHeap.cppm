/* SPDX-License-Identifier: Apache-2.0 */

/**
 * @file    DescriptorHeap.cppm
 * @brief
 *
 * @date    25/10/2026
 *
 * @copyright Copyright (c) 2026 The Project Contributors
 */
module;

#include <cstddef>
#include <cstdint>
#include <d3d12.h>
#include <type_traits>
#include <wrl/client.h>
export module d3d12.descriptor_heap;
import core.log;
import d3d12.com_resource;
using namespace Microsoft::WRL;

export namespace d3d12
{
template <D3D12_DESCRIPTOR_HEAP_TYPE HType> class DescriptorHeap : public ComResource<ID3D12DescriptorHeap>
{
    using base = ComResource<ID3D12DescriptorHeap>;

  public:
    DescriptorHeap() noexcept = default;
#pragma region factory
    [[nodiscard]] static auto create(ID3D12Device *device, const char *name, std::uint32_t count,
                                     D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE) noexcept
        -> DescriptorHeap
    {
        D3D12_DESCRIPTOR_HEAP_DESC descriptor_heap_desc{};
        descriptor_heap_desc.Type = HType;
        descriptor_heap_desc.NumDescriptors = count;
        descriptor_heap_desc.Flags = flags;
        descriptor_heap_desc.NodeMask = 0;

        DescriptorHeap descriptor_heap;
        HRESULT result = device->CreateDescriptorHeap(&descriptor_heap_desc, IID_PPV_ARGS(descriptor_heap.put()));
        if (FAILED(result))
        {
            core::diagnostics::log_error("Failed to create descriptor heap! Name: {}, HRESULT: {}", name, result);
            return {};
        }
        descriptor_heap.increment_ = device->GetDescriptorHandleIncrementSize(HType);
        descriptor_heap.is_shader_visible_ = flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        descriptor_heap.descriptor_count_ = count;
        descriptor_heap.start_handle_cpu_ = descriptor_heap.get()->GetCPUDescriptorHandleForHeapStart();
        if (descriptor_heap.is_shader_visible_)
        {
            descriptor_heap.start_handle_gpu_ = descriptor_heap.get()->GetGPUDescriptorHandleForHeapStart();
        }
        descriptor_heap.set_name(name);
        return descriptor_heap;
    }
#pragma endregion

    [[nodiscard]] auto cpu_handle(std::uint32_t index) const noexcept -> D3D12_CPU_DESCRIPTOR_HANDLE
    {
        if (index >= descriptor_count_)
        {
            core::diagnostics::log_warning(
                "Descriptor(Name: {}) index out of bounds: index {} is invalid (total descriptors: {})",
                this->get_name(), index, descriptor_count_);
            return {};
        }
        return {start_handle_cpu_.ptr + (static_cast<SIZE_T>(increment_ * index))};
    }
    [[nodiscard]] auto gpu_handle(std::uint32_t index) const noexcept -> D3D12_GPU_DESCRIPTOR_HANDLE
    {
        if (!is_shader_visible_)
        {
            core::diagnostics::log_error("Trying to get GPU handle, while Descriptor heap is not shader visible!");
            return {};
        }
        if (index >= descriptor_count_)
        {
            core::diagnostics::log_warning(
                "Descriptor index out of bounds: index {} is invalid (total descriptors: {})", index,
                descriptor_count_);
            return {};
        }
        return {start_handle_gpu_.ptr + (static_cast<SIZE_T>(increment_ * index))};
    }

  private:
    D3D12_CPU_DESCRIPTOR_HANDLE start_handle_cpu_{};
    D3D12_GPU_DESCRIPTOR_HANDLE start_handle_gpu_{};
    std::uint32_t descriptor_count_{};
    std::uint32_t increment_{};
    bool is_shader_visible_ = false;
};

using RTVDescriptorHeap = DescriptorHeap<D3D12_DESCRIPTOR_HEAP_TYPE_RTV>;
using SRVDescriptorHeap = DescriptorHeap<D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV>;
using DSVDescriptorHeap = DescriptorHeap<D3D12_DESCRIPTOR_HEAP_TYPE_DSV>;
} // namespace d3d12
