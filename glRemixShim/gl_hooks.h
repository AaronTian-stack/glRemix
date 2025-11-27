#pragma once

#include <mutex>
#include <tsl/robin_map.h>

#include <shared/ipc_protocol.h>
#include <shared/gl_commands.h>

#include <framework.h>

namespace glRemix::hooks
{
struct FakePixelFormat
{
    PIXELFORMATDESCRIPTOR descriptor{};
    int id = 1;
};

struct FakeContext
{
    HDC last_dc = nullptr;
};

struct GLRemixClientArrayInterface
{
    GLRemixClientArrayHeader ipc_payload;
    UINT32 stride = 0;
    bool enabled = false;
    const void* ptr = nullptr;
};

static UINT32 g_gen_lists_counter = 0;     // monotonic int, passed back to host app in `glGenLists`
static UINT32 g_gen_textures_counter = 0;  // monotonic int, passed back in `glGenTextures`

constexpr UINT32 NUM_CLIENT_ARRAYS = static_cast<UINT32>(GLRemixClientArrayType::_COUNT);

extern thread_local std::array<GLRemixClientArrayInterface, NUM_CLIENT_ARRAYS> g_client_arrays;
static UINT32 g_enabled_client_arrays_counter = 0;  // count of currently enabled client arrays

// WGL/OpenGL might be called from multiple threads
extern std::mutex g_mutex;

// wglSetPixelFormat will only be called once per context
// Or if there are multiple contexts they can share the same format since they're fake anyway...
extern tsl::robin_map<HDC, FakePixelFormat> g_pixel_formats;

extern thread_local HGLRC g_current_context;
extern thread_local HDC g_current_dc;

void install_overrides();
}  // namespace glRemix::hooks
