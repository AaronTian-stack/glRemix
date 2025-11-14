#pragma once
#include <ipc_protocol.h>  // TODO: per-project prefixes
#include <gl_commands.h>
#include <vector>
#include <mutex>

namespace glRemix
{
class FrameRecorder
{
public:
    inline bool initialize()
    {
        return m_ipc.init_writer();
    }

    inline void record(const GLCommandType type, const void* payload, UINT32 size)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_recording)
        {
            GLCommandObject cmd;
            cmd.cmdUnifs = { type, size };
            cmd.data.assign(reinterpret_cast<const uint8_t*>(payload),
                            reinterpret_cast<const uint8_t*>(payload) + size);

            m_frameUnifs.payload_size += sizeof(cmd.cmdUnifs) + size;

            m_commands.push_back(std::move(cmd));
        }
    }

    inline void start_frame()
    {
        m_recording = true;
    }

    inline void end_frame()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_recording)
        {
            return;
        }

        m_recording = false;

        std::vector<uint8_t> buffer;
        buffer.reserve(sizeof(m_frameUnifs) + m_frameUnifs.payload_size);

        buffer.insert(buffer.end(), reinterpret_cast<uint8_t*>(&m_frameUnifs),
                      reinterpret_cast<uint8_t*>(&m_frameUnifs) + sizeof(m_frameUnifs));

        if (!m_commands.empty())
        {
            for (auto& cmd : m_commands)
            {
                buffer.insert(buffer.end(), reinterpret_cast<uint8_t*>(&cmd.cmdUnifs),
                              reinterpret_cast<uint8_t*>(&cmd.cmdUnifs) + sizeof(cmd.cmdUnifs));
                buffer.insert(buffer.end(), cmd.data.begin(), cmd.data.end());
            }
        }

        m_ipc.send_frame(buffer.data(), static_cast<UINT32>(buffer.size()));

        m_commands.clear();
        m_frameUnifs.payload_size = 0;
        m_frameUnifs.frame_index++;
    }

private:
    struct GLCommandObject
    {
        GLCommandUnifs cmdUnifs;
        std::vector<uint8_t> data;
    };

    IPCProtocol m_ipc;

    std::mutex m_mutex;
    bool m_recording = false;

    GLFrameUnifs m_frameUnifs;
    std::vector<GLCommandObject> m_commands;
};
}  // namespace glRemix
