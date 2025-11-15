struct RayGenConstantBuffer
{
    float4x4 view_proj;
    float4x4 inv_view_proj;
    float width;
    float height;
};

// Other stuff matches C layout
// TODO: Make shared file for these structs

struct MeshRecord
{
    uint vertex_idx;
    uint index_idx;
    // Materials are structured buffers. For now assume index 0
    // TODO: Add second buffer index
    uint mat_idx;
};

struct Material
{
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float4 emission;

    float shininess;
};

struct Light
{
    float4 ambient;
    float4 diffuse;
    float4 specular;

    float4 position;

    float3 spot_direction;
    float spot_exponent;
    float spot_cutoff;

    float constant_attenuation;
    float linear_attenuation;
    float quadratic_attenuation;
};

RaytracingAccelerationStructure scene : register(t0);

RWTexture2D<float4> render_target : register(u0);
ConstantBuffer<RayGenConstantBuffer> g_raygen_cb : register(b0);

StructuredBuffer<MeshRecord> meshes[] : register(t1);

// https://learn.microsoft.com/en-us/windows/win32/direct3d12/intersection-attributes
typedef BuiltInTriangleIntersectionAttributes TriAttributes;

struct RayPayload
{
    float4 color;
    float3 normal;
    bool hit;
    float3 hit_pos;
};

// helper functions
float rand(inout uint seed)
{
    seed = 1664525u * seed + 1013904223u;
    return float(seed & 0x00FFFFFFu) / 16777216.0f;
}

float3 cosine_sample_hemisphere(float2 xi)
{
    float r = sqrt(xi.x);
    float theta = 2.0f * 3.14159265f * xi.y;
    float x = r * cos(theta);
    float y = r * sin(theta);
    float z = sqrt(max(0.0, 1.0 - xi.x));
    return float3(x, y, z);
}

float3 transform_to_world(float3 local_dir, float3 N)
{
    float3 up = abs(N.z) < 0.999f ? float3(0, 0, 1) : float3(1, 0, 0);
    float3 tangent = normalize(cross(up, N));
    float3 bitangent = cross(N, tangent);
    return normalize(local_dir.x * tangent + local_dir.y * bitangent + local_dir.z * N);
}

[shader("raygeneration")] void RayGenMain()
{
    float2 uv = (float2)DispatchRaysIndex() / float2(g_raygen_cb.width, g_raygen_cb.height);

    float2 ndc = uv * 2.0f - 1.0f;
    ndc.y = -ndc.y;  // Flip Y for correct screen space orientation

    // Transform from NDC to world space using inverse view-projection
    // Near plane point in clip space
    float4 near_point = float4(ndc, 0.0f, 1.0f);
    float4 far_point = float4(ndc, 1.0f, 1.0f);  // look down -Z match gl convention

    // Transform to world space
    float4 near_world = mul(near_point, g_raygen_cb.inv_view_proj);
    float4 far_world = mul(far_point, g_raygen_cb.inv_view_proj);

    near_world /= near_world.w;
    far_world /= far_world.w;

    float3 origin = near_world.xyz;
    float3 ray_dir = normalize(far_world.xyz - near_world.xyz);

    float3 total_color = float3(0, 0, 0);
    float3 throughput = 1.0;
    uint max_bounces = 3;
    uint seed = uint(DispatchRaysIndex().x * 1973 + DispatchRaysIndex().y * 9277 + 891);
    RayPayload payload;

    for (uint bounce = 0; bounce < max_bounces; ++bounce)
    {
        payload.color = float4(0, 0, 0, 0);
        payload.normal = float3(0, 0, 0);
        payload.hit = false;

        RayDesc ray;
        ray.Origin = origin;
        ray.Direction = ray_dir;
        ray.TMin = 0.001;
        ray.TMax = 10000.0;

        // Note winding order if you don't see anything
        TraceRay(scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);

        if (!payload.hit)
        {
            // same as miss shader
            total_color += throughput * payload.color.rgb;
            break;
        }

        // hit - accumulate color
        total_color += throughput * payload.color.rgb;
        // throughput *= 0.6f;

        float2 xi = float2(rand(seed), rand(seed));
        float3 local_dir = cosine_sample_hemisphere(xi);
        float3 N = normalize(payload.normal);
        float3 new_dir = transform_to_world(local_dir, N);

        ray_dir = new_dir;
        origin = payload.hit_pos + ray_dir * 0.001f;
    }

    render_target[DispatchRaysIndex().xy] = float4(total_color, 1.0);
    // RenderTarget[DispatchRaysIndex().xy] = float4(uv, 0.0, 1.0);
    // RenderTarget[DispatchRaysIndex().xy] = float4(ray_dir * 0.5 + 0.5, 1.0);
}

    [shader("closesthit")] void ClosestHitMain(inout RayPayload payload, in TriAttributes attr)
{
    /*
    // check indexing logic
    uint instanceID = InstanceID();
    MeshRecord mesh = meshes[instanceID];
    Material material = materials[mesh.matID];
    Light light = lights[0];  // TODO add shading for all lights

    float3 hit_pos = WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
    float3 normal = normalize(-WorldRayDirection());  // TODO use vertex buffer to get actual normal
    float3 l_vec = normalize(light.position.xyz - hit_pos);

    float3 ambient = material.ambient.rgb;
    float normal_l_dot = max(dot(normal, l_vec), 0.0);
    float3 diffuse = material.diffuse.rgb + light.diffuse.rgb + normal_l_dot;

    float3 color = ambient + diffuse;
    payload.color = float4(color, 1.0f);*/

    uint instanceID = InstanceID();
    uint tri = PrimitiveIndex();
    float3 hit_pos = WorldRayOrigin() + RayTCurrent() * WorldRayDirection();

    // hacky gear color
    float3 albedo = float3(0, 0, 0);

    if (instanceID >= 0 && instanceID < 4)
    {
        albedo = float3(1, 0, 0);
    }
    else if (instanceID == 4 || instanceID == 5)
    {
        albedo = float3(1, 0, 0);
    }

    else if (instanceID >= 6 && instanceID < 10)
    {
        albedo = float3(0, 1, 0);
    }
    else if (instanceID == 10 || instanceID == 11)
    {
        albedo = float3(0, 1, 0);
    }

    else if (instanceID == 16 || instanceID == 17)
    {
        albedo = float3(0, 0, 1);
    }
    else
    {
        albedo = float3(0, 0, 1);
    }

    float3 light_pos = float3(10.0, 10.0, 10.0);
    float3 light_color = float3(1.0, 1.0, 1.0);
    float3 ambient = 0.1 * albedo;

    // hacky normal calculation - TODO use actual vertex normals
    tri /= 2;
    float3 normal = (tri.xxx % 3 == uint3(0, 1, 2)) * (tri < 3 ? -1 : 1);

    // float3 bary = float3(1.0 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x,
    // attr.barycentrics.y); float3 normal = normalize(float3(bary.x, bary.y, bary.z) * 2.0 - 1.0);

    float3 light_vec = normalize(light_pos - hit_pos);

    float normal_dot_light = max(dot(normal, light_vec), 0.0);
    float3 diffuse = albedo * light_color * normal_dot_light;

    float3 color = ambient + diffuse;

    // further TraceRay calls needed for reflective materials

    payload.hit_pos = hit_pos;
    payload.normal = normal;
    payload.hit = true;
    payload.color = float4(color, 1.0);  // using hacky colors right now, not diffuse
}

[shader("miss")] void MissMain(inout RayPayload payload)
{
    payload.color = float4(0.0, 0.0, 0.0, 1.0);
    payload.hit = false;
}
