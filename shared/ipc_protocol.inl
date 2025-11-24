template<typename GLCommand>
inline void write_command(glRemix::GLCommandType type, const GLCommand& command,
                          bool has_data = false, const void* data_ptr = nullptr,
                          const UINT32 data_bytes = 0)
{
    if (!m_curr_slot)
    {
        throw std::logic_error("IPCProtocol.WRITER - `m_curr_slot` is null at time of writing.");
    }
    if (has_data && (!data_ptr || data_bytes == 0))
    {
        throw std::logic_error("IPCProtocol.WRITER - Incorrect usage of `write_command`.");
    }

    const UINT32 command_bytes = sizeof(GLCommand);

    constexpr const SIZE_T unifs_bytes = sizeof(GLCommandUnifs);
    const SIZE_T total_bytes = command_bytes + data_bytes;  // accounts for if data_bytes > 0

    GLCommandUnifs unifs = { type, static_cast<UINT32>(total_bytes) };

    this->write_simple(&unifs, unifs_bytes);

    this->write_simple(&command, command_bytes);

    if (has_data)
    {
        this->write_simple(data_ptr, data_bytes);
    }
}
