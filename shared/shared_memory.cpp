#include "shared_memory.h"

#include <sstream>

glRemix::SharedMemory::~SharedMemory()
{
    close_all();
}

// `CreateFileMapping`
bool glRemix::SharedMemory::create_for_writer(const wchar_t* name, const UINT32 capacity)
{
    close_all();

    const auto max_obj_size = static_cast<DWORD>(max_object_size(capacity));
    const auto h_map_file = CreateFileMappingW(INVALID_HANDLE_VALUE,  // use paging file
                                         nullptr,                  // default security
                                         PAGE_READWRITE,        // rw access
                                         0,            // maximum object size (high-order DWORD)
                                         max_obj_size,  // maximum object size (low-order DWORD)
                                         name);        // name of mapping object

    if (!h_map_file)
    {
        std::ostringstream ss;
        ss << "Could not create file mapping of file. Error Code: " << GetLastError() << "\n";
        OutputDebugStringA(ss.str().c_str());
        return false;
    }

    if (!map_common(h_map_file))
    {
        // _map_common does its own error handling
        CloseHandle(h_map_file);
        return false;
    }

    // init header
    m_header->capacity = capacity;
    m_header->size = 0;

    // InterlockedExchange to safely reset state to EMPTY now
    InterlockedExchange(reinterpret_cast<volatile LONG*>(&m_header->state),
                        static_cast<LONG>(SharedState::EMPTY));

    // create named events
    m_writeEvent = CreateEventW(nullptr, FALSE, FALSE, L"Local\\glRemix_WriteEvent");
    m_readEvent = CreateEventW(nullptr, FALSE, FALSE, L"Local\\glRemix_ReadEvent");

    return true;
}

bool glRemix::SharedMemory::open_for_reader(const wchar_t* name)
{
    close_all();

    HANDLE hMapFile = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, name);
    if (!hMapFile)
    {
        std::ostringstream ss;
        ss << "Could not open file mapping of file. Ensure external process has created it. ";
        ss << "Error Code: " << GetLastError() << "\n";
        OutputDebugStringA(ss.str().c_str());
        return false;
    }

    if (!map_common(hMapFile))
    {
        CloseHandle(hMapFile);
        return false;
    }

    // Try to open events (safe if they don't exist yet)
    m_writeEvent = OpenEventW(EVENT_ALL_ACCESS, FALSE, L"Local\\glRemix_WriteEvent");
    m_readEvent = OpenEventW(EVENT_ALL_ACCESS, FALSE, L"Local\\glRemix_ReadEvent");

    return true;
}

bool glRemix::SharedMemory::write(const void* src, const UINT32 bytes, const UINT32 offset) const
{
    std::ostringstream ss;
    if (!m_header || !src || bytes > m_header->capacity)
    {
        ss << "File not ready for writing, call `::create_for_writer` first." << "\n";
        OutputDebugStringA(ss.str().c_str());
        return false;
    }

    if (m_header->state == SharedState::CONSUMED)
    {
        InterlockedExchange(reinterpret_cast<volatile LONG*>(&m_header->state),
                            static_cast<LONG>(SharedState::EMPTY));
    }

    // only write when EMPTY
    if (m_header->state != SharedState::EMPTY)
    {
        ss << "File state not consumed. Will skip writing." << "\n";
        OutputDebugStringA(ss.str().c_str());
        return false;
    }

    memcpy(m_payload + offset, src, bytes);
    ss << "Wrote " << bytes << " bytes." << "\n";
    OutputDebugStringA(ss.str().c_str());
    m_header->size = bytes;

    // mark as FILLED so the reader knows data is ready
    MemoryBarrier();  // ensure size/payload visible before state
    InterlockedExchange(reinterpret_cast<volatile LONG*>(&m_header->state),
                        static_cast<LONG>(SharedState::FILLED));

    // signal write event
    if (m_writeEvent)
    {
        SetEvent(m_writeEvent);
    }

    return true;
}

bool glRemix::SharedMemory::read(void* dst, const UINT32 max_bytes, const UINT32 offset, UINT32* out_bytes)
{
    std::ostringstream ss;
    if (!m_header || !dst)
    {
        ss << "File not ready for reading, call `::open_for_reader` first." << "\n";
        OutputDebugStringA(ss.str().c_str());

        return false;
    }

    if (m_header->state != SharedState::FILLED)
    {
        ss << "File state not filled. Nothing to read." << "\n";
        OutputDebugStringA(ss.str().c_str());
        return false;
    }

    UINT32 n = m_header->size;
    if (n > max_bytes)
    {
        ss << "Header size greater than desired max bytes. Truncating..." << "\n";
        OutputDebugStringA(ss.str().c_str());

        n = max_bytes;
    }

    memcpy(dst, m_payload + offset, n);  // copy into payload

    if (out_bytes)
    {
        *out_bytes = n;
    }

    // mark as CONSUMED
    InterlockedExchange(reinterpret_cast<volatile LONG*>(&m_header->state),
                        static_cast<LONG>(SharedState::CONSUMED));

    // signal read event
    if (m_readEvent)
    {
        SetEvent(m_readEvent);
    }

    return true;
}

bool glRemix::SharedMemory::peek(void* dst, const UINT32 max_bytes, const UINT32 offset, UINT32* out_bytes) const
{
    if (!m_header || !dst)
    {
        return false;
    }
    if (m_header->state != SharedState::FILLED)
    {
        return false;
    }

    auto n = m_header->size;
    if (offset + n > max_bytes)
    {
        n = max_bytes > offset ? max_bytes - offset : 0;
    }
    memcpy(dst, m_payload + offset, n);
    if (out_bytes)
    {
        *out_bytes = n;
    }
    // no state change (remains FILLED)
    return true;
}

bool glRemix::SharedMemory::map_common(const HANDLE h_map_file)
{
    m_view = static_cast<LPTSTR>(MapViewOfFile(h_map_file, // handle to map object
                                               FILE_MAP_ALL_ACCESS, // rw permission
                                               0, 0, 0));

    if (m_view == nullptr)
    {
        std::ostringstream ss;
        ss << "Could not map view of file. Error Code: " << GetLastError() << "\n";
        OutputDebugStringA(ss.str().c_str());

        CloseHandle(h_map_file);

        return false;
    }

    m_map = h_map_file;
    m_header = reinterpret_cast<SharedMemoryHeader*>(m_view);
    m_payload = reinterpret_cast<UINT8*>(m_view) + sizeof(SharedMemoryHeader);

    return true;
}

void glRemix::SharedMemory::close_all()
{
    if (m_view)
    {
        UnmapViewOfFile(m_view);
        m_view = nullptr;
    }
    if (m_map)
    {
        CloseHandle(m_map);
        m_map = nullptr;
    }
    if (m_writeEvent)
    {
        CloseHandle(m_writeEvent);
        m_writeEvent = nullptr;
    }
    if (m_readEvent)
    {
        CloseHandle(m_readEvent);
        m_readEvent = nullptr;
    }

    m_header = nullptr;  // host side buffers
    m_payload = nullptr;
}
