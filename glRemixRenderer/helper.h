#pragma once

#include <cstdint>

inline bool u64_overflows_u32(uint64_t v)
{
	return v > UINT32_MAX;
}

inline uint64_t ceil_div(uint64_t a, uint64_t b)
{
	return (a + b - 1) / b;
}

