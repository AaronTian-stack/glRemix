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
}
