#pragma once

#include <windows.h>
#include <cstdint>

namespace glRemix
{
// Local keyword allows to stay per-session, Global requires elevated permissions
// wchar_t is standard for file mapping names in windows (though can maybe switch with TCHAR*)
constexpr const wchar_t* kDEFAULT_MAP_NAME = L"Local\\glRemix_DefaultMap";
constexpr uint32_t kDEFAULT_CAPACITY = 1u << 20;  // i.e. 1mb

/*
 * Current state options :
 * 0 = EMPTY (writer may write)
 * 1 = FILLED (reader may read)
 * 2 = CONSUMED (reader set after reading, may not be necessary)
 */
// use LONG so that we may use `InterlockedExchange`
enum class SharedState : LONG { EMPTY = 0, FILLED = 1, CONSUMED = 2 };

// payload[] will be written thereafter in memory
struct SharedMemoryHeader
{
    // must use uint32_t here, int does not fix size
    volatile SharedState state;  // tracks state across processes
    volatile uint32_t size;      // curr bytes in payload[]
    uint32_t capacity;           // total bytes available in payload[]
};

class SharedMemory
{
public:
    SharedMemory() = default;
    ~SharedMemory();

    // writer creates or opens existing mapping and initializes header.
    bool CreateForWriter(const wchar_t* name = kDEFAULT_MAP_NAME,
                         uint32_t capacity = kDEFAULT_CAPACITY);

    // reader opens existing mapping and maps view.
    bool OpenForReader(const wchar_t* name = kDEFAULT_MAP_NAME);

    // state: EMPTY || CONSUMED -> FILLED
    // returns true if write success
    bool Write(const void* src, uint32_t bytes, uint32_t offset = 0);

    bool Peek(void* dst, uint32_t maxBytes, uint32_t offset, uint32_t* outBytes);

    // state: FILLED -> CONSUMED
    // returns true if read success
    bool Read(void* dst, uint32_t maxBytes, uint32_t offset = 0, uint32_t* outBytes = nullptr);

    // accesssors
    inline SharedMemoryHeader* GetHeader() const
    {
        return m_header;
    }

    inline uint8_t* GetPayload() const
    {
        return m_payload;
    }

    inline uint32_t GetCapacity() const
    {
        return m_header ? m_header->capacity : 0;
    }

    /* for usage in future sync ops. dummy fxns for now */
    HANDLE WriteEvent() const
    {
        return m_writeEvent;
    }

    HANDLE ReadEvent() const
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
    bool _MapCommon(HANDLE hMap);
    void _CloseAll();

    // preview size, is passed into functions
    inline size_t _MaxObjectSize(uint32_t capacity)
    {
        return sizeof(SharedMemoryHeader) + capacity;
    }
};
}  // namespace glRemix
