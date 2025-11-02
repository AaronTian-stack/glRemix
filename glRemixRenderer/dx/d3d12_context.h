#pragma once

#include <array>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <dxgidebug.h>
#include <D3D12MemAlloc.h>
#include <DirectXMath.h>

#include "d3d12_command_allocator.h"
#include "d3d12_descriptor_heap.h"
#include "d3d12_fence.h"
#include "d3d12_queue.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

namespace glremix::dx
{
	class D3D12Context
	{
		ComPtr<IDXGIFactory6> m_dxgi_factory = nullptr;

		ComPtr<ID3D12Debug3> m_debug;
		ComPtr<IDXGIDebug1> m_dxgi_debug;

		ComPtr<ID3D12Device> m_device;
		ComPtr<D3D12MA::Allocator> m_allocator;

		HWND m_window = nullptr;
		ComPtr<IDXGISwapChain3> m_swapchain;
		std::array<ComPtr<ID3D12Resource>, 2> m_swapchain_buffers; // 2 is upper bound
		//std::array<Descriptor, 2> m_swapchain_descriptors{};
		D3D12Queue* m_swapchain_queue = nullptr;

		D3D12Fence m_fence_wait_all{};

	public:
		bool create(bool enable_debug_layer);

		bool get_window_dimensions(XMUINT2* dims);

		bool create_swapchain(HWND window, D3D12Queue* queue, UINT* frame_index);
		bool create_swapchain_descriptors(D3D12DescriptorTable* descriptors, D3D12DescriptorHeap* rtv_heap);
		UINT get_swapchain_index() const;
		bool present();

		void set_barrier_swapchain(D3D12_TEXTURE_BARRIER* barrier);
		//void set_barrier_resource(); // TODO: Take as input whatever abstraction is used for textures

		bool create_descriptor_heap(const D3D12_DESCRIPTOR_HEAP_DESC& desc, D3D12DescriptorHeap* heap, const char* debug_name = nullptr) const;
		//void set_descriptor_heap(ID3D12GraphicsCommandList7* cmd_list, const D3D12DescriptorHeap& heap) override;
		//void set_descriptor_heap(ID3D12GraphicsCommandList7* cmd_list, const D3D12DescriptorHeap& heap, const D3D12DescriptorHeap& sampler_heap) override;

		bool create_texture(const D3D12_RESOURCE_DESC1& desc, D3D12_BARRIER_LAYOUT init_layout, D3D12MA::Allocation** allocation, const char* debug_name = nullptr);
		bool create_render_target(const D3D12_RESOURCE_DESC1& desc, D3D12MA::Allocation** allocation, const char* debug_name = nullptr);

		bool create_queue(D3D12_COMMAND_LIST_TYPE type, D3D12Queue* queue, const char* debug_name = nullptr) const;
		bool create_command_allocator(D3D12CommandAllocator* cmd_allocator, D3D12Queue* queue, const char* debug_name = nullptr) const;
		bool create_command_list(ID3D12GraphicsCommandList7** cmd_list, const D3D12CommandAllocator& cmd_allocator, const char* debug_name = nullptr) const;

		bool create_fence(D3D12Fence* fence, uint64_t initial_value, const char* debug_name = nullptr) const;
		bool wait_fences(const WaitInfo& info) const;

		//virtual void init_imgui(TODO);
		//virtual void start_imgui_frame();
		//virtual void render_imgui_draw_data(ID3D12GraphicsCommandList7* cmd_list);
		//virtual void destroy_imgui();

		bool wait_idle(D3D12Queue* queue);

		~D3D12Context();

	};

	void set_debug_name(ID3D12Object* obj, const char* name);
}