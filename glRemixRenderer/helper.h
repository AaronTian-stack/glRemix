#pragma once

#include <cstdint>
#include <cassert>

inline bool u64_overflows_u32(UINT64 v)
{
    return v > UINT32_MAX;
}

inline UINT64 ceil_div(UINT64 a, UINT64 b)
{
    return (a + b - 1) / b;
}

inline bool is_power_of_two(UINT32 value)
{
    return (value & (value - 1)) == 0;
}

inline UINT32 align_u32(UINT32 size, UINT32 alignment)
{
    assert(is_power_of_two(alignment));
    return (size + alignment - 1) & ~(alignment - 1);
}

inline UINT64 align_u64(UINT64 size, UINT64 alignment)
{
    return (size + alignment - 1) & ~(alignment - 1);
}
