#pragma once

namespace glRemix::dx
{
class D3D12DescriptorHeap;

struct D3D12DescriptorTable
{
    UINT64 offset = 0;  // Offset from start of heap in number of descriptors
    size_t count = 0;   // Number of descriptors in this table
    D3D12DescriptorHeap* heap = nullptr;
private:
    D3D12MA::VirtualAllocation alloc{};
    friend class D3D12DescriptorHeap;
};
}  // namespace glRemix::dx
