#pragma once

#include "application.h"
#include "debug_window.h"
#include "dx/d3d12_as.h"
#include "gl/gl_matrix_stack.h"
#include <DirectXMath.h>
#include <ipc_protocol.h>
#include "structs.h"
#include "tsl/robin_map.h"

namespace glRemix
{
	class glRemixRenderer : public Application
	{
        std::array<dx::D3D12CommandAllocator, m_frames_in_flight> m_cmd_pools{};

		ComPtr<ID3D12RootSignature> m_root_signature{};
		ComPtr<ID3D12PipelineState> m_raster_pipeline{};

		ComPtr<ID3D12RootSignature> m_rt_global_root_signature{};
		ComPtr<ID3D12StateObject> m_rt_pipeline{};

		dx::D3D12DescriptorHeap m_rt_descriptor_heap{};
		dx::D3D12DescriptorTable m_rt_descriptors{};

		// TODO: Add other per frame constants as needed, rename accordingly
		// Copy parameters to this buffer each frame
		std::array<dx::D3D12Buffer, m_frames_in_flight> m_raygen_constant_buffers{};

		dx::D3D12Buffer m_scratch_space{};
		dx::D3D12TLAS m_tlas{};

		dx::D3D12Buffer m_meshes_buffer{};
        dx::D3D12Buffer m_materials_buffer{};
        dx::D3D12Buffer m_lights_buffer{};

		// Shader table buffer for ray tracing pipeline (contains raygen, miss, and hit group)
		dx::D3D12Buffer m_shader_table{};
		UINT64 m_raygen_shader_table_offset{};
		UINT64 m_miss_shader_table_offset{};
		UINT64 m_hit_group_shader_table_offset{};

        dx::D3D12Texture m_uav_render_target{};

		IPCProtocol m_ipc;

		// mesh resources
		tsl::robin_map<UINT64, MeshRecord> m_mesh_map;
		std::vector<MeshRecord> m_meshes;

		std::vector<dx::D3D12Buffer> m_blas_buffers;
		std::vector<dx::D3D12Buffer> m_vertex_buffers;
		std::vector<dx::D3D12Buffer> m_index_buffers;

		// matrix stack
        gl::glMatrixStack m_matrix_stack;
		std::vector<XMFLOAT4X4> m_matrix_pool;
		XMMATRIX inverse_view;

		// display lists
        tsl::robin_map<int, std::vector<UINT8>> m_display_lists;

		// state trackers
		gl::GLMatrixMode matrixMode = gl::GLMatrixMode::MODELVIEW; // "The initial matrix mode is MODELVIEW" - glspec pg. 29
		gl::GLListMode listMode = gl::GLListMode::COMPILE_AND_EXECUTE;

		std::array<float, 4> color = {1.0f, 1.0f, 1.0f, 1.0f}; // current color (may need to be tracked globally)
        std::array<float, 3> normal = {0.0f, 0.0f, 1.0f}; // global normal - default
		Material m_material; // global states that can be modified
		
		// shader resources
		std::array<Light, 8> m_lights{};
		std::vector<Material> m_materials;

		DebugWindow m_debug_window;

	protected:
		void create() override;
		void render() override;
		void destroy() override;

		void create_uav_rt();

		void read_gl_command_stream();
        void read_ipc_buffer(std::vector<UINT8>& ipcBuf, size_t start_offset, uint32_t bytesRead, ID3D12GraphicsCommandList7* cmd_list, bool callList = false);
        void read_geometry(std::vector<UINT8>& ipcBuf,
                           size_t& offset,
                           GLTopology topology,
                           UINT32 bytes_read,
                           ID3D12GraphicsCommandList7* cmd_list);


		// acceleration structure builders
		int build_mesh_blas(dx::D3D12Buffer& vertex_buffer, dx::D3D12Buffer& index_buffer, ID3D12GraphicsCommandList7* cmd_list);
		void build_tlas(ID3D12GraphicsCommandList7* cmd_list);

	public:
		glRemixRenderer() = default;
		~glRemixRenderer() override;
	};
}
