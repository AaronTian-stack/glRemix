#pragma once
#include <ipc_protocol.h> // TODO: per-project prefixes
#include <gl_commands.h>
#include <vector>
#include <mutex>

namespace glRemix
{
    class FrameRecorder
    {
    public:
        inline bool Initialize()
        {
            return m_ipc.InitWriter();
        }

        inline void Record(const GLCommandType type, const void* payload, uint32_t size)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_recording)
            {
                CommandRecord rec;
                rec.type = type;
                rec.size = size;
                rec.data.assign(reinterpret_cast<const uint8_t*>(payload),
                                reinterpret_cast<const uint8_t*>(payload) + size);
                m_commands.push_back(std::move(rec));
            }
        }

        inline void StartFrame() { m_recording = true; }
        inline void EndFrame()
        {
            if (!m_recording) return;
            m_recording = false;

            std::vector<uint8_t> buffer;
            for (auto& cmd : m_commands)
            {
                GLCommand header { cmd.type, cmd.size };
                buffer.insert(buffer.end(),
                              reinterpret_cast<uint8_t*>(&header),
                              reinterpret_cast<uint8_t*>(&header) + sizeof(header));
                buffer.insert(buffer.end(), cmd.data.begin(), cmd.data.end());
            }

            if (!buffer.empty())
                m_ipc.SendFrame(buffer.data(), static_cast<uint32_t>(buffer.size()));

            m_commands.clear();
        }

    private:
        struct CommandRecord
        {
            GLCommandType type;
            uint32_t size;
            std::vector<uint8_t> data;
        };

        IPCProtocol m_ipc;
        std::mutex m_mutex;
        bool m_recording = false;
        std::vector<CommandRecord> m_commands;
    };
}
