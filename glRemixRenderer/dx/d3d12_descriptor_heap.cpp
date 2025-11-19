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

    return true;
}

bool D3D12DescriptorHeap::allocate(D3D12Descriptor* descriptor)
{
    assert(m_descriptor_size);

    if (m_curr_offset >= m_desc.NumDescriptors && m_free_list.empty())
    {
        return false;
    }

    if (!m_free_list.empty())
    {
        descriptor->offset = m_free_list.back();
        m_free_list.pop_back();
    }
    else
    {
        descriptor->offset = m_curr_offset++;
    }

    descriptor->heap = this;

    return true;
}

void D3D12DescriptorHeap::deallocate(D3D12Descriptor* descriptor)
{
    m_free_list.push_back(descriptor->offset);
    *descriptor = {};
}

void D3D12DescriptorHeap::free_all()
{
    m_free_list.clear();
    m_curr_offset = 0;
}

void D3D12DescriptorHeap::get_cpu_descriptor(D3D12_CPU_DESCRIPTOR_HANDLE* handle,
                                             const size_t num_descriptor_offset) const
{
    D3D12_CPU_DESCRIPTOR_HANDLE start_handle = m_heap->GetCPUDescriptorHandleForHeapStart();
    handle->ptr = start_handle.ptr + num_descriptor_offset * m_descriptor_size;
}

bool D3D12DescriptorHeap::get_gpu_descriptor(D3D12_GPU_DESCRIPTOR_HANDLE* handle,
                                             const size_t num_descriptor_offset) const
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
