#pragma once

#include <array>
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
    std::array<float, 3> position;
    std::array<float, 3> color;
    std::array<float, 3> normal;
};

// TODO add more parameters (such as enabled) when encountered
struct Light
{
    std::array<float, 4> ambient = { 0.f, 0.f, 0.f, 1.f };
    std::array<float, 4> diffuse = { 1.f, 1.f, 1.f, 1.f };
    std::array<float, 4> specular = { 1.f, 1.f, 1.f, 1.f };

    std::array<float, 4> position = { 0.f, 0.f, 1.f, 0.f };  // default head down

    std::array<float, 3> spot_direction = { 0.f, 0.f, -1.f };
    float spot_exponent = 0.f;
    float spot_cutoff = 180.f;

    float constant_attenuation = 1.f;
    float linear_attenuation = 0.f;
    float quadratic_attenuation = 0.f;
};

// TODO add more parameters if encountered
struct Material
{
    std::array<float, 4> ambient = { 1.f, 1.f, 1.f, 1.f };
    std::array<float, 4> diffuse = { 1.f, 1.f, 1.f, 1.f };
    std::array<float, 4> specular = { 0.f, 0.f, 0.f, 1.f };
    std::array<float, 4> emission = { 0.f, 0.f, 0.f, 1.f };

    float shininess = 0.f;
};

struct MeshRecord
{
    UINT32 mesh_id;       // will eventually be hashed

    UINT32 blas_idx;
    UINT32 mv_idx;        // index into model view array
    UINT32 vertex_idx;    // index into vertex buffer
    UINT32 index_idx;     // index into index buffer
    UINT32 mat_idx;
};

struct GPUMeshRecord
{
    UINT32 vertex_idx;
    UINT32 index_idx;
    UINT32 mat_idx;
};

}  // namespace glRemix
