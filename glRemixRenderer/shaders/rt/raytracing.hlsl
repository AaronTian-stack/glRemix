struct RayGenConstantBuffer
{
    float width;
    float height;
    float4x4 projection_matrix;
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

    float4 ray_clip = float4(ndc, 1.0f, 1.0f);
    float4 ray_eye = mul(ray_clip, g_rayGenCB.projection_matrix);
    ray_eye = float4(ray_eye.xy, 1.0f, 0.0f);
    
    float3 origin = float3(0.0f, 0.0f, 0.0f);
    float3 rayDir = normalize(ray_eye.xyz);


    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = rayDir;

    ray.TMin = 0.001;
    ray.TMax = 10000.0;
    RayPayload payload = { float4(0, 0, 0, 0) };
    TraceRay(Scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, ~0, 0, 1, 0, ray, payload);


    RenderTarget[DispatchRaysIndex().xy] = payload.color;
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
    payload.color = float4(0, 0, 0, 1);
}
