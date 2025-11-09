#pragma once
#include "application.h"
#include "dx/d3d12_as.h"
#include <DirectXMath.h>
#include <ipc_protocol.h>

namespace glRemix
{
	class glRemixRenderer : public Application
	{
        struct Vertex
        {
            std::array<float, 3> position;
            std::array<float, 3> color;
        };

		struct MeshRecord
		{
			uint32_t meshId; // will eventually be hashed
			uint32_t vertexOffset; // offset into vertex atlas
			uint32_t vertexCount;
			uint32_t indexOffset; // offset into index atlas
			uint32_t indexCount; // number of indices belonging to this mesh
			uint32_t blasID;
			uint32_t MVID; // index into model view array
			uint32_t texId;
		};

		struct alignas(16) MVP
		{
            DirectX::XMFLOAT4X4 model;
            DirectX::XMFLOAT4X4 view;
            DirectX::XMFLOAT4X4 proj;
        };

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

		dx::D3D12Buffer m_vertex_buffer{};
		dx::D3D12Buffer m_index_buffer{};

		dx::D3D12Buffer m_scratch_space{};
		dx::D3D12Buffer m_blas_buffer{};
		dx::D3D12TLAS m_tlas{};

		// Shader table buffer for ray tracing pipeline (contains raygen, miss, and hit group)
		dx::D3D12Buffer m_shader_table{};
		UINT64 m_raygen_shader_table_offset{};
		UINT64 m_miss_shader_table_offset{};
		UINT64 m_hit_group_shader_table_offset{};

        dx::D3D12Texture m_uav_render_target{};

		IPCProtocol m_ipc;

		// mesh resources
		std::vector<MeshRecord> m_meshes;
		std::vector<dx::D3D12Buffer> m_blas_buffers;

	protected:
		void create() override;
		void render() override;
		void destroy() override;

		void read_gl_command_stream();
		void read_geometry(std::vector<uint8_t>& ipcBuf,
                                            size_t& offset,
                                            std::vector<Vertex>& vertices,
                                            std::vector<uint32_t>& indices,
                                            glRemix::GLTopology topology,
                                            uint32_t bytesRead);


		// acceleration structure builders
		int build_mesh_blas(uint32_t vertex_count, uint32_t vertex_offset, uint32_t index_count, uint32_t index_offset, ComPtr<ID3D12GraphicsCommandList7> cmd_list);
		void build_tlas(ComPtr<ID3D12GraphicsCommandList7> cmd_list);

	public:
		glRemixRenderer() = default;
		~glRemixRenderer() override;
	};
}
