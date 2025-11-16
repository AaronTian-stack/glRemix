#include "ipc_protocol.h"

bool glRemix::IPCProtocol::init_writer(const wchar_t* name, const UINT32 capacity)
{
    return this->m_smem.create_for_writer(name, capacity);
}

bool glRemix::IPCProtocol::send_frame(const void* data, const UINT32 bytes) const
{
    return this->m_smem.write(data, bytes);
}

bool glRemix::IPCProtocol::init_reader(const wchar_t* name)
{
    return this->m_smem.open_for_reader(name);
}

bool glRemix::IPCProtocol::try_consume_frame(void* dst, const UINT32 max_bytes, UINT32* out_bytes)
{
    return this->m_smem.read(dst, max_bytes, 0, out_bytes);
}
