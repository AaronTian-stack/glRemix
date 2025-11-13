#pragma once

#include <cstdint>
#include <cassert>

inline bool u64_overflows_u32(uint64_t v)
{
    return v > UINT32_MAX;
}

inline uint64_t ceil_div(uint64_t a, uint64_t b)
{
    return (a + b - 1) / b;
}

inline bool is_power_of_two(uint32_t value)
{
    return (value & (value - 1)) == 0;
}

inline uint32_t align_u32(uint32_t size, uint32_t alignment)
{
    assert(is_power_of_two(alignment));
    return (size + alignment - 1) & ~(alignment - 1);
}

inline uint64_t align_u64(uint64_t size, uint64_t alignment)
{
    return (size + alignment - 1) & ~(alignment - 1);
}
