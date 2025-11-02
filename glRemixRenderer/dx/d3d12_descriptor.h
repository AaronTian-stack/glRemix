#pragma once

namespace glremix::dx
{
	struct D3D12DescriptorHeap;

	struct D3D12DescriptorTable
	{
		D3D12MA::VirtualAllocation alloc{};
		UINT64 offset = 0; // Offset from start of heap in number of descriptors
		size_t count = 0; // Number of descriptors in this table
		D3D12DescriptorHeap* heap = nullptr;
	};
}
