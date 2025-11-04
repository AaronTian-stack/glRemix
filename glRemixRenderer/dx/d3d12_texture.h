#pragma once

#include <optional>
#include <d3d12.h>

namespace glRemix::dx
{
	struct TextureCreateDesc
	{
		D3D12_BARRIER_LAYOUT init_layout = D3D12_BARRIER_LAYOUT_UNDEFINED;
		std::optional<D3D12_CLEAR_VALUE> clear_value;
	};
}
