#pragma once

#include <d3d12.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace glRemix::dx
{
	struct TextureDesc
    {
        UINT32 width = 0;
        UINT32 height = 0;
        UINT16 depth_or_array_size = 1;
        UINT16 mip_levels = 1;
        DXGI_FORMAT format;
        // UINT16 sample_count = 1; // TODO: MSAA
        D3D12_RESOURCE_DIMENSION dimension;
        D3D12_BARRIER_LAYOUT initial_layout = D3D12_BARRIER_LAYOUT_COMMON; // If UAV is used the flag is added
        bool is_render_target = false;
    };

    struct D3D12Texture : D3D12Resource
    {
        TextureDesc desc;
        friend class D3D12Context;
    };
    
}
