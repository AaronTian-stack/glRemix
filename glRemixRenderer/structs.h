#pragma once

#include <DirectXMath.h>

using namespace DirectX;

namespace glRemix
{
struct RayGenConstantBuffer
{
    XMFLOAT4X4 projection_matrix;
    XMFLOAT4X4 inv_projection_matrix;
    float width;
    float height;
};

struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT3 color;
    XMFLOAT3 normal;
};

// TODO add more parameters (such as enabled) when encountered
struct Light
{
    XMFLOAT4 ambient = { 0.f, 0.f, 0.f, 1.f };
    XMFLOAT4 diffuse = { 1.f, 1.f, 1.f, 1.f };
    XMFLOAT4 specular = { 1.f, 1.f, 1.f, 1.f };

    XMFLOAT4 position = { 0.f, 0.f, 1.f, 0.f };  // default head down

    XMFLOAT3 spot_direction = { 0.f, 0.f, -1.f };
    float spot_exponent = 0.f;
    float spot_cutoff = 180.f;

    float constant_attenuation = 1.f;
    float linear_attenuation = 0.f;
    float quadratic_attenuation = 0.f;
};

// TODO add more parameters if encountered
struct Material
{
    XMFLOAT4 ambient = { 1.f, 1.f, 1.f, 1.f };
    XMFLOAT4 diffuse = { 1.f, 1.f, 1.f, 1.f };
    XMFLOAT4 specular = { 0.f, 0.f, 0.f, 1.f };
    XMFLOAT4 emission = { 0.f, 0.f, 0.f, 1.f };

    float shininess = 0.f;
};

struct MeshRecord
{
    UINT32 mesh_id;  // will eventually be hashed

    UINT32 blas_vb_ib_idx;
    UINT32 mv_idx;  // index into model view array
    UINT32 mat_idx;

    // For garbage collection, last frame this mesh record was accessed
    UINT32 last_frame;
};

struct GPUMeshRecord
{
    UINT32 vb_idx;
    UINT32 ib_idx;
    // Buffer index (since materials are multiple structured buffers)
    UINT32 mat_buffer_idx;
    // Index within that buffer
    UINT32 mat_idx;
};

}  // namespace glRemix
