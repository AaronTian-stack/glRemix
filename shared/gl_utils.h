#pragma once

#include <GL/gl.h>

namespace glRemix
{
namespace utils
{
/* Handled all cases specified here:
 * https://learn.microsoft.com/en-us/windows/win32/opengl/glteximage2d */
static inline UINT32 _BytesPerComponent(GLenum type)
{
    switch (type)
    {
        case GL_UNSIGNED_BYTE:
        case GL_BYTE:
        case GL_BITMAP: return 1;
        case GL_UNSIGNED_SHORT:
        case GL_SHORT: return 2;
        case GL_UNSIGNED_INT:
        case GL_INT:
        case GL_FLOAT: return 4;
        default: return 0;
    }
}

/* Handled all cases specified here:
 * https://learn.microsoft.com/en-us/windows/win32/opengl/glteximage2d */
static inline UINT32 _ComponentsPerPixel(GLenum format)
{
    switch (format)
    {
        case GL_RED:
        case GL_GREEN:
        case GL_BLUE:
        case GL_ALPHA:
        case GL_LUMINANCE: return 1;
        case GL_LUMINANCE_ALPHA: return 2;
        case GL_RGB: return 3;
        case GL_RGBA: return 4;
        default: return 0;
    }
}

inline UINT32 ComputePixelDataSize(GLsizei width, GLsizei height, GLenum format, GLenum type)
{
    return static_cast<UINT32>(width) * static_cast<UINT32>(height) * _ComponentsPerPixel(format)
           * _BytesPerComponent(type);
}

inline UINT32 ComputeClientArraySize(GLint count, GLint size, GLenum type, GLint stride)
{
    UINT32 t = 0;
    switch (type)
    {
        case GL_UNSIGNED_BYTE:
        case GL_BYTE: t = 1; break;

        case GL_SHORT:
        case GL_UNSIGNED_SHORT: t = 2; break;

        case GL_INT:
        case GL_UNSIGNED_INT:
        case GL_FLOAT: t = 4; break;

        case GL_DOUBLE: t = 8; break;

        default: assert(false && "glHooks - Unsupported client array GL type."); return 0;
    }

    if (stride == 0)
    {
        stride = size * t;
    }

    return count * stride;
}

inline UINT32 ComputeIndexArraySize(GLint count, GLenum type)
{
    switch (type)
    {
        case GL_UNSIGNED_BYTE: return count * 1;
        case GL_UNSIGNED_SHORT: return count * 2;
        case GL_UNSIGNED_INT: return count * 4;

        default: assert(false && "glHooks - Unsupported index array GL type"); return 0;
    }
}
}  // namespace utils
}  // namespace glRemix
