#pragma once

#include <array>
#include <vector>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <dxgidebug.h>
#include <D3D12MemAlloc.h>
#include <DirectXMath.h>
#include <d3d12shader.h>
#include <dxcapi.h>

#include "d3d12_buffer.h"
#include "d3d12_command_allocator.h"
#include "d3d12_descriptor_heap.h"
#include "d3d12_fence.h"
#include "d3d12_queue.h"
#include "d3d12_pipeline_types.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

namespace glRemix::dx
{
	class D3D12Context
	{
		D3D12DescriptorHeap m_imgui_srv_heap{};
		D3D12DescriptorTable m_imgui_font_desc{};
		D3D12_CPU_DESCRIPTOR_HANDLE m_imgui_font_cpu{}; // Persistent CPU handle for allocator callback
		D3D12_GPU_DESCRIPTOR_HANDLE m_imgui_font_gpu{}; // Persistent GPU handle for allocator callback

		ComPtr<IDXGIFactory6> m_dxgi_factory = nullptr;

		ComPtr<ID3D12Debug3> m_debug;
		ComPtr<IDXGIDebug1> m_dxgi_debug;

		ComPtr<ID3D12Device5> m_device;
		ComPtr<D3D12MA::Allocator> m_allocator;

		ComPtr<IDxcUtils> m_dxc_utils = nullptr;

		HWND m_window = nullptr;
		ComPtr<IDXGISwapChain3> m_swapchain;
		std::array<ComPtr<ID3D12Resource>, 2> m_swapchain_buffers; // 2 is upper bound
		D3D12Queue* m_swapchain_queue = nullptr;

		D3D12Fence m_fence_wait_all{};

		static DXGI_FORMAT mask_to_format(BYTE mask, D3D_REGISTER_COMPONENT_TYPE component_type);
		static std::vector<D3D12_INPUT_ELEMENT_DESC> shader_reflection(ID3D12ShaderReflection* shader_reflection,
		                                                                const D3D12_SHADER_DESC& shader_desc, bool increment_slot = false);

	public:
		bool create(bool enable_debug_layer);

		bool get_window_dimensions(XMUINT2* dims);

		bool create_swapchain(HWND window, D3D12Queue* queue, UINT* frame_index);
		bool create_swapchain_descriptors(D3D12DescriptorTable* descriptors, D3D12DescriptorHeap* rtv_heap);
		UINT get_swapchain_index() const;
		bool present();

		void set_barrier_swapchain(D3D12_TEXTURE_BARRIER* barrier);
		//void set_barrier_resource(); // TODO: Take as input whatever abstraction is used for textures

		bool create_buffer(const BufferDesc& desc, D3D12Buffer* buffer, const char* debug_name = nullptr) const;
		// Convenience functions
		bool map_buffer(D3D12Buffer* buffer, void** pointer);
		void unmap_buffer(D3D12Buffer* buffer);

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

		bool reflect_input_layout(IDxcBlob* vertex_shader, InputLayoutDesc* input_layout, bool increment_slot, ID3D12ShaderReflection** reflection);

		bool create_root_signature(const D3D12_ROOT_SIGNATURE_DESC& desc, ID3D12RootSignature** root_signature, const char* debug_name = nullptr) const;

		// TODO: Custom file loading, no wide char
		bool load_blob_from_file(const wchar_t* path, IDxcBlobEncoding** blob) const;

		bool create_graphics_pipeline(
			const GraphicsPipelineDesc& desc,
			IDxcBlob* vertex_shader, IDxcBlob* pixel_shader,
			ID3D12PipelineState** pipeline_state, const char* debug_name
		);

		bool create_raytracing_pipeline(
			const RayTracingPipelineDesc& desc,
			IDxcBlob* raytracing_shaders,
			ID3D12StateObject** state_object, const char* debug_name
		) const;

		// Note: ImGui using win32 is blurry, even the sample is like this, so I assume it's expected
		bool init_imgui();
		void start_imgui_frame();
		void render_imgui_draw_data(ID3D12GraphicsCommandList7* cmd_list);
		void destroy_imgui();

		bool wait_idle(D3D12Queue* queue);
		~D3D12Context();
	};

	void set_debug_name(ID3D12Object* obj, const char* name);
}