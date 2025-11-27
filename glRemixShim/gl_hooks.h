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

constexpr UINT32 NUM_CLIENT_ARRAYS = static_cast<UINT32>(GLRemixClientArrayType::_COUNT);

void install_overrides();
}  // namespace glRemix::hooks
