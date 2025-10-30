#pragma once

#ifndef GLREMIX_GL_VOID_WRAPPER
#define GLREMIX_GL_VOID_WRAPPER(name, params, args)                                                        \
    extern "C" __declspec(dllexport) void APIENTRY name params                                             \
    {                                                                                                      \
        using FnType = void(APIENTRY*) params;                                                             \
        if (auto override_fn = reinterpret_cast<FnType>(glremix::gl::find_hook(#name)))                    \
        {                                                                                                  \
            override_fn args;                                                                              \
            return;                                                                                        \
        }                                                                                                  \
        glremix::gl::report_missing_function(#name);                                                       \
    }
#endif

#ifndef GLREMIX_GL_RETURN_WRAPPER
#define GLREMIX_GL_RETURN_WRAPPER(ret, name, params, args)                                                 \
    extern "C" __declspec(dllexport) ret APIENTRY name params                                              \
    {                                                                                                      \
        using FnType = ret(APIENTRY*) params;                                                              \
        if (auto override_fn = reinterpret_cast<FnType>(glremix::gl::find_hook(#name)))                    \
        {                                                                                                  \
            return override_fn args;                                                                       \
        }                                                                                                  \
        glremix::gl::report_missing_function(#name);                                                       \
        return {};                                                                                         \
    }
#endif

#ifndef GLREMIX_WGL_RETURN_WRAPPER
#define GLREMIX_WGL_RETURN_WRAPPER(retType, name, params, args, default_value)                             \
    extern "C" __declspec(dllexport) retType WINAPI name params                                            \
    {                                                                                                      \
        using FnType = retType(WINAPI*) params;                                                            \
        if (auto override_fn = reinterpret_cast<FnType>(glremix::gl::find_hook(#name)))                    \
        {                                                                                                  \
            return override_fn args;                                                                       \
        }                                                                                                  \
        glremix::gl::report_missing_function(#name);                                                       \
        return default_value;                                                                              \
    }
#endif
