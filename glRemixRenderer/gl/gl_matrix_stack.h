#pragma once

#include <basetsd.h>
#include <DirectXMath.h>
#include <stack>

using namespace DirectX;

namespace glRemix::gl
{

enum class GLListMode : UINT32
{
    COMPILE = 0x1300,
    COMPILE_AND_EXECUTE = 0x1301,
};

enum class GLMatrixMode : UINT32
{
    MODELVIEW = 0x1700,
    PROJECTION = 0x1701,
    TEXTURE = 0x1702
};

class glMatrixStack
{
public:
    glMatrixStack();

    std::stack<XMFLOAT4X4> model_view;
    std::stack<XMFLOAT4X4> projection;
    std::stack<XMFLOAT4X4> texture;

    void push(GLMatrixMode mode);
    void pop(GLMatrixMode mode);
    XMFLOAT4X4& top(GLMatrixMode mode);
    void mul_set(GLMatrixMode mode, const XMMATRIX& r);  // multiplies and sets top of stack

    // operations
    void identity(GLMatrixMode mode);
    void rotate(GLMatrixMode mode, float angle, float x, float y, float z);
    void translate(GLMatrixMode mode, float x, float y, float z);
    void frustum(GLMatrixMode mode, double l, double r, double b, double t, double n, double f);

    // debug
    void print_stacks() const;
};

}  // namespace glRemix::gl
