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

// Local keyword allows to stay per-session, Global requires elevated permissions
// wchar_t is standard for file mapping names in windows (though can maybe switch with TCHAR*)
constexpr const wchar_t* k_MAP_A = L"Local\\glRemix_Map_A";
constexpr const wchar_t* k_WRITE_EVENT_A = L"Local\\glRemix_WriteEvent_A";
constexpr const wchar_t* k_READ_EVENT_A = L"Local\\glRemix_ReadEvent_A";

constexpr const wchar_t* k_MAP_B = L"Local\\glRemix_Map_B";
constexpr const wchar_t* k_WRITE_EVENT_B = L"Local\\glRemix_WriteEvent_B";
constexpr const wchar_t* k_READ_EVENT_B = L"Local\\glRemix_ReadEvent_B";

// For now, a simple manager for `SharedMemory
class IPCProtocol
{
public:
    // for shim
    void init_writer(UINT32 capacity = k_DEFAULT_CAPACITY);
    void send_frame(const void* data, UINT32 bytes);

    // for renderer
    void init_reader();
    void consume_frame(void* dst, UINT32 max_bytes, UINT32* out_bytes = nullptr);

    UINT32 get_capacity() const
    {
        return m_slot_A.smem.get_capacity();
    }

private:
    struct MemorySlot
    {
        SharedMemory smem;
        UINT32 frame_index = 0;  // each smem keeps track of their own frame index

        bool operator<(const MemorySlot& other) const
        {
            return frame_index < other.frame_index;
        }
    };

    // double buffers
    MemorySlot m_slot_A;
    MemorySlot m_slot_B;

    UINT32 g_frame_index = 0;
};
}  // namespace glRemix
