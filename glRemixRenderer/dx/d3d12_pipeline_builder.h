#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include <optional>

using Microsoft::WRL::ComPtr;

namespace glremix::dx
{
	struct ShaderDesc
	{
		const void* bytecode = nullptr;
		size_t size = 0;
	};

	struct InputLayoutDesc
	{
		const D3D12_INPUT_ELEMENT_DESC* elements = nullptr;
		UINT count = 0;
	};

	struct RenderTargetDesc
	{
		UINT num_render_targets = 1;
		const DXGI_FORMAT* rtv_formats = nullptr;
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
		const char* debug_name = nullptr;
	};

	class D3D12PipelineBuilder
	{
		static D3D12_BLEND_DESC make_default_blend_desc();
		static D3D12_DEPTH_STENCIL_DESC make_default_depth_stencil_desc();

	public:
		bool create_graphics_pipeline(
			ID3D12Device* device,
			const GraphicsPipelineDesc& desc,
			const ShaderDesc& vertex_shader,
			const ShaderDesc& pixel_shader,
			ID3D12PipelineState** pipeline_state
		);
	};
}
