template<typename GLCommand>
inline void write_command_base(glRemix::GLCommandType type, const GLCommand& command,
                               const SIZE_T command_bytes, bool has_data = false,
                               const void* data_ptr = nullptr, const SIZE_T data_bytes = 0)
{
    if (!m_curr_slot)
    {
        throw std::logic_error("IPCProtocol.WRITER - `m_curr_slot` is null at time of writing.");
    }
    if (has_data && (!data_ptr || data_bytes == 0))
    {
        throw std::logic_error("IPCProtocol.WRITER - Incorrect usage of `write_command`.");
    }

    constexpr const SIZE_T unifs_bytes = sizeof(GLCommandUnifs);
    const SIZE_T payload_bytes = command_bytes + data_bytes;

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
