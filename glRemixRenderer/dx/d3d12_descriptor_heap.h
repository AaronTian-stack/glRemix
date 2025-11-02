#pragma once

#include <d3d12.h>
#include <D3D12MemAlloc.h>
#include <wrl/client.h>

#include "d3d12_descriptor.h"

using Microsoft::WRL::ComPtr;

namespace glremix::dx
{
	// Not thread safe
	// TODO: For RTV/DSV maybe make specific version just for those because they are never copied
	class D3D12DescriptorHeap
	{
		D3D12_DESCRIPTOR_HEAP_DESC m_desc{};
		ComPtr<D3D12MA::VirtualBlock> m_virtual_block;
		ComPtr<ID3D12DescriptorHeap> m_heap;
		UINT m_descriptor_size = 0;
		
	public:
		const D3D12_DESCRIPTOR_HEAP_DESC& desc = m_desc;

		bool create(ID3D12Device* device, const D3D12_DESCRIPTOR_HEAP_DESC& desc);

		// Contiguous allocation
		bool allocate(UINT count, D3D12DescriptorTable* descriptor);
		void deallocate(D3D12DescriptorTable* descriptor) const;
		void free_all() const;

		void get_cpu_descriptor(D3D12_CPU_DESCRIPTOR_HANDLE* handle, size_t num_descriptor_offset) const;
		bool get_gpu_descriptor(D3D12_GPU_DESCRIPTOR_HANDLE* handle, size_t num_descriptor_offset) const;

		D3D12DescriptorHeap() = default;
		D3D12DescriptorHeap(const D3D12DescriptorHeap&) = delete;
		D3D12DescriptorHeap& operator=(const D3D12DescriptorHeap&) = delete;

		~D3D12DescriptorHeap() = default;

		friend class D3D12Context;
	};
}
