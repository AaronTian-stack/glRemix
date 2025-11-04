#pragma once

#include <cstdint>
#include <wrl/client.h>
#include <D3D12MemAlloc.h>

using Microsoft::WRL::ComPtr;

namespace glRemix::dx
{
    // Bitfield
    enum BufferVisibility : uint8_t
    {
		GPU = 1 << 0,
        CPU = 1 << 1,
    };

    struct BufferDesc
    {
        UINT64 size = 0;
        UINT64 stride = 0; // This is metadata that can used in binding vertex buffers 
        BufferVisibility visibility = GPU;
        // Normally you should have some usage enum that could enumerate this stuff and more
        bool uav = false; // Still need to transition after creating resource
        bool acceleration_structure = false;
    };

    struct D3D12Buffer
    {
        BufferDesc desc;
        ComPtr<D3D12MA::Allocation> allocation;
    };

}
