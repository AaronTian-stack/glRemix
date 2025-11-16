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
    float ambient[4] = { 0.f, 0.f, 0.f, 1.f };
    float diffuse[4] = { 1.f, 1.f, 1.f, 1.f };
    float specular[4] = { 1.f, 1.f, 1.f, 1.f };

    float position[4] = { 0.f, 0.f, 1.f, 0.f };  // default head down

    float spot_direction[3] = { 0.f, 0.f, -1.f };
    float spot_exponent = 0.f;
    float spot_cutoff = 180.f;

    float constant_attenuation = 1.f;
    float linear_attenuation = 0.f;
    float quadratic_attenuation = 0.f;
};

// TODO add more parameters if encountered
struct Material
{
    float ambient[4] = { 1.f, 1.f, 1.f, 1.f };
    float diffuse[4] = { 1.f, 1.f, 1.f, 1.f };
    float specular[4] = { 0.f, 0.f, 0.f, 1.f };
    float emission[4] = { 0.f, 0.f, 0.f, 1.f };

    float shininess = 0.f;
};

struct MeshRecord
{
    UINT32 mesh_id;      // will eventually be hashed
    UINT32 vertex_count;
    UINT32 index_count;  // number of indices belonging to this mesh

    // pointers
    UINT32 vertex_id;  // index into vertex buffer
    UINT32 index_id;   // index into index buffer
    UINT32 blas_id;
    UINT32 mv_id;      // index into model view array
    UINT32 mat_id;
    UINT32 tex_id;
};
}  // namespace glRemix
