#pragma once
#include <stdexcept>
#include <windows.h>

#include "dx/d3d12_context.h"
#include "dx/d3d12_descriptor_heap.h"

#define THROW_IF_FALSE(cond) \
    do \
    { \
        if (!(cond)) \
            throw std::runtime_error(#cond " failed"); \
    } while (0)

namespace glRemix
{
class Application
{
public:
    static constexpr UINT m_frames_in_flight = 2;

protected:
    dx::D3D12Context m_context;
    dx::D3D12DescriptorHeap m_rtv_heap;
    dx::D3D12DescriptorTable m_swapchain_descriptors{};
    D3D12Queue m_gfx_queue;  // Direct command queue

    dx::D3D12Fence m_fence_frame_ready{};
    std::array<uint64_t, m_frames_in_flight> m_fence_frame_ready_val{0, 0};

    UINT m_frame_index = 0;
    bool m_quit = false;

    virtual void create() {}

    virtual void render() {}

    virtual void destroy() {}

public:
    UINT get_frame_index() const
    {
        return m_frame_index;
    }

    void increment_frame_index()
    {
        m_frame_index = (m_frame_index + 1) % m_frames_in_flight;
    }

    Application() = default;
    void run_with_hwnd(bool enable_debug_layer);

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;
    virtual ~Application() = default;
};
}  // namespace glRemix
