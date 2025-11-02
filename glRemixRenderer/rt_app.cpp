#include "rt_app.h"

void glremix::glRemixRenderer::create()
{
	for (UINT i = 0; i < m_frames_in_flight; i++)
	{
		THROW_IF_FALSE(m_context.create_command_allocator(&m_cmd_pools[i], &m_gfx_queue, "frame command allocator"));
	}
	// Compile ray tracing pipeline
	// Build BLAS here for now, but renderer will construct them dynamically for new geometry in render loop
}

void glremix::glRemixRenderer::render()
{
	// TODO: start ImGui frame

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
	const std::array clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
	//cmd_list->OMSetRenderTargets(1, &swapchain_rtv, )
	cmd_list->ClearRenderTargetView(swapchain_rtv, clear_color.data(), 0, nullptr);

	// This is where most of the rendering code will fit into

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

void glremix::glRemixRenderer::destroy()
{
	
}

glremix::glRemixRenderer::~glRemixRenderer()
{

}
