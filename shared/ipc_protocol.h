#pragma once

#include <windows.h>
#include "shared_memory.h"

namespace glRemix
{
// maintains per-frame uniforms for OpenGL commands sent via IPC
struct GLFrameUnifs
{
    UINT32 frame_index = 0;   // incremental frame counter
    UINT32 payload_size = 0;  // bytes following this header
};

// For now, a simple manager for `SharedMemory
class IPCProtocol
{
public:
    // for shim
    bool init_writer(const wchar_t* name = k_DEFAULT_MAP_NAME, UINT32 capacity = k_DEFAULT_CAPACITY);
    bool send_frame(const void* data, UINT32 bytes) const;  // TODO: offset

    // for renderer
    bool init_reader(const wchar_t* name = k_DEFAULT_MAP_NAME);  // use same map name or else...
    bool consume_frame(void* dst, UINT32 max_bytes, UINT32* out_bytes = nullptr);

    UINT32 get_capacity() const
    {
        return m_smem.get_capacity();
    }

private:
    SharedMemory m_smem;  // smem!
};
}  // namespace glRemix
