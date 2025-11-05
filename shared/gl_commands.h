// Structures for representing OpenGL commands to be sent to the renderer process
// Not the same as the OpenGL API calls coming from gl.xml

#pragma once

#include <cstdint>

namespace glRemix
{
    enum class GLCommandType : uint32_t
    {
        // Basic OpenGL 1.x commands
        GL_BEGIN = 1,
        GL_END,
        GL_VERTEX2F,
        GL_VERTEX3F,
        GL_COLOR3F,
        GL_COLOR4F,
        GL_NORMAL3F,
        GL_TEXCOORD2F,

        // Matrix operations
        GL_MATRIX_MODE,
        GL_LOAD_IDENTITY,
        GL_LOAD_MATRIX,
        GL_MULT_MATRIX,
        GL_PUSH_MATRIX,
        GL_POP_MATRIX,
        GL_TRANSLATE,
        GL_ROTATE,
        GL_SCALE,

        // Texture operations
        GL_BIND_TEXTURE,
        GL_GEN_TEXTURES,
        GL_DELETE_TEXTURES,
        GL_TEX_IMAGE_2D,
        GL_TEX_PARAMETER,

        // Lighting
        GL_ENABLE,
        GL_DISABLE,
        GL_LIGHT,
        GL_LIGHTF,
        GL_LIGHTFV,
        GL_MATERIAL,
        GL_MATERIALF,
        GL_MATERIALFV,

        // Buffer operations
        GL_CLEAR,
        GL_CLEAR_COLOR,
        GL_FLUSH,
        GL_FINISH,

        // Viewport and projection
        GL_VIEWPORT,
        GL_ORTHO,
        GL_FRUSTUM,
        GL_PERSPECTIVE,

        // Other
        GL_SWAP_BUFFERS,
        GL_SHUTDOWN
    };

    enum class GLTopology : uint32_t
    {
        GL_QUADS = 0x0007,
        GL_QUAD_STRIP = 0x0008
    };

    // Name *Unifs for clear association
    // Header for all commands
    struct GLCommandUnifs
    {
        GLCommandType type;
        uint32_t dataSize;
    };

    // Specific command structures
    struct GLBeginCommand
    {
        uint32_t mode;  // GL_TRIANGLES, GL_QUADS, etc.
    };

    struct GLEndCommand {
        uint32_t reserved = 0; // to maintain alignment. think of as padding GPUBuffers
    };

    // TODO: Compress these?

    // NOTE FOR PERSON DOING SHIM/IPC:
	// It is not strictly required to have separate structs for each command, but it may make serialization/deserialization more clear.
	// You could just use the DirectXMath XM types (storage) for vectors of 2/3/4 floats/ints/uints etc.
    // If you choose to go that route, you can delete the below structs.

    struct GLVertex3fCommand
    {
        float x, y, z;
    };

    struct GLColor4fCommand
    {
        float r, g, b, a;
    };

    struct GLNormal3fCommand
    {
        float nx, ny, nz;
    };

    struct GLTexCoord2fCommand
    {
        float s, t;
    };

    struct GLViewportCommand
    {
        int x, y;
        int width, height;
    };

    struct GLClearColorCommand
    {
        float r, g, b, a;
    };

    // TODO: Add more structs as needed
}