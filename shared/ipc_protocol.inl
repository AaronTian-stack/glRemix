/**
 * @brief Helper function to write a `GLCommand` to IPC.
 * The last 3 arguments are relevant for GL functions that accept a pointer,
 * notably `glTexImage2D`, `glDrawArrays`, `glDrawElements`.
 * Furthermore, if there are multiple "data pointers" associated with the command,
 * such as in `glDrawArrays`, `has_extra_data` and `p_extra_data` may stay
 * `false` and `nullptr` respectively, and those pointers may be written one-by-one
 * using calls to `IPCProtocol::write_simple`.
 * See `gl_draw_arrays_ovr` in `glRemixShim/gl_hooks.cpp` for such an use case.
 *
 * @tparam GLCommand
 * @param type
 * @param command
 * @param extra_data_bytes: Can be > 0 even if `has_extra_data` is `false`.
 * i.e. should ALWAYS be set to the total desired byte size of extra data associated with this
 * command. This effectively lets us avoid temporary data allocations while still passing the
 * correct `total_bytes` to `GLCommandUnifs`.
 * @param has_extra_data: whether contents of `p_extra_data == nullptr`.
 * @param p_extra_data: pointer to extra data
 */
template<typename GLCommand>
inline void write_command(glRemix::GLCommandType type, const GLCommand& command,
                          const UINT32 extra_data_bytes = 0, bool has_extra_data = false,
                          const void* p_extra_data = nullptr)
{
    if (!m_curr_slot)
    {
        throw std::logic_error("IPCProtocol.WRITER - `m_curr_slot` is null at time of writing.");
    }
    if (has_extra_data && (!p_extra_data || extra_data_bytes == 0))
    {
        throw std::logic_error("IPCProtocol.WRITER - Incorrect usage of `write_command`.");
    }

    const UINT32 command_bytes = sizeof(GLCommand);

    constexpr const SIZE_T unifs_bytes = sizeof(GLCommandUnifs);

    // always write `total_bytes`
    const SIZE_T total_bytes = command_bytes + extra_data_bytes;

    GLCommandUnifs unifs = { type, static_cast<UINT32>(total_bytes) };

    this->write_simple(&unifs, unifs_bytes);

    this->write_simple(&command, command_bytes);

    if (has_extra_data)
    {
        this->write_simple(p_extra_data, extra_data_bytes);
    }
}
