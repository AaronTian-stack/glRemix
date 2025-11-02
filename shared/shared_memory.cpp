#include "shared_memory.h"

glRemix::SharedMemory::~SharedMemory()
{
    _CloseAll();
}

// `CreateFileMapping`
bool glRemix::SharedMemory::CreateForWriter(const wchar_t* name, uint32_t capacity)
{
    return false;
}

// `
bool glRemix::SharedMemory::OpenForReader(const wchar_t* name)
{
    return false;
}

bool glRemix::SharedMemory::WriteOnce(const void* src, uint32_t bytes, uint32_t offset)
{
    return false;
}

bool glRemix::SharedMemory::ReadOnce(void* dst, uint32_t maxBytes, uint32_t offset, uint32_t* outBytes)
{
    return false;
}

bool glRemix::SharedMemory::_MapCommon(HANDLE hMap)
{
    return false;
}

void glRemix::SharedMemory::_CloseAll()
{
}

