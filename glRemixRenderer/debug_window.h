#pragma once

#include <imgui.h>
#include <structs.h>
#include <vector>
#include <functional>
#include <string>

namespace glRemix
{
class DebugWindow
{
    float m_fps = 0.0f;

    std::vector<MeshRecord> m_meshes;
    uint64_t m_meshID_to_replace = -1;
    std::string m_new_asset_path;
    std::function<void(uint64_t meshID, const std::string& asset_path)> m_replace_mesh_callback; // replace_mesh function from rt_app

    void render_performance_stats();
    void render_settings();
    void render_debug_log();
    void render_mesh_ids();

public:
    void render();

    void set_mesh_buffer(const std::vector<MeshRecord>& meshes);
    void set_replace_mesh_callback(
        std::function<void(uint64_t meshID, const std::string& asset_path)> callback
    );

};
}  // namespace glRemix
