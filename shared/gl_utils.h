#pragma once

#include <GL/gl.h>

namespace glRemix
{
namespace utils
{
/* Handled all cases specified here:
 * https://learn.microsoft.com/en-us/windows/win32/opengl/glteximage2d */
static inline UINT32 _BytesPerComponentType(GLenum type)
{
    switch (type)
    {
        case GL_UNSIGNED_BYTE:
        case GL_BYTE:
        case GL_BITMAP: return 1;
        case GL_UNSIGNED_SHORT:
        case GL_SHORT:
        case GL_UNSIGNED_INT:
        case GL_INT:
        case GL_FLOAT: return 4;
        case GL_DOUBLE: return 8;
        default: return 0;
    }
}

/* Handled all cases specified here:
 * https://learn.microsoft.com/en-us/windows/win32/opengl/glteximage2d */
static inline UINT32 _ComponentsPerPixelFormat(GLenum format)
{
    switch (format)
    {
        case GL_RED:
        case GL_GREEN:
        case GL_BLUE:
        case GL_ALPHA:
        case GL_LUMINANCE: return 1;
        case GL_LUMINANCE_ALPHA:
        case GL_RGB: return 3;
        case GL_RGBA: return 4;
        default: return 0;
    }
}

static inline UINT32 ComputePixelDataSize(GLsizei width, GLsizei height, GLenum format, GLenum type)
{
    return static_cast<UINT32>(width) * static_cast<UINT32>(height)
           * _ComponentsPerPixelFormat(format) * _BytesPerComponentType(type);
}

static inline UINT32 ComputeStride(GLint size, GLenum type, GLint stride)
{
    UINT32 bytes_per_component = _BytesPerComponentType(type);

    if (stride == 0)
    {
        stride = size * bytes_per_component;
    }
    return stride;
}

static inline UINT32 ComputeClientArraySize(GLint count, GLint size, GLenum type, GLint stride)
{
    stride = ComputeStride(size, type, stride);

    return count * stride;
}

static inline GLRemixClientArrayKind MapTo(GLenum cap)
{
    switch (cap)
    {
        case GL_VERTEX_ARRAY: return GLRemixClientArrayKind::VERTEX;
        case GL_NORMAL_ARRAY: return GLRemixClientArrayKind::NORMAL;
        case GL_COLOR_ARRAY: return GLRemixClientArrayKind::COLOR;
        case GL_TEXTURE_COORD_ARRAY: return GLRemixClientArrayKind::TEXCOORD;
        case GL_INDEX_ARRAY: return GLRemixClientArrayKind::COLORIDX;
        case GL_EDGE_FLAG_ARRAY: return GLRemixClientArrayKind::EDGEFLAG;
        default: return GLRemixClientArrayKind::_INVALID;
    }
}

static inline UINT32 FindMaxIndexValue(const void* indices, UINT32 count, GLenum type)
{
    switch (type)
    {
        case GL_UNSIGNED_BYTE:
        {
            const UINT8* idx = reinterpret_cast<const UINT8*>(indices);

            UINT8 max_val = 0;
            for (UINT32 i = 0; i < count; ++i)
            {
                if (idx[i] > max_val)
                {
                    max_val = idx[i];
                }
            }
            return static_cast<UINT32>(max_val);
        }

        case GL_UNSIGNED_SHORT:
        {
            const UINT16* idx = reinterpret_cast<const UINT16*>(indices);

            UINT16 max_val = 0;
            for (UINT32 i = 0; i < count; ++i)
            {
                if (idx[i] > max_val)
                {
                    max_val = idx[i];
                }
            }
            return static_cast<UINT32>(max_val);
        }

        case GL_UNSIGNED_INT:
        {
            const UINT32* idx = reinterpret_cast<const UINT32*>(indices);

            UINT32 max_val = 0;
            for (UINT32 i = 0; i < count; ++i)
            {
                if (idx[i] > max_val)
                {
                    max_val = idx[i];
                }
            }
            return max_val;
        }

        default: return 0;
    }
}
}  // namespace utils
}  // namespace glRemix
