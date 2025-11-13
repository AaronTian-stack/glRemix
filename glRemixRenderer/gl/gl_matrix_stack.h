#pragma once

#include <DirectXMath.h>
#include <stack>

namespace glRemix::gl
{

enum class GLListMode : uint32_t {
    COMPILE = 0x1300,
    COMPILE_AND_EXECUTE = 0x1301,
};

enum class GLMatrixMode : uint32_t { MODELVIEW = 0x1700, PROJECTION = 0x1701, TEXTURE = 0x1702 };

class glMatrixStack
{
public:
    glMatrixStack();

    std::stack<DirectX::XMFLOAT4X4> model_view;
    std::stack<DirectX::XMFLOAT4X4> projection;
    std::stack<DirectX::XMFLOAT4X4> texture;

    void push(GLMatrixMode mode);
    void pop(GLMatrixMode mode);
    DirectX::XMFLOAT4X4& top(GLMatrixMode mode);
    void mulSet(GLMatrixMode mode, DirectX::XMMATRIX R);  // multiplies and sets top of stack

    // operations
    void identity(GLMatrixMode mode);
    void rotate(GLMatrixMode mode, float angle, float x, float y, float z);
    void translate(GLMatrixMode mode, float x, float y, float z);
    void frustum(GLMatrixMode mode, double l, double r, double b, double t, double n, double f);

    // debug
    void printStacks() const;
};

}  // namespace glRemix::gl
