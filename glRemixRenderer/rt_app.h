#pragma once
#include "application.h"

namespace glRemix
{
	class glRemixRenderer : public Application
	{
		std::array<dx::D3D12CommandAllocator, m_frames_in_flight> m_cmd_pools{};

		ComPtr<ID3D12RootSignature> m_root_signature{};
		ComPtr<ID3D12PipelineState> m_raster_pipeline{};

	protected:
		void create() override;
		void render() override;
		void destroy() override;

	public:
		glRemixRenderer() = default;
		~glRemixRenderer() override;
	};
}
