#pragma once

#include "framework.h"

#include "framerecorder.h"

namespace glRemix
{
extern FrameRecorder g_recorder;

namespace gl
{
extern std::atomic<uint32_t> g_list_id_counter;

void initialize();

// Add function pointer to hooks map using name as key
void register_hook(const char* name, PROC proc);

// Try to return hooked function pointer using name as hook map lookup
PROC find_hook(const char* name);

// Print out missing function name to debug output
void report_missing_function(const char* name);
}  // namespace gl
}  // namespace glRemix
