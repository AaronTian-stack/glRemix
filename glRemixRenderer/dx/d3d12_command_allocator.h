#pragma once

#include "d3d12_queue.h"

namespace glremix::dx
{
    struct D3D12CommandAllocator
    {
        D3D12Queue* queue = nullptr;
        ComPtr<ID3D12CommandAllocator> cmd_allocator;
    };
}
