#include "d3d12_pipeline_builder.h"

#include <cassert>

#include "d3d12_context.h"

using namespace glremix::dx;

D3D12_BLEND_DESC D3D12PipelineBuilder::make_default_blend_desc()
{
	D3D12_BLEND_DESC desc
	{
		.AlphaToCoverageEnable = FALSE,
		.IndependentBlendEnable = FALSE,
	};

	constexpr D3D12_RENDER_TARGET_BLEND_DESC default_render_target_blend_desc
	{
		.BlendEnable = FALSE,
		.LogicOpEnable = FALSE,
		.SrcBlend = D3D12_BLEND_ONE,
		.DestBlend = D3D12_BLEND_ZERO,
		.BlendOp = D3D12_BLEND_OP_ADD,
		.SrcBlendAlpha = D3D12_BLEND_ONE,
		.DestBlendAlpha = D3D12_BLEND_ZERO,
		.BlendOpAlpha = D3D12_BLEND_OP_ADD,
		.LogicOp = D3D12_LOGIC_OP_NOOP,
		.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL,
	};

	for (auto& i : desc.RenderTarget)
	{
		i = default_render_target_blend_desc;
	}

	return desc;
}

D3D12_DEPTH_STENCIL_DESC D3D12PipelineBuilder::make_default_depth_stencil_desc()
{
	D3D12_DEPTH_STENCIL_DESC desc
	{
		.DepthEnable = FALSE,
		.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL,
		.DepthFunc = D3D12_COMPARISON_FUNC_LESS,
		.StencilEnable = FALSE,
		.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK,
		.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK,
		.FrontFace = {},
		.BackFace = {},
	};

	constexpr D3D12_DEPTH_STENCILOP_DESC default_stencil_op
	{
		.StencilFailOp = D3D12_STENCIL_OP_KEEP,
		.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP,
		.StencilPassOp = D3D12_STENCIL_OP_KEEP,
		.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS
	};

	desc.FrontFace = default_stencil_op;
	desc.BackFace = default_stencil_op;

	return desc;
}

bool D3D12PipelineBuilder::create_graphics_pipeline(
	ID3D12Device* device,
	const GraphicsPipelineDesc& desc,
	const ShaderDesc& vertex_shader,
	const ShaderDesc& pixel_shader,
	ID3D12PipelineState** pipeline_state
)
{
	if (!device || !pipeline_state)
	{
		OutputDebugStringA("D3D12 ERROR: Invalid device or pipeline state pointer\n");
		return false;
	}

	if (!desc.root_signature)
	{
		OutputDebugStringA("D3D12 ERROR: Root signature is required\n");
		return false;
	}

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc{};

	pso_desc.pRootSignature = desc.root_signature;

	pso_desc.VS = 
	{
		.pShaderBytecode = vertex_shader.bytecode,
		.BytecodeLength = vertex_shader.size
	};
	pso_desc.PS = 
	{
		.pShaderBytecode = pixel_shader.bytecode,
		.BytecodeLength = pixel_shader.size
	};

	pso_desc.InputLayout = 
	{
		.pInputElementDescs = desc.input_layout.elements,
		.NumElements = desc.input_layout.count
	};

	if (desc.rasterizer_state.has_value())
	{
		pso_desc.RasterizerState = desc.rasterizer_state.value();
	}
	else
	{
		pso_desc.RasterizerState =
		{
			.FillMode = D3D12_FILL_MODE_SOLID,
			.CullMode = D3D12_CULL_MODE_BACK,
			.FrontCounterClockwise = FALSE,
			.DepthBias = D3D12_DEFAULT_DEPTH_BIAS,
			.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
			.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
			.DepthClipEnable = TRUE,
			.MultisampleEnable = FALSE,
			.AntialiasedLineEnable = FALSE,
			.ForcedSampleCount = 0,
			.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
		};
	}

	if (desc.blend_state.has_value())
	{
		pso_desc.BlendState = desc.blend_state.value();
	}
	else
	{
		pso_desc.BlendState = make_default_blend_desc();
	}

	if (desc.depth_stencil_state.has_value())
	{
		pso_desc.DepthStencilState = desc.depth_stencil_state.value();
	}
	else
	{
		pso_desc.DepthStencilState = make_default_depth_stencil_desc();
	}

	pso_desc.SampleMask = UINT_MAX;

	pso_desc.PrimitiveTopologyType = desc.topology_type;

	pso_desc.NumRenderTargets = desc.render_targets.num_render_targets;
	for (UINT i = 0; i < desc.render_targets.num_render_targets && i < 8; ++i)
	{
		pso_desc.RTVFormats[i] = desc.render_targets.rtv_formats[i];
	}

	pso_desc.DSVFormat = desc.render_targets.dsv_format;

	pso_desc.SampleDesc =
	{
		.Count = desc.sample_count,
		.Quality = desc.sample_quality
	};

	if (FAILED(device->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(pipeline_state))))
	{
		OutputDebugStringA("D3D12 ERROR: Failed to create Graphics Pipeline State\n");
		return false;
	}

	set_debug_name(*pipeline_state, desc.debug_name);

	return true;
}

