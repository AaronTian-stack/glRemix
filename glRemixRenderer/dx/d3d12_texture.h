#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include <D3D12MemAlloc.h>
#include "d3d12_barrier.h"

using Microsoft::WRL::ComPtr;

namespace glRemix::dx
{
	struct TextureDesc
    {
        uint64_t width = 0;
        uint32_t height = 0;
        uint16_t depth_or_array_size = 1;
        uint16_t mip_levels = 1;
        DXGI_FORMAT format;
        // uint16_t sample_count = 1; // TODO: MSAA
        D3D12_RESOURCE_DIMENSION dimension;
        D3D12_BARRIER_LAYOUT initial_layout = D3D12_BARRIER_LAYOUT_COMMON; // If UAV is used the flag is added
        bool is_render_target = false;
    };

    struct D3D12Texture
    {
        TextureDesc desc;

        // TODO: Delete these
        D3D12_GPU_VIRTUAL_ADDRESS get_gpu_address() const
        {
            return allocation.Get()->GetResource()->GetGPUVirtualAddress();
        }

        ID3D12Resource* get_resource() const
        {
            return allocation.Get()->GetResource();
        }

    private:
        Resource barrier_state;
        ComPtr<D3D12MA::Allocation> allocation;
        friend class D3D12Context;
    };
    
}
