#include "ipc_protocol.h"

#include <sstream>
#include <algorithm>
#include <thread>
#include <chrono>

#include <format>

#define FSTR(fmt, ...) std::format(fmt, __VA_ARGS__)

void glRemix::IPCProtocol::init_writer(const UINT32 capacity)
{
    if (!m_slot_A.smem.create_for_writer(k_MAP_A, k_WRITE_EVENT_A, k_READ_EVENT_A, capacity))
    {
        throw std::runtime_error("IPCProtocol.WRITER - Failed to create SharedMemory A");
    }
    if (!m_slot_B.smem.create_for_writer(k_MAP_B, k_WRITE_EVENT_B, k_READ_EVENT_B, capacity))
    {
        throw std::runtime_error("IPCProtocol.WRITER - Failed to create SharedMemory B");
    }
}

void glRemix::IPCProtocol::send_frame(const void* data, const UINT32 bytes)
{
    MemorySlot* oldest;  // shared memory that has the smaller recorded time frame
    MemorySlot* latest;  // shared memory that has the larger recorded time frame
    if (m_slot_A < m_slot_B)
    {
        oldest = &m_slot_A;
        latest = &m_slot_B;
    }
    else
    {
        oldest = &m_slot_B;
        latest = &m_slot_A;
    }

    HANDLE tmp_events[2] = { oldest->smem.get_read_event(), latest->smem.get_read_event() };

    DWORD dw_wait_result = WaitForMultipleObjects(2, tmp_events, false, INFINITE);

    MemorySlot* current;

    switch (dw_wait_result)
    {
        case WAIT_OBJECT_0:
            current = oldest;
            DBG_PRINT("IPCProtocol.WRITER - Oldest SharedMemory slot became available to write "
                      "before latest.");  // theoretically should not occur
            break;
        case WAIT_OBJECT_0 + 1: current = latest; break;
        default:
            throw std::runtime_error(FSTR(
                "IPCProtocol.WRITER - WaitForMultipleObjects for writer failed. Error Code: {}",
                dw_wait_result));
    }

    current->smem.write(data, bytes);

    g_frame_index++;
    current->frame_index = g_frame_index;
}

void glRemix::IPCProtocol::init_reader()
{
    constexpr UINT16 MAX_WAIT_MS = 5000;
    constexpr UINT16 RETRY_MS = 50;

    UINT16 elapsed = 0;
    while (elapsed < MAX_WAIT_MS)
    {
        bool result_A = m_slot_A.smem.open_for_reader(k_MAP_A, k_WRITE_EVENT_A, k_READ_EVENT_A);
        bool result_B = m_slot_B.smem.open_for_reader(k_MAP_B, k_WRITE_EVENT_B, k_READ_EVENT_B);

        if (result_A && result_B)
        {
            return;  // success
        }
        else
        {
            if (!result_A)
            {
                DBG_PRINT(
                    "IPCProtocol.READER - Failed to open SharedMemory A after %u milliseconds.",
                    elapsed);
            }
            if (!result_B)
            {
                DBG_PRINT(
                    "IPCProtocol.READER - Failed to open SharedMemory B after %u milliseconds.",
                    elapsed);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_MS));

        elapsed += RETRY_MS;
    }
    throw std::runtime_error("IPCProtocol.READER - Timed out waiting for writer initialization.");
}

void glRemix::IPCProtocol::consume_frame(void* dst, const UINT32 max_bytes, UINT32* out_bytes)
{
    MemorySlot* oldest;  // shared memory that has the smaller recorded time frame
    MemorySlot* latest;  // shared memory that has the larger recorded time frame
    if (m_slot_A < m_slot_B)
    {
        oldest = &m_slot_A;
        latest = &m_slot_B;
    }
    else
    {
        oldest = &m_slot_B;
        latest = &m_slot_A;
    }

    HANDLE tmp_events[2] = { oldest->smem.get_write_event(), latest->smem.get_write_event() };

    DWORD dw_wait_result = WaitForMultipleObjects(2, tmp_events, false, INFINITE);

    MemorySlot* current;

    switch (dw_wait_result)
    {
        case WAIT_OBJECT_0: current = oldest; break;
        case WAIT_OBJECT_0 + 1:
            current = latest;
            DBG_PRINT("IPCProtocol.READER - Latest SharedMemory slot became available to read "
                      "before oldest.");
            break;  // theoretically should not occur
        default:
            throw std::runtime_error(FSTR(
                "IPCProtocol.READER - WaitForMultipleObjects for reader failed. Error Code: {}",
                dw_wait_result));
    }

    current->smem.read(dst, max_bytes, 0, out_bytes);
}
