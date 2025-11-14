#pragma once

#include <windows.h>
#include <cstdint>

namespace glRemix
{
// Local keyword allows to stay per-session, Global requires elevated permissions
// wchar_t is standard for file mapping names in windows (though can maybe switch with TCHAR*)
constexpr const wchar_t* kDEFAULT_MAP_NAME = L"Local\\glRemix_DefaultMap";
constexpr UINT32 kDEFAULT_CAPACITY = 1u << 20;  // i.e. 1mb

/*
 * Current state options :
 * 0 = EMPTY (writer may write)
 * 1 = FILLED (reader may read)
 * 2 = CONSUMED (reader set after reading, may not be necessary)
 */
// use LONG so that we may use `InterlockedExchange`
enum class SharedState : LONG
{
    EMPTY = 0,
    FILLED = 1,
    CONSUMED = 2
};

// payload[] will be written thereafter in memory
struct SharedMemoryHeader
{
    // must use UINT32 here, int does not fix size
    volatile SharedState state;  // tracks state across processes
    volatile UINT32 size;      // curr bytes in payload[]
    UINT32 capacity;           // total bytes available in payload[]
};

class SharedMemory
{
public:
    SharedMemory() = default;
    ~SharedMemory();

    // writer creates or opens existing mapping and initializes header.
    bool create_for_writer(const wchar_t* name = kDEFAULT_MAP_NAME,
                           UINT32 capacity = kDEFAULT_CAPACITY);

    // reader opens existing mapping and maps view.
    bool open_for_reader(const wchar_t* name = kDEFAULT_MAP_NAME);

    // state: EMPTY || CONSUMED -> FILLED
    // returns true if write success
    bool write(const void* src, UINT32 bytes, UINT32 offset = 0);

    bool peek(void* dst, UINT32 maxBytes, UINT32 offset, UINT32* outBytes);

    // state: FILLED -> CONSUMED
    // returns true if read success
    bool read(void* dst, UINT32 maxBytes, UINT32 offset = 0, UINT32* outBytes = nullptr);

    // accesssors
    inline SharedMemoryHeader* get_header() const
    {
        return m_header;
    }

    inline uint8_t* get_payload() const
    {
        return m_payload;
    }

    inline UINT32 get_capacity() const
    {
        return m_header ? m_header->capacity : 0;
    }

    /* for usage in future sync ops. dummy fxns for now */
    HANDLE write_event() const
    {
        return m_writeEvent;
    }

    HANDLE read_event() const
    {
        return m_readEvent;
    }

private:
    HANDLE m_map = nullptr;  // windows provides `HANDLE` typedef
    LPTSTR m_view = nullptr;
    SharedMemoryHeader* m_header = nullptr;
    uint8_t* m_payload = nullptr;

    HANDLE m_writeEvent = nullptr;
    HANDLE m_readEvent = nullptr;

    /* helpers */
    bool _map_common(HANDLE hMap);
    void _close_all();

    // preview size, is passed into functions
    inline size_t _max_object_size(UINT32 capacity)
    {
        return sizeof(SharedMemoryHeader) + capacity;
    }
};
}  // namespace glRemix
