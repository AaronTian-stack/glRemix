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

	// TODO: transition swapchain image
	D3D12_TEXTURE_BARRIER swapchain_render
	{
		.SyncBefore = D3D12_BARRIER_SYNC_RAYTRACING | D3D12_BARRIER_SYNC_DRAW, // Ensure we are not drawing anything to swapchain
		.SyncAfter = D3D12_BARRIER_SYNC_RENDER_TARGET, // Setting swapchain as render target requires transition to finish first

		.AccessBefore = D3D12_BARRIER_ACCESS_COMMON,
		.AccessAfter = D3D12_BARRIER_ACCESS_RENDER_TARGET,

		.LayoutBefore = D3D12_BARRIER_LAYOUT_PRESENT,
		.LayoutAfter = D3D12_BARRIER_LAYOUT_RENDER_TARGET,

		.pResource = m_context.get_swapchain_buffer(swapchain_idx),
		.Subresources =
		{
			.IndexOrFirstMipLevel = 0,
			.NumMipLevels = 1,
			.FirstArraySlice = 0,
			.NumArraySlices = 1,
			// Most formats are single plane so ignore this
			.FirstPlane = 0,
			.NumPlanes = 1,
		},
		.Flags = D3D12_TEXTURE_BARRIER_FLAG_NONE // Or discard
	};

	

	// Set viewport, scissor
	////const D3D12_VIEWPORT viewport
	////{
	////	.TopLeftX = 0,
	////	.TopLeftY = 0,
	////	.Width = static_cast<float>(dim.x),
	////	.Height = static_cast<float>(dim.y),
	////	.MinDepth = 0.0f,
	////	.MaxDepth = 1.0f,
	////};
	////const D3D12_RECT scissor_rect
	////{
	////	.left = 0,
	////	.top = 0,
	////	.right = static_cast<LONG>(dim.x),
	////	.bottom = static_cast<LONG>(dim.y),
	////};

	// Build TLAS

	// Draw everything
	// This is where most of the rendering code will fit into

	// Transition swapchain image to present

	D3D12_TEXTURE_BARRIER swapchain_present{};

	// Submit the command list
	THROW_IF_FALSE(SUCCEEDED(cmd_list->Close()));

	// Present

	// If next frame is ready to be used, otherwise wait


	increment_frame_index();
}

void glremix::glRemixRenderer::destroy()
{
	
}

glremix::glRemixRenderer::~glRemixRenderer()
{

}
