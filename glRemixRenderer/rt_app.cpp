#include "rt_app.h"
#include "imgui.h"

void glRemix::glRemixRenderer::create()
{
	for (UINT i = 0; i < m_frames_in_flight; i++)
	{
		THROW_IF_FALSE(m_context.create_command_allocator(&m_cmd_pools[i], &m_gfx_queue, "frame command allocator"));
	}

	// Create root signature
	{
		D3D12_ROOT_SIGNATURE_DESC root_sig_desc;
		root_sig_desc.NumParameters = 0;
		root_sig_desc.pParameters = nullptr;
		root_sig_desc.NumStaticSamplers = 0;
		root_sig_desc.pStaticSamplers = nullptr;
		root_sig_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		THROW_IF_FALSE(m_context.create_root_signature(root_sig_desc, m_root_signature.ReleaseAndGetAddressOf(), "triangle root signature"));
	}

	// Compile dummy raster pipeline for testing
	ComPtr<IDxcBlobEncoding> vertex_shader;
	THROW_IF_FALSE(m_context.load_blob_from_file(L"./shaders/triangle_vs_6_6_VSMain.dxil", vertex_shader.ReleaseAndGetAddressOf()));
	ComPtr<IDxcBlobEncoding> pixel_shader;
	THROW_IF_FALSE(m_context.load_blob_from_file(L"./shaders/triangle_ps_6_6_PSMain.dxil", pixel_shader.ReleaseAndGetAddressOf()));

	dx::GraphicsPipelineDesc pipeline_desc
	{
		.render_targets
		{
			.num_render_targets = 1,
			.rtv_formats = { DXGI_FORMAT_R8G8B8A8_UNORM },
			.dsv_format = DXGI_FORMAT_UNKNOWN,
		},
	};
	
	pipeline_desc.root_signature = m_root_signature.Get();

	ComPtr<ID3D12ShaderReflection> shader_reflection_interface; // Needs to stay in scope until pipeline is created
	THROW_IF_FALSE(m_context.reflect_input_layout(vertex_shader.Get(), &pipeline_desc.input_layout, false, shader_reflection_interface.ReleaseAndGetAddressOf()));

	THROW_IF_FALSE(m_context.create_graphics_pipeline(pipeline_desc, 
		vertex_shader.Get(), pixel_shader.Get(), 
		m_raster_pipeline.ReleaseAndGetAddressOf(), "raster pipeline"));
	

	// Compile ray tracing pipeline
	// Build BLAS here for now, but renderer will construct them dynamically for new geometry in render loop

	// Init ImGui
	THROW_IF_FALSE(m_context.init_imgui());
}

void glRemix::glRemixRenderer::render()
{
	// Start ImGui frame
	m_context.start_imgui_frame();
	ImGui::ShowDemoWindow();

	// Be careful not to call the ID3D12Interface reset instead
	THROW_IF_FALSE(SUCCEEDED(m_cmd_pools[get_frame_index()].cmd_allocator->Reset()));

	// Create a command list in the open state
	ComPtr<ID3D12GraphicsCommandList7> cmd_list;
	THROW_IF_FALSE(m_context.create_command_list(cmd_list.ReleaseAndGetAddressOf(), m_cmd_pools[get_frame_index()]));

	const auto swapchain_idx = m_context.get_swapchain_index();

	// Transition swapchain image
	{
		D3D12_TEXTURE_BARRIER swapchain_render
		{
			.SyncBefore = D3D12_BARRIER_SYNC_DRAW, // Ensure we are not drawing anything to swapchain
			.SyncAfter = D3D12_BARRIER_SYNC_RENDER_TARGET, // Setting swapchain as render target requires transition to finish first

			.AccessBefore = D3D12_BARRIER_ACCESS_COMMON,
			.AccessAfter = D3D12_BARRIER_ACCESS_RENDER_TARGET,

			.LayoutBefore = D3D12_BARRIER_LAYOUT_PRESENT,
			.LayoutAfter = D3D12_BARRIER_LAYOUT_RENDER_TARGET,

			.Flags = D3D12_TEXTURE_BARRIER_FLAG_NONE // Or discard
		};
		m_context.set_barrier_swapchain(&swapchain_render);
		D3D12_BARRIER_GROUP barrier_group =
		{
			.Type = D3D12_BARRIER_TYPE_TEXTURE,
			.NumBarriers = 1,
			.pTextureBarriers = &swapchain_render,
		};
		cmd_list->Barrier(1, &barrier_group);
	}

	XMUINT2 win_dims{};
	m_context.get_window_dimensions(&win_dims);

	// Set viewport, scissor
	const D3D12_VIEWPORT viewport
	{
		.TopLeftX = 0,
		.TopLeftY = 0,
		.Width = static_cast<float>(win_dims.x),
		.Height = static_cast<float>(win_dims.y),
		.MinDepth = 0.0f,
		.MaxDepth = 1.0f,
	};
	const D3D12_RECT scissor_rect
	{
		.left = 0,
		.top = 0,
		.right = static_cast<LONG>(win_dims.x),
		.bottom = static_cast<LONG>(win_dims.y),
	};
	cmd_list->RSSetViewports(1, &viewport);
	cmd_list->RSSetScissorRects(1, &scissor_rect);

	// Build TLAS

	// Draw everything
	D3D12_CPU_DESCRIPTOR_HANDLE swapchain_rtv{};
	m_swapchain_descriptors.heap->get_cpu_descriptor(&swapchain_rtv, m_swapchain_descriptors.offset + swapchain_idx);
	const std::array clear_color = { 1.0f, 0.0f, 0.0f, 1.0f };
	cmd_list->OMSetRenderTargets(1, &swapchain_rtv, FALSE, nullptr);
	cmd_list->ClearRenderTargetView(swapchain_rtv, clear_color.data(), 0, nullptr);

	// This is where rasterization will go

	// Render ImGui
	m_context.render_imgui_draw_data(cmd_list.Get());

	// Transition swapchain image to present
	{
		D3D12_TEXTURE_BARRIER swapchain_present
		{
			.SyncBefore = D3D12_BARRIER_SYNC_DRAW, // Ensure we are not drawing anything to swapchain
			.SyncAfter = D3D12_BARRIER_SYNC_NONE, // Setting swapchain as render target requires transition to finish first

			.AccessBefore = D3D12_BARRIER_ACCESS_RENDER_TARGET,
			.AccessAfter = D3D12_BARRIER_ACCESS_NO_ACCESS,

			.LayoutBefore = D3D12_BARRIER_LAYOUT_RENDER_TARGET,
			.LayoutAfter = D3D12_BARRIER_LAYOUT_PRESENT,

			.Flags = D3D12_TEXTURE_BARRIER_FLAG_NONE // Or discard
		};
		m_context.set_barrier_swapchain(&swapchain_present);
		D3D12_BARRIER_GROUP barrier_group =
		{
			.Type = D3D12_BARRIER_TYPE_TEXTURE,
			.NumBarriers = 1,
			.pTextureBarriers = &swapchain_present,
		};
		cmd_list->Barrier(1, &barrier_group);
	}
		
	// Submit the command list
	auto current_fence_value = ++m_fence_frame_ready_val[get_frame_index()]; // Increment wait value
	THROW_IF_FALSE(SUCCEEDED(cmd_list->Close()));
	const std::array<ID3D12CommandList*, 1> lists = { cmd_list.Get() };
	m_gfx_queue.queue->ExecuteCommandLists(1, lists.data());

	// End of all work for queue, signal fence
	m_gfx_queue.queue->Signal(m_fence_frame_ready.fence.Get(), current_fence_value);

	// Present
	m_context.present();

	increment_frame_index();

	// If next frame is ready to be used, otherwise wait
	if (m_fence_frame_ready.fence->GetCompletedValue() < m_fence_frame_ready_val[get_frame_index()])
	{
		dx::WaitInfo wait_info
		{
			.wait_all = true,
			.count = 1,
			.fences = &m_fence_frame_ready,
			.values = &current_fence_value,
			.timeout = INFINITE
		};
		THROW_IF_FALSE(m_context.wait_fences(wait_info));
	}
	m_fence_frame_ready_val[get_frame_index()] = current_fence_value + 1;
}

void glRemix::glRemixRenderer::destroy()
{
	m_context.destroy_imgui();
}

glRemix::glRemixRenderer::~glRemixRenderer()
{

}
