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
        UINT64 stride = 0;
        BufferVisibility visibility = GPU;
        bool uav = false;
    };

    struct D3D12Buffer
    {
        BufferDesc desc;
        ComPtr<D3D12MA::Allocation> allocation;
    };

}
