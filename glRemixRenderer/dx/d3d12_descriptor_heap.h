#pragma once

#include <d3d12.h>
#include <vector>
#include <wrl/client.h>

#include "d3d12_descriptor.h"

using Microsoft::WRL::ComPtr;

namespace glRemix::dx
{
// Not thread safe
class D3D12DescriptorHeap
{
    std::vector<UINT> m_free_list;
    D3D12_DESCRIPTOR_HEAP_DESC m_desc{};
    ComPtr<ID3D12DescriptorHeap> m_heap;
    UINT m_descriptor_size = 0;
    UINT m_curr_offset = 0;

public:
    const D3D12_DESCRIPTOR_HEAP_DESC& desc = m_desc;

    bool create(ID3D12Device* device, const D3D12_DESCRIPTOR_HEAP_DESC& desc);

    bool allocate(D3D12Descriptor* descriptor);
    // Marks descriptor offset as free
    void deallocate(D3D12Descriptor* descriptor);
    void free_all();

    void get_cpu_descriptor(D3D12_CPU_DESCRIPTOR_HANDLE* handle, size_t num_descriptor_offset) const;
    bool get_gpu_descriptor(D3D12_GPU_DESCRIPTOR_HANDLE* handle, size_t num_descriptor_offset) const;

    D3D12DescriptorHeap() = default;
    D3D12DescriptorHeap(const D3D12DescriptorHeap&) = delete;
    D3D12DescriptorHeap& operator=(const D3D12DescriptorHeap&) = delete;
    D3D12DescriptorHeap(D3D12DescriptorHeap&&) = default;
    D3D12DescriptorHeap& operator=(D3D12DescriptorHeap&&) = default;

    ~D3D12DescriptorHeap() = default;

    friend class D3D12Context;
    friend class DescriptorPager;
};
}  // namespace glRemix::dx
