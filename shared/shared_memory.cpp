#include "shared_memory.h"

#include <sstream>

glRemix::SharedMemory::~SharedMemory()
{
    _CloseAll();
}

// `CreateFileMapping`
bool glRemix::SharedMemory::CreateForWriter(const wchar_t* name, uint32_t capacity)
{
    _CloseAll();

    DWORD _maxObjSize = static_cast<DWORD>(_MaxObjectSize(capacity));
    HANDLE hMapFile = CreateFileMappingW(INVALID_HANDLE_VALUE,  // use paging file
                                         NULL,                  // default security
                                         PAGE_READWRITE,        // rw access
                                         0,            // maximum object size (high-order DWORD)
                                         _maxObjSize,  // maximum object size (low-order DWORD)
                                         name);        // name of mapping object

    if (!hMapFile)
    {
        std::ostringstream ss;
        ss << "Could not create file mapping of file" << GetLastError() << "\n";
        OutputDebugStringA(ss.str().c_str());
        return false;
    }

    if (!_MapCommon(hMapFile))
    {
        // _MapCommon does its own error handling
        CloseHandle(hMapFile);
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

// `
bool glRemix::SharedMemory::OpenForReader(const wchar_t* name)
{
    _CloseAll();

    HANDLE hMapFile = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, name);
    if (!hMapFile)
    {
        std::ostringstream ss;
        ss << "Could not open file mapping of file. Ensure external process has created it.";
        ss << GetLastError() << "\n";
        OutputDebugStringA(ss.str().c_str());
        return false;
    }

    if (!_MapCommon(hMapFile))
    {
        CloseHandle(hMapFile);
        return false;
    }

    // Try to open events (safe if they don't exist yet)
    m_writeEvent = OpenEventW(EVENT_ALL_ACCESS, FALSE, L"Local\\glRemix_WriteEvent");
    m_readEvent = OpenEventW(EVENT_ALL_ACCESS, FALSE, L"Local\\glRemix_ReadEvent");

    return true;
}

bool glRemix::SharedMemory::WriteOnce(const void* src, uint32_t bytes, uint32_t offset)
{
    std::ostringstream ss;
    if (!m_header || !src || bytes > m_header->capacity)
    {
        ss << "File not ready for writing, call `::CreateForWriter` first." << "\n";
        OutputDebugStringA(ss.str().c_str());
        return false;
    }

    // Only write when EMPTY
    if (m_header->state != SharedState::EMPTY)
    {
        ss << "File not available for writing." << "\n";
        OutputDebugStringA(ss.str().c_str());
        return false;
    }

    std::memcpy(m_payload + offset, src, bytes);
    m_header->size = bytes;

    // mark as FILLED so the reader knows data is ready
    InterlockedExchange(reinterpret_cast<volatile LONG*>(&m_header->state),
                        static_cast<LONG>(SharedState::FILLED));

    // signal write event
    if (m_writeEvent)
    {
        SetEvent(m_writeEvent);
    }

    return true;
}

bool glRemix::SharedMemory::ReadOnce(void* dst,
                                     uint32_t maxBytes,
                                     uint32_t offset,
                                     uint32_t* outBytes)
{
    if (!m_header || !dst)
    {
        std::ostringstream ss;
        ss << "File not ready for writing, call `::CreateForWriter` first." << "\n";
        OutputDebugStringA(ss.str().c_str());

        return false;
    }

    // wait for data to be FILLED
    // this is probably not best practice and just for MVP purposes
    while (m_header->state != SharedState::FILLED)
    {
        Sleep(1);
    }

    uint32_t n = m_header->size;
    if (n > maxBytes)
    {
        std::ostringstream ss;
        ss << "Header size greater than desired max bytes. Truncating..." << "\n";
        OutputDebugStringA(ss.str().c_str());

        n = maxBytes;
    }

    std::memcpy(dst, m_payload + offset, n); // copy into payload

    if (outBytes)
    {
        *outBytes = n;
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

bool glRemix::SharedMemory::_MapCommon(HANDLE hMapFile)
{
    m_view = (LPTSTR)MapViewOfFile(hMapFile,             // handle to map object
                                   FILE_MAP_ALL_ACCESS,  // rw permission
                                   0,
                                   0,
                                   0);

    if (m_view == NULL)
    {
        std::ostringstream ss;
        ss << "Could not map view of file" << GetLastError() << "\n";
        OutputDebugStringA(ss.str().c_str());

        CloseHandle(hMapFile);

        return false;
    }

    m_map = hMapFile;
    m_header = reinterpret_cast<SharedMemoryHeader*>(m_view);
    m_payload = reinterpret_cast<uint8_t*>(m_view) + sizeof(SharedMemoryHeader);

    return true;
}

void glRemix::SharedMemory::_CloseAll()
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
