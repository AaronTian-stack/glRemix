#include "gl_matrix_stack.h"

#include <cmath>
#include <vector>

using namespace DirectX;
using namespace glRemix::gl;

static XMFLOAT4X4 LoadIdentity()
{
    XMFLOAT4X4 m;
    XMStoreFloat4x4(&m, XMMatrixIdentity());
    return m;
}

// "Initially there is only one matrix on each stack and all matrices are set to the identity"
// -gl 1.x spec pg 29
glMatrixStack::glMatrixStack()
{
    XMFLOAT4X4 I = LoadIdentity();

    model_view.push(I);
    projection.push(I);
    texture.push(I);
}

void glMatrixStack::identity(GLMatrixMode mode)
{
    std::stack<XMFLOAT4X4>* stack = nullptr;
    switch (mode)
    {
        case GLMatrixMode::MODELVIEW: stack = &model_view; break;
        case GLMatrixMode::PROJECTION: stack = &projection; break;
        case GLMatrixMode::TEXTURE: stack = &texture; break;
        default: return;
    }

    if (stack->empty())
    {
        return;
    }

    XMStoreFloat4x4(&stack->top(), XMMatrixIdentity());
}

void glMatrixStack::push(GLMatrixMode mode)
{
    switch (mode)
    {
        case GLMatrixMode::MODELVIEW:
            if (!model_view.empty())
            {
                model_view.push(model_view.top());
            }
            break;

        case GLMatrixMode::PROJECTION:
            if (!projection.empty())
            {
                projection.push(projection.top());
            }
            break;

        case GLMatrixMode::TEXTURE:
            if (!texture.empty())
            {
                texture.push(texture.top());
            }
            break;

        default: std::printf("Matrix Push: Unhandled matrix mode\n"); break;
    }
}

void glMatrixStack::pop(GLMatrixMode mode)
{
    switch (mode)
    {
        case GLMatrixMode::MODELVIEW:
            if (model_view.size() > 1)
            {
                model_view.pop();
            }
            break;

        case GLMatrixMode::PROJECTION:
            if (projection.size() > 1)
            {
                projection.pop();
            }
            break;

        case GLMatrixMode::TEXTURE:
            if (texture.size() > 1)
            {
                texture.pop();
            }
            break;

        default: break;
    }
}

XMFLOAT4X4& glMatrixStack::top(GLMatrixMode mode)
{
    switch (mode)
    {
        case GLMatrixMode::MODELVIEW: return model_view.top();

        case GLMatrixMode::PROJECTION: return projection.top();

        case GLMatrixMode::TEXTURE: return texture.top();

        default: {
            static XMFLOAT4X4 identity;
            XMStoreFloat4x4(&identity, XMMatrixIdentity());
            return identity;
        }
    }
}

void glMatrixStack::mulSet(GLMatrixMode mode, XMMATRIX R)
{
    std::stack<XMFLOAT4X4>* stack = nullptr;
    switch (mode)
    {
        case GLMatrixMode::MODELVIEW: stack = &model_view; break;
        case GLMatrixMode::PROJECTION: stack = &projection; break;
        case GLMatrixMode::TEXTURE: stack = &texture; break;
        default: return;
    }

    if (!stack || stack->empty())
    {
        return;
    }

    XMMATRIX M = XMLoadFloat4x4(&stack->top());

    XMMATRIX out = XMMatrixMultiply(R, M);

    XMStoreFloat4x4(&stack->top(), out);
}

// operations

void glMatrixStack::rotate(GLMatrixMode mode, float angle, float x, float y, float z)
{
    float len = std::sqrt(x * x + y * y + z * z);
    float invLen = 1.0f / len;

    XMVECTOR axis = XMVectorSet(x * invLen, y * invLen, z * invLen, 0.0f);
    float radians = XMConvertToRadians(angle);

    XMMATRIX R = XMMatrixRotationAxis(axis, radians);

    mulSet(mode, R);
}

void glMatrixStack::translate(GLMatrixMode mode, float x, float y, float z)
{
    XMMATRIX T = XMMatrixTranslation(x, y, z);

    mulSet(mode, T);
}

void glMatrixStack::frustum(GLMatrixMode mode, double l, double r, double b, double t, double n,
                            double f)
{
    const float L = static_cast<float>(l);
    const float R = static_cast<float>(r);
    const float B = static_cast<float>(b);
    const float T = static_cast<float>(t);
    const float N = static_cast<float>(n);
    const float F = static_cast<float>(f);

    XMMATRIX P = XMMatrixPerspectiveOffCenterRH(L, R, B, T, N, F);

    mulSet(mode, P);
}

void glMatrixStack::printStacks() const
{
    auto printMatrix = [](const DirectX::XMFLOAT4X4& m, const char* label, int level) {
        std::printf("[%s stack level %d]\n", label, level);
        std::printf("  %.6f  %.6f  %.6f  %.6f\n"
                    "  %.6f  %.6f  %.6f  %.6f\n"
                    "  %.6f  %.6f  %.6f  %.6f\n"
                    "  %.6f  %.6f  %.6f  %.6f\n",
                    m._11, m._12, m._13, m._14, m._21, m._22, m._23, m._24, m._31, m._32, m._33,
                    m._34, m._41, m._42, m._43, m._44);
    };

    auto dumpStack = [&](const std::stack<DirectX::XMFLOAT4X4>& s, const char* name) {
        if (s.empty())
        {
            std::printf("[%s stack] (empty)\n", name);
            return;
        }

        // Copy to preserve original
        std::stack<DirectX::XMFLOAT4X4> tmp = s;

        // Collect to vector to print bottom → top
        std::vector<DirectX::XMFLOAT4X4> mats;
        mats.reserve(tmp.size());
        while (!tmp.empty())
        {
            mats.push_back(tmp.top());
            tmp.pop();
        }

        // mats[0] is original top; reverse for bottom-first
        std::reverse(mats.begin(), mats.end());

        std::printf("=== %s stack (%zu matrices, bottom to top) ===\n", name, mats.size());
        for (size_t i = 0; i < mats.size(); ++i)
        {
            printMatrix(mats[i], name, static_cast<int>(i));
        }
    };

    std::printf("\n===== glMatrixStack dump =====\n");
    dumpStack(model_view, "MODELVIEW");
    dumpStack(projection, "PROJECTION");
    dumpStack(texture, "TEXTURE");
    std::printf("===== end dump =====\n\n");
}
