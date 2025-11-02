#pragma once
#include "application.h"

namespace glremix
{
	class glRemixRenderer : public Application
	{
		std::array<dx::D3D12CommandAllocator, m_frames_in_flight> m_cmd_pools{};

	protected:
		void create() override;
		void render() override;
		void destroy() override;

	public:
		glRemixRenderer() = default;
		~glRemixRenderer() override;
	};
}
