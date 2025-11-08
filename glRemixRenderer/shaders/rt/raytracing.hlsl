struct RayGenConstantBuffer
{
    float4x4 view_proj;
    float4x4 inv_view_proj;
    float width;
    float height;
};

RaytracingAccelerationStructure Scene : register(t0, space0);
RWTexture2D<float4> RenderTarget : register(u0);
ConstantBuffer<RayGenConstantBuffer> g_rayGenCB : register(b0);

// https://learn.microsoft.com/en-us/windows/win32/direct3d12/intersection-attributes
typedef BuiltInTriangleIntersectionAttributes MyAttributes;
struct RayPayload
{
    float4 color;
};

[shader("raygeneration")]
void RayGenMain()
{
    float2 uv = (float2)DispatchRaysIndex() / float2(g_rayGenCB.width, g_rayGenCB.height);

    float2 ndc = uv * 2.0f - 1.0f;
    ndc.y = -ndc.y; // Flip Y for correct screen space orientation

    // Transform from NDC to world space using inverse view-projection
    // Near plane point in clip space
    float4 near_point = float4(ndc, 0.0f, 1.0f);
    float4 far_point = float4(ndc, 1.0f, 1.0f);
    
    // Transform to world space
    float4 near_world = mul(near_point, g_rayGenCB.inv_view_proj);
    float4 far_world = mul(far_point, g_rayGenCB.inv_view_proj);
    
    near_world /= near_world.w;
    far_world /= far_world.w;
    
    float3 origin = near_world.xyz;
    float3 ray_dir = normalize(far_world.xyz - near_world.xyz);


    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = ray_dir;

    ray.TMin = 0.001;
    ray.TMax = 10000.0;
    RayPayload payload = { float4(0, 0, 0, 0) };
    // Note winding order if you don't see anything
    TraceRay(Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);


    RenderTarget[DispatchRaysIndex().xy] = payload.color;
    //RenderTarget[DispatchRaysIndex().xy] = float4(uv, 0.0, 1.0);
    //RenderTarget[DispatchRaysIndex().xy] = float4(ray_dir * 0.5 + 0.5, 1.0);
}

[shader("closesthit")]
void ClosestHitMain(inout RayPayload payload, in MyAttributes attr)
{
    float3 barycentrics = float3(1 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y);
    payload.color = float4(barycentrics, 1);
}

[shader("miss")]
void MissMain(inout RayPayload payload)
{
    payload.color = float4(1.0, 0.5, 0.0, 1.0);
}
