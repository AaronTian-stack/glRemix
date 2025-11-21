#pragma once

#include <windows.h>

#include "gl_commands.h"
#include "shared_memory.h"

#include <stdexcept>

namespace glRemix
{
// maintains per-frame uniforms for OpenGL commands sent via IPC
struct GLFrameUnifs
{
    UINT32 payload_size = 0;  // bytes following this header
    UINT32 frame_index = 0;   // incremental frame counter
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
    void init_writer();
    void start_frame_or_wait();  // uses `WaitForMultipleObjects` to stall thread here

    template<typename GLCommand>
    inline void write_command(GLCommandType type, const GLCommand& command, bool has_data = false,
                              const void* data_ptr = nullptr, const UINT32 data_bytes = 0)
    {
        const uint32_t command_bytes = sizeof(GLCommand);

        this->write_command_base(type, command, command_bytes, has_data, data_ptr, data_bytes);
    }

    /*
     * Writes `GLFrameUnifs` at 0 offset of IPC file-mapped object.
     * Signals write event when complete
     */
    void end_frame();

    // for renderer
    void init_reader();
    // uses `WaitForMultipleObjects` to stall thread here. signals read event when complete.
    void consume_frame_or_wait(void* payload, UINT32* payload_size, UINT32* frame_index);

    inline UINT32 get_max_payload_size()
    {
        const UINT32 cap_A = m_slot_A.smem.get_capacity();
        const UINT32 cap_B = m_slot_B.smem.get_capacity();
        return (cap_A > cap_B ? cap_A : cap_B) - sizeof(GLFrameUnifs);
    }

private:
    struct MemorySlot
    {
        SharedMemory smem;
        UINT32 frame_index = 0;  // each smem keeps track of their own frame index for now

        inline bool operator<(const MemorySlot& other) const
        {
            return frame_index < other.frame_index;
        }
    };

    // double buffers
    MemorySlot m_slot_A;
    MemorySlot m_slot_B;

    UINT32 g_frame_index = 0;

    MemorySlot* m_curr_slot = nullptr;

    UINT32 m_offset = 0;

    template<typename GLCommand>
    inline void write_command_base(GLCommandType type, const GLCommand& command,
                                   const SIZE_T command_bytes, bool has_data = false,
                                   const void* data_ptr = nullptr, const SIZE_T data_bytes = 0)
    {
        if (!m_curr_slot)
        {
            throw std::logic_error(
                "IPCProtocol.WRITER - `m_curr_slot` is null at time of writing.");
        }
        if (has_data && (!data_ptr || data_bytes == 0))
        {
            throw std::logic_error("IPCProtocol.WRITER - Incorrect usage of `write_command`.");
        }

        constexpr const SIZE_T unifs_bytes = sizeof(GLCommandUnifs);
        const SIZE_T payload_bytes = command_bytes + data_bytes;
        // const SIZE_T total_bytes = unifs_bytes + payload_bytes;

        GLCommandUnifs unifs = { type, static_cast<UINT32>(payload_bytes) };

        m_curr_slot->smem.write(&unifs, m_offset, unifs_bytes);
        m_offset += unifs_bytes;

        m_curr_slot->smem.write(&command, m_offset, command_bytes);
        m_offset += command_bytes;

        if (has_data)
        {
            m_curr_slot->smem.write(data_ptr, m_offset, data_bytes);
            m_offset += data_bytes;
        }
    }
};
}  // namespace glRemix
