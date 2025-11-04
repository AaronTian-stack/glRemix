#pragma once
#include "application.h"

namespace glRemix
{
	class glRemixRenderer : public Application
	{
		std::array<dx::D3D12CommandAllocator, m_frames_in_flight> m_cmd_pools{};

		ComPtr<ID3D12RootSignature> m_root_signature{};
		ComPtr<ID3D12PipelineState> m_raster_pipeline{};

		ComPtr<ID3D12RootSignature> m_rt_global_root_signature{};
		ComPtr<ID3D12StateObject> m_rt_pipeline{};

		dx::D3D12Buffer m_vertex_buffer{};
		dx::D3D12Buffer m_index_buffer{};

	protected:
		void create() override;
		void render() override;
		void destroy() override;

	public:
		glRemixRenderer() = default;
		~glRemixRenderer() override;
	};
}
