#pragma once

namespace glRemix
{
class DebugWindow
{
    float m_fps = 0.0f;

    void render_performance_stats();
    void render_settings();
    void render_debug_log();

public:
    struct
    {
        bool unlocked = false;
    } m_parameters;

    void render();
};
}  // namespace glRemix
