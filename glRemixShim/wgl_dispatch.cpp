#include "gl_loader.h"

#include "wgl_export_aliases.inl"

#define GLREMIX_WGL_RETURN_WRAPPER(retType, name, params, args, defaultValue)                        \
    extern "C" __declspec(dllexport) retType WINAPI name params                                      \
    {                                                                                                \
        using FnType = retType(WINAPI*) params;                                                      \
        if (auto override_fn = reinterpret_cast<FnType>(glremix::gl::find_override(#name)))          \
        {                                                                                            \
            return override_fn args;                                                                 \
        }                                                                                            \
        glremix::gl::report_missing_function(#name);                                                 \
        return defaultValue;                                                                         \
    }

#include "wgl_wrappers.inl"

#undef GLREMIX_WGL_RETURN_WRAPPER

extern "C" __declspec(dllexport) PROC WINAPI wglGetProcAddress(LPCSTR name)
{
    if (name == nullptr)
    {
        return nullptr;
    }

    // wgl export, sometimes wgl functions are queried through wglGetProcAddress
    if (PROC exported = glremix::gl::find_export(name); exported != nullptr)
    {
        return exported;
    }

	// Find an override we defined
    if (PROC override_proc = glremix::gl::find_override(name); override_proc != nullptr)
    {
        return override_proc;
    }

    glremix::gl::report_missing_function(name);
    return nullptr;
}

namespace glremix::gl
{
    void register_wgl_wrappers()
    {
	#include "wgl_register.inl"
        // wglGetProcAddress has custom implementation, register manually
        register_export("wglGetProcAddress", reinterpret_cast<PROC>(&wglGetProcAddress));
    }
}
