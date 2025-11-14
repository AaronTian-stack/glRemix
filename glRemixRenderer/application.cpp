#include "application.h"

using namespace glRemix;

void Application::run_with_hwnd(const bool enable_debug_layer)
{
    // Setup and create default resources
    THROW_IF_FALSE(m_context.create(enable_debug_layer));

    THROW_IF_FALSE(m_context.create_queue(D3D12_COMMAND_LIST_TYPE_DIRECT, &m_gfx_queue));

    D3D12_DESCRIPTOR_HEAP_DESC desc{ .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
                                     .NumDescriptors = 32,
                                     .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
                                     .NodeMask = 0 };
    THROW_IF_FALSE(m_context.create_descriptor_heap(desc, &m_rtv_heap, "default rtv heap"));

    // Swapchain creation is deferred until we receive HWND from command stream

    THROW_IF_FALSE(
        m_context.create_fence(&m_fence_frame_ready, m_fence_frame_ready_val[get_frame_index()]));

    create();

    bool quit = false;
    while (!quit)
    {
        // Process window messages for ImGui input
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
            {
                quit = true;
            }
        }

        // TODO: Get quit signal from IPC and get it out of the app class?
        // Right now this process just gets killed externally
        render();
    }

    THROW_IF_FALSE(m_context.wait_idle(&m_gfx_queue));
    destroy();

    for (UINT i = 0; i < m_frames_in_flight; i++)
    {
        m_rtv_heap.deallocate(&m_swapchain_descriptors);
    }
}
