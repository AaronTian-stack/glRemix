#pragma once
#include "application.h"
#include "dx/d3d12_as.h"
#include <DirectXMath.h>
#include <ipc_protocol.h>
#include <iostream>

namespace glRemix
{
	class glRemixRenderer : public Application
	{
        struct Vertex
        {
            std::array<float, 3> position;
            std::array<float, 3> color;
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
		
		float rot = 0;


	protected:
		void create() override;
		void render() override;
		void destroy() override;

		// TODO: Decoding should go in a separate module
		void readGLStream();
		void readGeometry(std::vector<uint8_t>& ipcBuf,
                                            size_t& offset,
                                            std::vector<Vertex>& vertices,
                                            std::vector<uint32_t>& indices,
                                            GLTopology topology,
                                            uint32_t bytesRead);

	public:
		glRemixRenderer() = default;
		~glRemixRenderer() override;
	};
}
