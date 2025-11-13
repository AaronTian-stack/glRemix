#include "d3d12_pipeline_types.h"

using namespace glRemix::dx;

RayTracingPipelineDesc make_ray_tracing_pipeline_desc(const wchar_t* ray_gen_shader,
                                                      const wchar_t* miss_shader,
                                                      const wchar_t* closest_hit_shader,
                                                      const wchar_t* any_hit_shader = nullptr,
                                                      const wchar_t* intersection_shader = nullptr)
{
    RayTracingPipelineDesc desc{};
    desc.export_names[static_cast<UINT>(ExportType::RAY_GEN)] = ray_gen_shader;
    desc.export_names[static_cast<UINT>(ExportType::MISS)] = miss_shader;
    desc.export_names[static_cast<UINT>(ExportType::CLOSEST_HIT)] = closest_hit_shader;
    desc.export_names[static_cast<UINT>(ExportType::ANY_HIT)] = any_hit_shader;
    desc.export_names[static_cast<UINT>(ExportType::INTERSECTION)] = intersection_shader;

    // Count non-null exports
    UINT count = 0;
    if (ray_gen_shader)
    {
        count++;
    }
    if (miss_shader)
    {
        count++;
    }
    if (closest_hit_shader)
    {
        count++;
    }
    if (any_hit_shader)
    {
        count++;
    }
    if (intersection_shader)
    {
        count++;
    }
    desc.num_exports = count;

    return desc;
}
