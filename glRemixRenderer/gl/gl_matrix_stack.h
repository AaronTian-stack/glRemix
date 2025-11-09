#pragma once

#include <DirectXMath.h>
#include <stack>

namespace glRemix::gl
{

enum class GLMatrixMode : uint32_t
{
    MODELVIEW = 0x1700,
    PROJECTION = 0x1701,
    TEXTURE = 0x1702 // the 02 is just a guess im not sure what the actual enum expands to
};

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
    void mulSet(GLMatrixMode mode, DirectX::XMMATRIX R); // multiplies and sets top of stack

    // operations
    void identity(GLMatrixMode mode);
    void rotate(GLMatrixMode mode, float angle, float x, float y, float z);
    void translate(GLMatrixMode mode, float x, float y, float z);
    void frustum(GLMatrixMode mode, double l, double r, double b, double t, double n, double f);

    // debug
    void printStacks() const;
};

}  // namespace glRemix
