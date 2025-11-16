// Structures for representing OpenGL commands to be sent to the renderer process
// Not the same as the OpenGL API calls coming from gl.xml

#pragma once

#include <cstdint>

namespace glRemix
{

enum class GLTopology : UINT32
{
    GL_QUADS = 0x0007,
    GL_QUAD_STRIP = 0x0008
};

enum class GLLight : UINT32
{
    // light name
    GL_LIGHT0 = 0x4000,
    GL_LIGHT1 = 0x4001,
    GL_LIGHT2 = 0x4002,
    GL_LIGHT3 = 0x4003,
    GL_LIGHT4 = 0x4004,
    GL_LIGHT5 = 0x4005,
    GL_LIGHT6 = 0x4006,
    GL_LIGHT7 = 0x4007,

    // light parameter
    GL_AMBIENT = 0x1200,
    GL_DIFFUSE = 0x1201,
    GL_SPECULAR = 0x1202,
    GL_POSITION = 0x1203,
    GL_SPOT_DIRECTION = 0x1204,
    GL_SPOT_EXPONENT = 0x1205,
    GL_SPOT_CUTOFF = 0x1206,
    GL_CONSTANT_ATTENUATION = 0x1207,
    GL_LINEAR_ATTENUATION = 0x1208,
    GL_QUADRATIC_ATTENUATION = 0x1209
};

enum class GLMaterial : UINT32
{
    // face
    GL_NONE = 0x0000,
    GL_FRONT_LEFT = 0x0400,
    GL_FRONT_RIGHT = 0x0401,
    GL_BACK_LEFT = 0x0402,
    GL_BACK_RIGHT = 0x0403,
    GL_FRONT = 0x0404,
    GL_BACK = 0x0405,
    GL_LEFT = 0x0406,
    GL_RIGHT = 0x0407,
    GL_FRONT_AND_BACK = 0x0408,
    GL_AUX0 = 0x0409,
    GL_AUX1 = 0x040A,
    GL_AUX2 = 0x040B,
    GL_AUX3 = 0x040C,

    // property
    GL_EMISSION = 0x1600,
    GL_SHININESS = 0x1601,
    GL_AMBIENT_AND_DIFFUSE = 0x1602,
    GL_COLOR_INDEXES = 0x1603
};

enum class GLCommandType : UINT32
{
    // Basic OpenGL 1.x commands
    GLCMD_BEGIN = 1,
    GLCMD_END = 2,
    GLCMD_VERTEX2F = 3,
    GLCMD_VERTEX3F = 4,
    GLCMD_COLOR3F = 5,
    GLCMD_COLOR4F = 6,
    GLCMD_NORMAL3F = 7,
    GLCMD_TEXCOORD2F = 8,

    // Matrix operations
    GLCMD_MATRIX_MODE = 9,
    GLCMD_LOAD_IDENTITY = 10,
    GLCMD_LOAD_MATRIX = 11,
    GLCMD_MULT_MATRIX = 12,
    GLCMD_PUSH_MATRIX = 13,
    GLCMD_POP_MATRIX = 14,
    GLCMD_TRANSLATE = 15,
    GLCMD_ROTATE = 16,
    GLCMD_SCALE = 17,

    // Texture operations
    GLCMD_BIND_TEXTURE = 18,
    GLCMD_GEN_TEXTURES = 19,
    GLCMD_DELETE_TEXTURES = 20,
    GLCMD_TEX_IMAGE_2D = 21,
    GLCMD_TEX_PARAMETER = 22,

    // Lighting
    GLCMD_ENABLE = 23,
    GLCMD_DISABLE = 24,
    GLCMD_LIGHT = 25,
    GLCMD_LIGHTF = 26,
    GLCMD_LIGHTFV = 27,
    GLCMD_MATERIAL = 28,
    GLCMD_MATERIALF = 29,
    GLCMD_MATERIALFV = 30,

    // Buffer operations
    GLCMD_CLEAR = 31,
    GLCMD_CLEAR_COLOR = 32,
    GLCMD_FLUSH = 33,
    GLCMD_FINISH = 34,

    // Viewport and projection
    GLCMD_VIEWPORT = 35,
    GLCMD_ORTHO = 36,
    GLCMD_FRUSTUM = 37,
    GLCMD_PERSPECTIVE = 38,

    // Other
    GLCMD_CREATE = 39,
    GLCMD_SHUTDOWN = 40,

    // Display Lists
    GLCMD_CALL_LIST = 41,
    GLCMD_NEW_LIST = 42,
    GLCMD_END_LIST = 43,
};

struct GLVec2f
{
    float x, y;
};

struct GLVec3f
{
    float x, y, z;
};

struct GLVec4f
{
    float x, y, z, w;
};

struct GLVec3d
{
    double x, y, z;
};

struct GLVec4d
{
    double x, y, z, w;
};

struct GLEmptyCommand
{
    UINT32 reserved = 0;  // to maintain alignment. think of as padding GPUBuffers
};

// Name *Unifs for clear association
// Header for all commands
struct GLCommandUnifs
{
    GLCommandType type;
    UINT32 dataSize;
};

// Specific command structures
struct GLBeginCommand
{
    UINT32 mode;  // GL_TRIANGLES, GL_QUADS, etc.
};

using GLEndCommand = GLEmptyCommand;

using GLVertex2fCommand = GLVec2f;
using GLVertex3fCommand = GLVec3f;
using GLColor3fCommand = GLVec3f;
using GLColor4fCommand = GLVec4f;
using GLNormal3fCommand = GLVec3f;
using GLTexCoord2fCommand = GLVec2f;

// Matrix operations
struct GLMatrixModeCommand
{
    UINT32 mode;
};

struct GLLoadMatrixCommand
{
    float m[16];
};

struct GLMultMatrixCommand
{
    float m[16];
};

using GLLoadIdentityCommand = GLEmptyCommand;
using GLPushMatrixCommand = GLEmptyCommand;
using GLPopMatrixCommand = GLEmptyCommand;

struct GLTranslateCommand
{
    GLVec3f t;
};

struct GLRotateCommand
{
    float angle;
    GLVec3f axis;
};

struct GLScaleCommand
{
    GLVec3f s;
};

// Texture operations
struct GLBindTextureCommand
{
    UINT32 target;
    UINT32 texture;
};

struct GLGenTexturesCommand
{
    UINT32 n;
};

struct GLDeleteTexturesCommand
{
    UINT32 n;
};

struct GLTexImage2DCommand
{
    UINT32 target;
    UINT32 level;
    UINT32 internalFormat;  // must be same as format.
    UINT32 width;
    UINT32 height;
    UINT32 border;  // the width of the border. must be either 0 or 1.
    UINT32 format;
    UINT32 type;

    // these two members are a bit repetitive but here for ease of parsing
    UINT32 pixelDataSize;    // number of bytes of pixel data following this struct
    UINT32 pixelDataOffset;  // byte offset from start of command (sizeof(GLTexImage2DCommand))
};

struct GLTexParameterCommand
{
    UINT32 target;
    UINT32 pname;
    float param;
};

// Lighting
struct GLEnableCommand
{
    UINT32 cap;
};

struct GLDisableCommand
{
    UINT32 cap;
};

struct GLLightCommand
{
    UINT32 light;
    UINT32 pname;
    float param;
};

struct GLLightfvCommand
{
    UINT32 light;
    UINT32 pname;
    GLVec4f params;
};

struct GLMaterialCommand
{
    UINT32 face;
    UINT32 pname;
    float param;
};

struct GLMaterialfvCommand
{
    UINT32 face;
    UINT32 pname;
    GLVec4f params;
};

// Buffer ops
struct GLClearCommand
{
    UINT32 mask;
};

struct GLClearColorCommand
{
    GLVec4f color;
};

using GLFlushCommand = GLEmptyCommand;
using GLFinishCommand = GLEmptyCommand;

// Viewport & projection
struct GLViewportCommand
{
    int32_t x, y, width, height;
};

struct GLOrthoCommand
{
    double left, right, bottom, top, zNear, zFar;
};

struct GLFrustumCommand
{
    double left, right, bottom, top, zNear, zFar;
};

struct GLPerspectiveCommand
{
    double fovY, aspect, zNear, zFar;
};

// Other
using GLShutdownCommand = GLEmptyCommand;

// Display Lists
struct GLCallListCommand
{
    UINT32 list;
};

struct GLNewListCommand
{
    UINT32 list;
    UINT32 mode;  // enum GL_COMPILE or GL_COMPILE_AND_EXECUTE
};

using GLEndListCommand = GLEmptyCommand;

}  // namespace glRemix
