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
    // Core Immediate Mode
    GLCMD_BEGIN = 1,
    GLCMD_END,
    GLCMD_VERTEX2F,
    GLCMD_VERTEX3F,
    GLCMD_COLOR3F,
    GLCMD_COLOR4F,
    GLCMD_NORMAL3F,
    GLCMD_TEXCOORD2F,

    // Display Lists
    GLCMD_CALL_LIST,
    GLCMD_NEW_LIST,
    GLCMD_END_LIST,

    // Client State
    GLCMD_ENABLE_CLIENT_STATE,
    GLCMD_DISABLE_CLIENT_STATE,
    GLCMD_VERTEX_POINTER,
    GLCMD_NORMAL_POINTER,
    GLCMD_TEXCOORD_POINTER,
    GLCMD_COLOR_POINTER,
    GLCMD_DRAW_ARRAYS,    // within fixed-function scope
    GLCMD_DRAW_ELEMENTS,  // within fixed-function scope

    // Matrix Operations
    GLCMD_MATRIX_MODE,
    GLCMD_LOAD_IDENTITY,
    GLCMD_LOAD_MATRIX,
    GLCMD_MULT_MATRIX,
    GLCMD_PUSH_MATRIX,
    GLCMD_POP_MATRIX,
    GLCMD_TRANSLATE,
    GLCMD_ROTATE,
    GLCMD_SCALE,
    GLCMD_VIEWPORT,
    GLCMD_ORTHO,
    GLCMD_FRUSTUM,
    GLCMD_PERSPECTIVE,

    // Rendering
    GLCMD_CLEAR,
    GLCMD_CLEAR_COLOR,
    GLCMD_FLUSH,
    GLCMD_FINISH,
    GLCMD_BIND_TEXTURE,
    GLCMD_GEN_TEXTURES,
    GLCMD_DELETE_TEXTURES,
    GLCMD_TEX_IMAGE_2D,
    GLCMD_TEX_PARAMETER,
    GLCMD_TEX_ENV_I,
    GLCMD_TEX_ENV_F,

    // Fixed Function
    GLCMD_LIGHTF,
    GLCMD_LIGHTFV,
    GLCMD_MATERIALI,
    GLCMD_MATERIALF,
    GLCMD_MATERIALIV,
    GLCMD_MATERIALFV,
    GLCMD_ALPHA_FUNC,

    // State Management
    GLCMD_ENABLE,
    GLCMD_DISABLE,
    GLCMD_COLOR_MASK,
    GLCMD_DEPTH_MASK,
    GLCMD_BLEND_FUNC,
    GLCMD_POINT_SIZE,
    GLCMD_POLYGON_OFFSET,
    GLCMD_CULL_FACE,
    GLCMD_STENCIL_MASK,
    GLCMD_STENCIL_FUNC,
    GLCMD_STENCIL_OP,
    GLCMD_STENCIL_OP_SEPARATE_ATI,

    // Other
    WGLCMD_CREATE_CONTEXT,  // wglCreateContext needs IPC

    // Input Events
    WGLCMD_INPUT_EVENT,
};

// Component Structs
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

// Named *Unifs for clear association
// Header for all commands
struct GLCommandUnifs
{
    GLCommandType type;
    UINT32 dataSize;
};

/* CORE IMMEDIATE MODE */
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

/* DISPLAY LISTS */
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

/* CLIENT STATE */
struct GLEnableClientStateCommand
{
    UINT32 array;
};

struct GLDisableClientStateCommand
{
    UINT32 array;
};

struct GLVertexPointerCommand
{
    UINT32 size;
    UINT32 type;
    UINT32 stride;
};

struct GLNormalPointerCommand
{
    UINT32 type;
    UINT32 stride;
};

struct GLTexCoordPointerCommand
{
    UINT32 size;
    UINT32 type;
    UINT32 stride;
};

struct GLColorPointerCommand
{
    UINT32 size;
    UINT32 type;
    UINT32 stride;
};

struct GLDrawArraysCommand
{
    UINT32 mode;
    UINT32 first;
    UINT32 count;
};

struct GLDrawElementsCommand
{
    UINT32 mode;
    UINT32 count;
    UINT32 type;
};

/* MATRIX OPERATIONS */
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

/* RENDERING */
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
};

struct GLTexParameterCommand
{
    UINT32 target;
    UINT32 pname;
    float param;
};

struct GLTexEnviCommand
{
    UINT32 target;
    UINT32 pname;
    UINT32 param;
};

struct GLTexEnvfCommand
{
    UINT32 target;
    UINT32 pname;
    float param;
};

/* FIXED FUNCTION */
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

struct GLMaterialiCommand
{
    UINT32 face;
    UINT32 pname;
    int param;
};

struct GLMaterialfCommand
{
    UINT32 face;
    UINT32 pname;
    float param;
};

struct GLMaterialivCommand
{
    UINT32 face;
    UINT32 pname;
    GLVec4f params;
};

struct GLMaterialfvCommand
{
    UINT32 face;
    UINT32 pname;
    GLVec4f params;
};

struct GLAlphaFuncCommand
{
    UINT32 func;
    float ref;
};

/* STATE MANAGEMENT */
struct GLEnableCommand
{
    UINT32 cap;
};

struct GLDisableCommand
{
    UINT32 cap;
};

struct GLColorMaskCommand
{
    UINT8 r, g, b, a;
};

struct GLDepthMaskCommand
{
    UINT8 flag;
};

struct GLBlendFuncCommand
{
    UINT32 sfactor;
    UINT32 dfactor;
};

struct GLPointSizeCommand
{
    float size;
};

struct GLPolygonOffsetCommand
{
    float factor;
    float units;
};

struct GLCullFaceCommand
{
    UINT32 mode;
};

struct GLStencilMaskCommand
{
    UINT32 mask;
};

struct GLStencilFuncCommand
{
    UINT32 func;
    INT32 ref;
    UINT32 mask;
};

struct GLStencilOpCommand
{
    UINT32 sfail;
    UINT32 dpfail;
    UINT32 dppass;
};

struct GLStencilOpSeparateATICommand
{
    UINT32 face;
    UINT32 sfail;
    UINT32 dpfail;
    UINT32 dppass;
};

struct WGLCreateContextCommand
{
    HWND hwnd;  // NOTE: 32-bit in x86 shim, 64-bit in x64 shim
};

struct WGLInputEventCommand
{
    UINT32 msg;
    UINT64 wparam;
    UINT64 lparam;
};
}  // namespace glRemix
