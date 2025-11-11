#pragma once
#include <DirectXMath.h>

using namespace DirectX;

namespace glRemix
{
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
        uint32_t meshId;        // hashed
        uint32_t vertexOffset;  // offset into vertex atlas
        uint32_t vertexCount;
        uint32_t indexOffset;   // offset into index atlas
        uint32_t indexCount;    // number of indices belonging to this mesh

        // pointers
        uint32_t blasID;
        uint32_t MVID;  // index into model view array
        uint32_t matID;
        uint32_t texID;
    };

	struct MVP
    {
        XMFLOAT4X4 model;
        XMFLOAT4X4 view;
        XMFLOAT4X4 proj;
    };
}
