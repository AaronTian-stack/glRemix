#include "frame_recorder.h"

namespace glRemix
{
// Global instance
FrameRecorder g_recorder;

bool FrameRecorder::initialize()
{
    return m_ipc.init_writer();
}

void FrameRecorder::record(const GLCommandType type, const void* payload, const UINT32 size)
{
    std::lock_guard lock(m_mutex);
    if (m_recording)
    {
        GLCommandObject cmd;
        cmd.cmd_unifs.type = type;
        cmd.cmd_unifs.dataSize = size;
        cmd.data.resize(size);
        std::memcpy(cmd.data.data(), payload, size);

        m_frame_unifs.payload_size += sizeof(cmd.cmd_unifs) + size;

        m_commands.push_back(std::move(cmd));
    }
}

void FrameRecorder::start_frame()
{
    m_recording = true;
}

void FrameRecorder::end_frame()
{
    std::lock_guard lock(m_mutex);
    if (!m_recording)
    {
        return;
    }

    m_recording = false;

    m_buffer.clear();
    m_buffer.reserve(sizeof(m_frame_unifs) + m_frame_unifs.payload_size);

    m_buffer.insert(m_buffer.end(), reinterpret_cast<UINT8*>(&m_frame_unifs),
                    reinterpret_cast<UINT8*>(&m_frame_unifs) + sizeof(m_frame_unifs));

    if (!m_commands.empty())
    {
        for (auto& cmd : m_commands)
        {
            m_buffer.insert(m_buffer.end(), reinterpret_cast<UINT8*>(&cmd.cmd_unifs),
                            reinterpret_cast<UINT8*>(&cmd.cmd_unifs) + sizeof(cmd.cmd_unifs));
            m_buffer.insert(m_buffer.end(), cmd.data.begin(), cmd.data.end());
        }
    }

    m_ipc.send_frame(m_buffer.data(), static_cast<UINT32>(m_buffer.size()));

    m_commands.clear();
    m_frame_unifs.payload_size = 0;
    m_frame_unifs.frame_index++;
}

UINT8* FrameRecorder::get_scratch_buffer(size_t requiredSize)
{
    static thread_local std::vector<UINT8> scratch;
    if (scratch.size() < requiredSize)
    {
        scratch.resize(requiredSize);
    }
    return scratch.data();
}

}  // namespace glRemix
