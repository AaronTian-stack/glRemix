#include "d3d12_descriptor_heap.h"

#include <cassert>

using namespace glRemix::dx;

bool D3D12DescriptorHeap::create(ID3D12Device* device, const D3D12_DESCRIPTOR_HEAP_DESC& desc)
{
    this->m_desc = desc;

    if (FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_heap.ReleaseAndGetAddressOf()))))
    {
        OutputDebugStringA("D3D12 ERROR: Failed to create descriptor heap\n");
        return false;
    }

    m_descriptor_size = device->GetDescriptorHandleIncrementSize(desc.Type);

    // GPU heap always uses linear for ring buffer
    D3D12MA::VIRTUAL_BLOCK_DESC block_desc{
        .Flags = desc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
                     ? D3D12MA::VIRTUAL_BLOCK_FLAG_ALGORITHM_LINEAR
                     : D3D12MA::VIRTUAL_BLOCK_FLAG_NONE,
        .Size = desc.NumDescriptors,  // Descriptor is atomic unit, not bytes
        .pAllocationCallbacks = nullptr,
    };
    const auto hr = D3D12MA::CreateVirtualBlock(&block_desc,
                                                m_virtual_block.ReleaseAndGetAddressOf());

    return SUCCEEDED(hr);
}

bool D3D12DescriptorHeap::allocate(const UINT count, D3D12DescriptorTable* descriptor)
{
    assert(m_descriptor_size);
    D3D12MA::VIRTUAL_ALLOCATION_DESC alloc_desc{};
    alloc_desc.Size = count;

    const auto hr = m_virtual_block->Allocate(&alloc_desc, &descriptor->alloc, &descriptor->offset);

    descriptor->heap = this;

    return SUCCEEDED(hr);
}

void D3D12DescriptorHeap::deallocate(D3D12DescriptorTable* descriptor) const
{
    m_virtual_block->FreeAllocation(descriptor->alloc);
    *descriptor = {};
}

void D3D12DescriptorHeap::free_all() const
{
    m_virtual_block->Clear();
}

void D3D12DescriptorHeap::get_cpu_descriptor(D3D12_CPU_DESCRIPTOR_HANDLE* handle,
                                             size_t num_descriptor_offset) const
{
    D3D12_CPU_DESCRIPTOR_HANDLE start_handle = m_heap->GetCPUDescriptorHandleForHeapStart();
    handle->ptr = start_handle.ptr + num_descriptor_offset * m_descriptor_size;
}

bool D3D12DescriptorHeap::get_gpu_descriptor(D3D12_GPU_DESCRIPTOR_HANDLE* handle,
                                             size_t num_descriptor_offset) const
{
    if (!(m_desc.Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE))
    {
        OutputDebugStringA("D3D12 ERROR: Trying to get GPU descriptor handle from a "
                           "non-shader-visible descriptor heap\n");
        return false;
    }
    D3D12_GPU_DESCRIPTOR_HANDLE start_handle = m_heap->GetGPUDescriptorHandleForHeapStart();
    handle->ptr = start_handle.ptr + num_descriptor_offset * m_descriptor_size;
    return true;
}
