#pragma once

#include <windows.h>

#include "math_utils.h"

#define DBG_PRINT(fmt, ...)                                                                        \
    do                                                                                             \
    {                                                                                              \
        char _buf[256];                                                                            \
        std::snprintf(_buf, sizeof(_buf), fmt "\n", __VA_ARGS__);                                  \
        OutputDebugStringA(_buf);                                                                  \
    } while (0)

namespace glRemix
{
constexpr UINT32 k_DEFAULT_CAPACITY = 1 * MEGABYTE;  // i.e. 1mb

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
    volatile UINT32 size;        // curr bytes in payload[]
    UINT32 capacity;             // total bytes available in payload[]
};

class SharedMemory
{
private:
    HANDLE m_map = nullptr;  // windows provides `HANDLE` typedef
    LPTSTR m_view = nullptr;
    SharedMemoryHeader* m_header = nullptr;
    UINT8* m_payload = nullptr;

    HANDLE m_write_event = nullptr;
    HANDLE m_read_event = nullptr;

    // helpers
    bool map_common(HANDLE h_map);
    void close_all();

    // preview size, is passed into functions
    static size_t max_object_size(const UINT32 capacity)
    {
        return sizeof(SharedMemoryHeader) + capacity;
    }

public:
    SharedMemory() = default;
    ~SharedMemory();

    // writer creates or opens existing mapping and initializes header.
    bool create_for_writer(const wchar_t* map_name, const wchar_t* write_event_name,
                           const wchar_t* read_event_name, UINT32 capacity = k_DEFAULT_CAPACITY);

    // reader opens existing mapping and maps view.
    bool open_for_reader(const wchar_t* map_name, const wchar_t* write_event_name,
                         const wchar_t* read_event_name);

    // state: EMPTY || CONSUMED -> FILLED
    // returns true if write success
    bool write(const void* src, UINT32 bytes, UINT32 offset = 0) const;

    bool peek(void* dst, UINT32 max_bytes, UINT32 offset, UINT32* out_bytes) const;

    // state: FILLED -> CONSUMED
    // returns true if read success
    bool read(void* dst, UINT32 max_bytes, UINT32 offset = 0, UINT32* out_bytes = nullptr);

    // accesssors
    SharedMemoryHeader* get_header() const
    {
        return m_header;
    }

    UINT8* get_payload() const
    {
        return m_payload;
    }

    UINT32 get_capacity() const
    {
        return m_header ? m_header->capacity : 0;
    }

    /* for usage in future sync ops. dummy fxns for now */
    HANDLE get_write_event() const
    {
        return m_write_event;
    }

    HANDLE get_read_event() const
    {
        return m_read_event;
    }
};
}  // namespace glRemix
