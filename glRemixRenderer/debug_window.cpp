#include "debug_window.h"
#include "imgui.h"
// #include "structs.h"
// #include <cstdio>

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

        if (ImGui::BeginTabBar("Asset Replacement"))
        {
            if (ImGui::BeginTabItem("Mesh IDs"))
            {
                render_mesh_ids();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}

// get mesh buffer from rt_app
void DebugWindow::set_mesh_buffer(const std::vector<MeshRecord> meshes)
{
    m_meshes = meshes;
}

// get replace_mesh function from rt_app
void DebugWindow::set_replace_mesh_callback(
    std::function<void(uint64_t meshID, const std::string& asset_path)> callback)
{
    m_replace_mesh_callback = callback;
}

void DebugWindow::render_mesh_ids()
{
    ImGui::Text("Asset Replacement");
    ImGui::Text("List of Assets");

    // render meshIDs and get selected mesh
    if (ImGui::BeginListBox("##assets"))
    {
        for (int i = 0; i < m_meshes.size(); i++)
        {
            uint64_t meshID = m_meshes[i].mesh_id;

            const bool is_selected = (m_meshID_to_replace == meshID);
            char buf[64];
            snprintf(buf, 64, "Mesh ID: %llu", meshID);
            if (ImGui::Selectable(buf, is_selected))
            {
                m_meshID_to_replace = meshID;
            }
        }
        ImGui::EndListBox();
    }

    m_meshID_to_replace = 2564736516315296377;
    m_new_asset_path = "C:/Users/anyaa/OneDrive/Desktop/UPenn/CIS5650/avocado_gltf/Avocado.gltf";

    m_replace_mesh_callback(m_meshID_to_replace, m_new_asset_path);

    // handle asset replacement with selected mesh
    if (m_meshID_to_replace != -1)
    {
        ImGui::Separator();

        // get new asset path from user input
        char new_asset_path_char[256] = "";
        ImGui::InputText("Replacement Asset Path", new_asset_path_char, sizeof(new_asset_path_char));
        m_new_asset_path = new_asset_path_char;

        // if button is pressed to replace asset, call replace_mesh from rt_app
        if (ImGui::Button("Replace Asset"))
        {
            if (m_replace_mesh_callback)
            {
                m_replace_mesh_callback(m_meshID_to_replace, m_new_asset_path);
            }
        }
    }
}

void DebugWindow::render_performance_stats()
{
    const ImGuiIO& io = ImGui::GetIO();
    m_fps = io.Framerate;

    ImGui::Text("FPS: %.1f (%.3f ms/frame)", m_fps, 1000.0f / m_fps);
}

void DebugWindow::render_settings()
{
    // TODO: Reload environment map, shaders, etc
}

void DebugWindow::render_debug_log()
{
    // TODO: Debug messages
}
