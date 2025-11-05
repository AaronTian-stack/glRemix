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

		// TODO: Add other per frame constants as needed, rename accordingly
		// Copy parameters to this buffer each frame
		dx::D3D12Buffer m_raygen_constant_buffer{};

		dx::D3D12Buffer m_vertex_buffer{};
		dx::D3D12Buffer m_index_buffer{};

		dx::D3D12Buffer m_scratch_space{};
		dx::D3D12Buffer m_blas_buffer{};
		dx::D3D12TLAS m_tlas{};

		dx::D3D12Buffer m_mvp{};

		IPCProtocol m_ipc;
		
		float rot = 0;


	protected:
		void create() override;
		void render() override;
		void destroy() override;

		void readGLStream();
		void readGeometry(std::vector<uint8_t>& ipcBuf,
                                            size_t& offset,
                                            std::vector<Vertex>& vertices,
                                            std::vector<uint32_t>& indices,
                                            glRemix::GLTopology topology,
                                            uint32_t bytesRead);
		void updateMVP(float rot);

	public:
		glRemixRenderer() = default;
		~glRemixRenderer() override;
	};
}
