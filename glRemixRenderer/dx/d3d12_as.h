#pragma once

#include <wrl/client.h>

#include "d3d12_buffer.h"

using Microsoft::WRL::ComPtr;

namespace glRemix::dx
{
	// BLAS is just a buffer
	
	struct D3D12TLAS
	{
		D3D12Buffer tlas;
		D3D12Buffer instance; // This should use a GPU upload heap for dynamic updates
	};

	// TODO: SBT
}