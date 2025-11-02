#include "ipc_protocol.h"

bool glRemix::IPCProtocol::InitWriter(const wchar_t* name, uint32_t capacity)
{
    return this->m_smem.CreateForWriter(name, capacity);
}

bool glRemix::IPCProtocol::SendFrame(const void* data, uint32_t bytes)
{
    return this->m_smem.Write(data, bytes);
}

bool glRemix::IPCProtocol::InitReader(const wchar_t* name)
{
    return this->m_smem.OpenForReader(name);
}

bool glRemix::IPCProtocol::TryConsumeFrame(void* dst, uint32_t maxBytes, uint32_t* outBytes)
{
    return this->m_smem.Read(dst, maxBytes, 0, outBytes);
}
