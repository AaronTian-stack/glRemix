#pragma once

#include <cstdint>
#include <wrl/client.h>
#include <d3d12.h>

using Microsoft::WRL::ComPtr;

namespace glremix::dx
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
		bool wait_all; // If false will wait on any of the fences
		UINT count;
		D3D12Fence* fences;
		uint64_t* values;
		uint64_t timeout = INFINITE;
	};
}