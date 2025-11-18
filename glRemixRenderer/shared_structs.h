// Structs shared between C++ and HLSL shaders

#pragma once

#include "hlsl_compat.h"

struct RayGenConstantBuffer
{
    XMFLOAT4X4 view_proj;
    XMFLOAT4X4 inv_view_proj;
    float width;
    float height;
};

struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT3 color;
    XMFLOAT3 normal;
};

struct Light
{
    XMFLOAT4 ambient;
    XMFLOAT4 diffuse;
    XMFLOAT4 specular;

    XMFLOAT4 position;

    XMFLOAT3 spot_direction;
    float spot_exponent;
    float spot_cutoff;

    float constant_attenuation;
    float linear_attenuation;
    float quadratic_attenuation;
};

struct Material
{
    XMFLOAT4 ambient;
    XMFLOAT4 diffuse;
    XMFLOAT4 specular;
    XMFLOAT4 emission;

    float shininess;
};

struct GPUMeshRecord
{
    UINT32 vb_idx;
    UINT32 ib_idx;
    UINT32 mat_buffer_idx;
    UINT32 mat_idx;
};
