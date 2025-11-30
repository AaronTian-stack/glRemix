#include <cstdio>
#include <format>

#define DBG_PRINT(fmt, ...)                                                                        \
    do                                                                                             \
    {                                                                                              \
        char _buf[256];                                                                            \
        std::snprintf(_buf, sizeof(_buf), fmt "\n", __VA_ARGS__);                                  \
        OutputDebugStringA(_buf);                                                                  \
    } while (0)

#define FSTR(fmt, ...) std::format(fmt, __VA_ARGS__)
