#pragma once

#include <wrl/client.h>
#include <D3D12MemAlloc.h>
#include "d3d12_barrier.h"

using Microsoft::WRL::ComPtr;

// Operator overloads for D3D12MA enums to work with designated initializers
namespace D3D12MA
{
constexpr ALLOCATOR_FLAGS operator|(ALLOCATOR_FLAGS a, ALLOCATOR_FLAGS b)
{
    return static_cast<ALLOCATOR_FLAGS>(static_cast<int>(a) | static_cast<int>(b));
}
}  // namespace D3D12MA

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
enum BufferVisibility : UINT8
{
    GPU = 1 << 0,
    CPU = 1 << 1,
};

constexpr BufferVisibility operator|(const BufferVisibility a, const BufferVisibility b)
{
    return static_cast<BufferVisibility>(static_cast<UINT8>(a) | static_cast<UINT8>(b));
}

constexpr BufferVisibility operator&(const BufferVisibility a, const BufferVisibility b)
{
    return static_cast<BufferVisibility>(static_cast<UINT8>(a) & static_cast<UINT8>(b));
}

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
