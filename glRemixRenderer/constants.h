#pragma once
#include <DirectXMath.h>

using namespace DirectX;

namespace glRemix
{
    struct RayGenConstantBuffer
    {
        float width;
        float height;
        XMFLOAT4X4 projection_matrix;
    };
}
