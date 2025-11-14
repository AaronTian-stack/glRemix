#pragma once

#include <cstdint>
#include <wrl/client.h>
#include <D3D12MemAlloc.h>
#include "d3d12_barrier.h"

using Microsoft::WRL::ComPtr;

namespace glRemix::dx
{
struct D3D12Resource
{
protected:
    Resource barrier_state{};
    ComPtr<D3D12MA::Allocation> allocation{};

public:
    D3D12_GPU_VIRTUAL_ADDRESS get_gpu_address() const
    {
        assert(allocation);
        return allocation.Get()->GetResource()->GetGPUVirtualAddress();
    }
};

// Bitfield
enum BufferVisibility : uint8_t
{
    GPU = 1 << 0,
    CPU = 1 << 1,
};

struct BufferDesc
{
    UINT64 size = 0;
    UINT64 stride = 0;  // This is metadata that can used in binding vertex buffers
    BufferVisibility visibility = GPU;
    // Normally you should have some usage enum that could enumerate this stuff and more
    bool uav = false;  // Still need to transition after creating resource
    bool acceleration_structure = false;
};

struct D3D12Buffer : D3D12Resource
{
    BufferDesc desc;
    friend class D3D12Context;  // It would be better if the internals were just exposed as a void
                                // pointer or something
};

}  // namespace glRemix::dx
