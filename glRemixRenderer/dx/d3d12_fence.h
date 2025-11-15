#pragma once

#include <wrl/client.h>
#include <d3d12.h>

using Microsoft::WRL::ComPtr;

namespace glRemix::dx
{
struct D3D12Fence
{
    HANDLE event;
    ComPtr<ID3D12Fence> fence;

    ~D3D12Fence()
    {
        if (event)
        {
            CloseHandle(event);
        }
    }
};

struct WaitInfo
{
    bool wait_all;  // If false will wait on any of the fences
    UINT count;
    D3D12Fence* fences;
    UINT64* values;
    UINT64 timeout = INFINITE;
};
}  // namespace glRemix::dx
