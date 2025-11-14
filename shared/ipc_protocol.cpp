#include "ipc_protocol.h"

bool glRemix::IPCProtocol::init_writer(const wchar_t* name, UINT32 capacity)
{
    return this->m_smem.create_for_writer(name, capacity);
}

bool glRemix::IPCProtocol::send_frame(const void* data, UINT32 bytes)
{
    return this->m_smem.write(data, bytes);
}

bool glRemix::IPCProtocol::init_reader(const wchar_t* name)
{
    return this->m_smem.open_for_reader(name);
}

bool glRemix::IPCProtocol::try_consume_frame(void* dst, UINT32 maxBytes, UINT32* outBytes)
{
    return this->m_smem.read(dst, maxBytes, 0, outBytes);
}
