#include "debug_window.h"
#include "imgui.h"

using namespace glRemix;

void DebugWindow::render()
{
	if (ImGui::Begin("glRemix", nullptr, 0))
	{
		if (ImGui::BeginTabBar("DebugTabs"))
		{
			if (ImGui::BeginTabItem("Performance"))
			{
				render_performance_stats();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Render Settings"))
			{
				render_settings();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Debug Log"))
			{
				render_debug_log();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
	ImGui::End();
}

void DebugWindow::render_performance_stats()
{
	const ImGuiIO& io = ImGui::GetIO();
	m_fps = io.Framerate;
	
	ImGui::Text("FPS: %.1f (%.3f ms/frame)", m_fps, 1000.f / m_fps);
}

void DebugWindow::render_settings()
{
    // TODO: Reload environment map, shaders, etc
}

void DebugWindow::render_debug_log()
{
    // TODO: Debug messages
}
