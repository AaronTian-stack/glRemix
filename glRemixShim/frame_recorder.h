#pragma once

#include <ipc_protocol.h>  // TODO: per-project prefixes
#include <gl_commands.h>
#include <vector>
#include <mutex>

namespace glRemix
{
class FrameRecorder
{
    struct GLCommandObject
    {
        GLCommandUnifs cmd_unifs;
        std::vector<UINT8> data;
    };

    IPCProtocol m_ipc;

    std::mutex m_mutex;
    bool m_recording = false;

    GLFrameUnifs m_frame_unifs;
    std::vector<GLCommandObject> m_commands;

public:
    bool initialize()
    {
        return m_ipc.init_writer();
    }

    void record(const GLCommandType type, const void* payload, const UINT32 size)
    {
        std::lock_guard lock(m_mutex);
        if (m_recording)
        {
            GLCommandObject cmd;
            cmd.cmd_unifs = { .type = type, .dataSize = size };
            cmd.data.assign(static_cast<const UINT8*>(payload),
                            static_cast<const UINT8*>(payload) + size);

            m_frame_unifs.payload_size += sizeof(cmd.cmd_unifs) + size;

            m_commands.push_back(std::move(cmd));
        }
    }

    void start_frame()
    {
        m_recording = true;
    }

    void end_frame()
    {
        std::lock_guard lock(m_mutex);
        if (!m_recording)
        {
            return;
        }

        m_recording = false;

        std::vector<UINT8> buffer;
        buffer.reserve(sizeof(m_frame_unifs) + m_frame_unifs.payload_size);

        buffer.insert(buffer.end(), reinterpret_cast<UINT8*>(&m_frame_unifs),
                      reinterpret_cast<UINT8*>(&m_frame_unifs) + sizeof(m_frame_unifs));

        if (!m_commands.empty())
        {
            for (auto& cmd : m_commands)
            {
                buffer.insert(buffer.end(), reinterpret_cast<UINT8*>(&cmd.cmd_unifs),
                              reinterpret_cast<UINT8*>(&cmd.cmd_unifs) + sizeof(cmd.cmd_unifs));
                buffer.insert(buffer.end(), cmd.data.begin(), cmd.data.end());
            }
        }

        m_ipc.send_frame(buffer.data(), static_cast<UINT32>(buffer.size()));

        m_commands.clear();
        m_frame_unifs.payload_size = 0;
        m_frame_unifs.frame_index++;
    }


};
}  // namespace glRemix
