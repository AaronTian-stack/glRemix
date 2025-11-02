#pragma once

#include <windows.h>
#include <cstdint>

namespace glRemix
{
    // Local keyword allows to stay per-session, Global requires elevated permissions
    // wchar_t is standard for file mapping names in windows
    constexpr const wchar_t* kDEFAULT_MAP_NAME = L"Local\\glRemix_Shared\\SingleFrameCapture";
    constexpr uint32_t kDEFAULT_CAPACITY = 1u << 20; // i.e. 1mb

   // Current state options:
   // 0 = EMPTY (writer may write)
   // 1 = FILLED (reader may read)
   // 2 = CONSUMED (reader set after reading, may not be necessary)
    struct SharedMemoryHeader
    {
        // must use uint32_t here, int does not fix size
        volatile uint32_t state;       // tracks state across processes
        volatile uint32_t size;    // curr bytes in payload[]
        uint32_t maxSize;         // total bytes available in payload[]
    };

    class SharedMemory
    {
    public:
        SharedMemory() = default;
        ~SharedMemory();

        // writer creates or opens existing mapping and initializes header.
        bool CreateForWriter(const wchar_t* name = kDEFAULT_MAP_NAME, uint32_t capacity = kDefaultCapacity);

        // reader opens existing mapping and maps view.
        bool OpenForReader(const wchar_t* name = kDEFAULT_MAP_NAME);

        // state: EMPTY -> FILLED
        // returns true if write success
        bool WriteOnce(const void* src, uint32_t bytes);

        // state: FILLED -> CONSUMED
        // returns true if read success
        bool ReadOnce(void* dst, uint32_t maxBytes, uint32_t* outBytes = nullptr);

        // access to header + payload
        inline SharedMemoryHeader* Header() const { return m_header; }
        inline uint8_t* Payload() const { return m_payload; }
        inline uint32_t Capacity() const { return m_header ? m_header->maxSize : 0; }

    private:
        HANDLE m_map = nullptr; // windows provides `HANDLE` typedef
        void* m_view = nullptr;
        SharedMemoryHeader* m_header = nullptr;
        uint8_t* m_payload = nullptr;

        /* helpers */
        bool _MapCommon(HANDLE hMap);
        void _CloseAll();
    };
}
