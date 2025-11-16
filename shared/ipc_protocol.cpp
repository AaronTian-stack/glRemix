#include "ipc_protocol.h"

#include <sstream>

bool glRemix::IPCProtocol::init_writer(const wchar_t* name, const UINT32 capacity)
{
    return this->m_smem_A.create_for_writer(name, capacity);
}

/*
 * Using `WaitForSingleObject`, ::send_frame effectively forces all writers to stall
 * until they are able to write.
 */
bool glRemix::IPCProtocol::send_frame(const void* data, const UINT32 bytes) const
{
    SharedMemoryHeader* header = m_smem_A.get_header();
    if (!header || !data)
    {
        DBG_PRINT("IPCProtocol - Invalid state.\n");
        return false;
    }

    DWORD dw_wait_result;

    while (header->state == SharedState::FILLED)
    {
        HANDLE get_read_event = m_smem_A.get_read_event();
        if (!get_read_event)
        {
            DBG_PRINT("IPCProtocol - `read_event` handle is NULL.\n");
            return false;
        }

        dw_wait_result = WaitForSingleObject(get_read_event, INFINITE);

        if (dw_wait_result != WAIT_OBJECT_0)
        {
            DBG_PRINT("IPCProtocol - WaitForSingleObject get_read_event failed. Error Code: %u\n",
                      dw_wait_result);
            return false;
        }
    }
    return this->m_smem_A.write(data, bytes);
}

bool glRemix::IPCProtocol::init_reader(const wchar_t* name)
{
    return this->m_smem_A.open_for_reader(name);
}

bool glRemix::IPCProtocol::consume_frame(void* dst, const UINT32 max_bytes, UINT32* out_bytes)
{
    return this->m_smem_A.read(dst, max_bytes, 0, out_bytes);
}

glRemix::SharedMemory* glRemix::IPCProtocol::wait_for_write_buffer()
{
    return nullptr;
}

glRemix::SharedMemory* glRemix::IPCProtocol::wait_for_read_buffer()
{
    return nullptr;
}
