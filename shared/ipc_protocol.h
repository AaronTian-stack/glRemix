#pragma once

#include <windows.h>
#include "gl_commands.h"
#include "shared_memory.h"

namespace glRemix
{
// maintains per-frame uniforms for OpenGL commands sent via IPC
struct GLFrameUnifs
{
    uint32_t frameIndex = 0;   // incremental frame counter
    uint32_t payloadSize = 0;  // bytes following this header
};

/* For now, a simple manager for `SharedMemory` */
class IPCProtocol
{
public:
    /* for shim */
    bool InitWriter(const wchar_t* name = kDEFAULT_MAP_NAME, uint32_t capacity = kDEFAULT_CAPACITY);
    bool SendFrame(const void* data, uint32_t bytes);  // TODO: offset

    /* for renderer */
    bool InitReader(const wchar_t* name = kDEFAULT_MAP_NAME);  // use same map name or else...
    bool TryConsumeFrame(void* dst, uint32_t maxBytes, uint32_t* outBytes = nullptr);

    uint32_t GetCapacity() const
    {
        return m_smem.GetCapacity();
    }

private:
    SharedMemory m_smem;  // smem!
};
}  // namespace glRemix
