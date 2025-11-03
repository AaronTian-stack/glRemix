#pragma once

#include <d3d12.h>
#include <vector>
#include <optional>

namespace glRemix::dx
{
	using InputLayoutDesc = std::vector<D3D12_INPUT_ELEMENT_DESC>;

	struct RenderTargetDesc
	{
		UINT num_render_targets = 1;
		std::array<DXGI_FORMAT, 8> rtv_formats;
		DXGI_FORMAT dsv_format = DXGI_FORMAT_UNKNOWN;
	};

	struct GraphicsPipelineDesc
	{
		ID3D12RootSignature* root_signature = nullptr;
		InputLayoutDesc input_layout{};
		D3D12_PRIMITIVE_TOPOLOGY_TYPE topology_type = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		RenderTargetDesc render_targets{};
		
		std::optional<D3D12_RASTERIZER_DESC> rasterizer_state;
		std::optional<D3D12_BLEND_DESC> blend_state;
		std::optional<D3D12_DEPTH_STENCIL_DESC> depth_stencil_state;
		
		UINT sample_count = 1;
		UINT sample_quality = 0;
		bool increment_input_slot = false; // For shader reflection: if true, increment slot for each input element
	};
}
