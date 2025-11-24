#pragma once

#include <shared/ipc_protocol.h>

#include "framework.h"

namespace glRemix
{
extern glRemix::IPCProtocol g_ipc;

constexpr UINT32 NUMCLIENTARRAYS = static_cast<UINT32>(GLRemixClientArrayKind::_COUNT);

struct GLRemixClientArrayInterface
{
    UINT32 size = 0;
    UINT32 type = 0;
    UINT32 stride = 0;
    const void* ptr = nullptr;
    bool enabled = false;
};

extern std::array<GLRemixClientArrayInterface, NUMCLIENTARRAYS> g_client_arrays;
extern UINT32 g_client_count;

namespace gl
{
extern HANDLE g_renderer_process;

void initialize();

// Add function pointer to hooks map using name as key
void register_hook(const char* name, PROC proc);

// Try to return hooked function pointer using name as hook map lookup
PROC find_hook(const char* name);

// Print out missing function name to debug output
void report_missing_function(const char* name);
}  // namespace gl
}  // namespace glRemix
