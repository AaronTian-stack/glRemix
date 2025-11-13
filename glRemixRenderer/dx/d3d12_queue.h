#pragma once
#include <d3d12.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

struct D3D12Queue
{
    D3D12_COMMAND_LIST_TYPE type;
    ComPtr<ID3D12CommandQueue> queue;
};
