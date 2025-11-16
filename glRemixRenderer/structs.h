#pragma once
#include <DirectXMath.h>

using namespace DirectX;

namespace glRemix
{
constexpr UINT64 MEGABYTE = 1024u * 1024u;
constexpr UINT CB_ALIGNMENT = 256;

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
    float ambient[4] = {0.f, 0.f, 0.f, 1.f};
    float diffuse[4] = {1.f, 1.f, 1.f, 1.f};
    float specular[4] = {1.f, 1.f, 1.f, 1.f};

    float position[4] = {0.f, 0.f, 1.f, 0.f};  // default head down

    float spotDirection[3] = {0.f, 0.f, -1.f};
    float spotExponent = 0.f;
    float spotCutoff = 180.f;

    float constantAttenuation = 1.f;
    float linearAttenuation = 0.f;
    float quadraticAttenuation = 0.f;
};

// TODO add more parameters if encountered
struct Material
{
    float ambient[4] = {1.f, 1.f, 1.f, 1.f};
    float diffuse[4] = {1.f, 1.f, 1.f, 1.f};
    float specular[4] = {0.f, 0.f, 0.f, 1.f};
    float emission[4] = {0.f, 0.f, 0.f, 1.f};

    float shininess = 0.f;
};

struct MeshRecord
{
    uint32_t mesh_id;      // will eventually be hashed
    uint32_t vertex_count;
    uint32_t index_count;  // number of indices belonging to this mesh

    // pointers
    uint32_t vertex_id;  // index into vertex buffer
    uint32_t index_id;   // index into index buffer
    uint32_t blas_id;
    uint32_t mv_id;      // index into model view array
    uint32_t mat_id;
    uint32_t tex_id;

    // garbage collection
    uint32_t last_frame; // last frame this mesh record was accessed
};

struct BufferPool
{
    std::vector<dx::D3D12Buffer> buffers;
    std::vector<uint32_t> free_indices;

    uint32_t push_back(dx::D3D12Buffer&& buffer)
    {
        if (!free_indices.empty())
        {
            uint32_t id = free_indices.back();
            free_indices.pop_back();

            buffers[id] = std::move(buffer);
            return id;
        }

        buffers.push_back(std::move(buffer));
        return static_cast<uint32_t>(buffers.size() - 1);
    }

    void destroy(uint32_t id)
    {
        buffers[id].destroy();
        free_indices.push_back(id);
    }

    dx::D3D12Buffer& operator[](uint32_t id) { return buffers[id]; }
};

struct alignas(16) MVP
{
    XMFLOAT4X4 model;
    XMFLOAT4X4 view;
    XMFLOAT4X4 proj;
};
}  // namespace glRemix
