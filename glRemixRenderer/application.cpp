#include "application.h"

using namespace glremix;

void Application::run(const HWND hwnd, const bool enable_debug_layer)
{
	// Setup and create default resources
	THROW_IF_FALSE(m_context.create(enable_debug_layer));

	THROW_IF_FALSE(m_context.create_queue(D3D12_COMMAND_LIST_TYPE_DIRECT, &m_gfx_queue));

	D3D12_DESCRIPTOR_HEAP_DESC desc
	{
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		.NumDescriptors = 32,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		.NodeMask = 0
	};
	THROW_IF_FALSE(m_context.create_descriptor_heap(desc, &m_rtv_heap, "default rtv heap"));

	THROW_IF_FALSE(m_context.create_swapchain(hwnd, &m_gfx_queue, &m_frame_index));

	THROW_IF_FALSE(m_context.create_swapchain_descriptors(&m_swapchain_descriptors, &m_rtv_heap));

	THROW_IF_FALSE(m_context.create_fence(&m_fence_frame_ready, m_fence_frame_ready_val[get_frame_index()]));

	create();

	bool quit = false;
	while (!quit)
	{
		// TODO: Delete this if statement after shim/IPC is integrated
		MSG msg{};
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				quit = true;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			render();
		}
	}

	THROW_IF_FALSE(m_context.wait_idle(&m_gfx_queue));
	destroy();

	for (UINT i = 0; i < m_frames_in_flight; i++)
	{
		m_rtv_heap.deallocate(&m_swapchain_descriptors);
	}
}
