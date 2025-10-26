#include "gl_hooks.h"

#include "framework.h"
#include "gl_loader.h"

#include <GL/gl.h>

#include <memory>
#include <mutex>
#include <tsl/robin_map.h>

namespace glremix::hooks
{
    // If forwarding a call is needed for whatever reason, you can write a function to load the true dll,
    // then use GetProcAddress or wglGetProcAddress to store the real function pointer. But we are not
    // using OpenGL functions, so this does not apply.

    struct FakePixelFormat
    {
        PIXELFORMATDESCRIPTOR descriptor{};
        int id = 1;
    };

    struct FakeContext
    {
        HDC last_dc = nullptr;
    };

    // wgl/ogl might be called from multiple threads
    std::mutex g_mutex;

    tsl::robin_map<HDC, FakePixelFormat> g_pixel_formats;
    tsl::robin_map<HGLRC, std::unique_ptr<FakeContext>> g_contexts;

    thread_local HGLRC g_current_context = nullptr;
    thread_local HDC g_current_dc = nullptr;

    FakePixelFormat create_default_pixel_format(const PIXELFORMATDESCRIPTOR *requested)
    {
        FakePixelFormat result;
        result.descriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        result.descriptor.nVersion = 1;
        result.descriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        result.descriptor.iPixelType = PFD_TYPE_RGBA;
        result.descriptor.cColorBits = 32;
        result.descriptor.cDepthBits = 24;
        result.descriptor.cStencilBits = 8;
        result.descriptor.iLayerType = PFD_MAIN_PLANE;

        if (requested != nullptr)
        {
            result.descriptor = *requested;
            result.descriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);
            result.descriptor.nVersion = 1;
        }

        return result;
    }

    void APIENTRY gl_vertex3f_ovr(GLfloat x, GLfloat y, GLfloat z)
    {
        int a = 0; // Breakpoint test
    }

    BOOL WINAPI swap_buffers_ovr(HDC)
    {
        return TRUE;
    }

    int WINAPI choose_pixel_format_ovr(HDC dc, const PIXELFORMATDESCRIPTOR *descriptor)
    {
        if (dc == nullptr)
        {
            return 0;
        }

        std::scoped_lock lock(g_mutex);
        g_pixel_formats[dc] = create_default_pixel_format(descriptor);
        return g_pixel_formats[dc].id;
    }

    int WINAPI describe_pixel_format_ovr(HDC dc, int pixel_format, UINT bytes, LPPIXELFORMATDESCRIPTOR descriptor)
    {
        if (dc == nullptr || pixel_format <= 0 || descriptor == nullptr)
        {
            return 0;
        }

        UNREFERENCED_PARAMETER(bytes);

        std::scoped_lock lock(g_mutex);
        if (!g_pixel_formats.contains(dc))
        {
            *descriptor = create_default_pixel_format(nullptr).descriptor;
            return pixel_format;
        }

        *descriptor = g_pixel_formats[dc].descriptor;
        return pixel_format;
    }

    int WINAPI get_pixel_format_ovr(HDC dc)
    {
        if (dc == nullptr)
        {
            return 0;
        }

        std::scoped_lock lock(g_mutex);
        if (!g_pixel_formats.contains(dc))
        {
            return 0;
        }

        return g_pixel_formats[dc].id;
    }

    BOOL WINAPI set_pixel_format_ovr(HDC dc, int pixel_format, const PIXELFORMATDESCRIPTOR *descriptor)
    {
        if (dc == nullptr || pixel_format <= 0)
        {
            return FALSE;
        }

        std::scoped_lock lock(g_mutex);
        g_pixel_formats[dc] = create_default_pixel_format(descriptor);
        g_pixel_formats[dc].id = pixel_format;
        return TRUE;
    }

    HGLRC WINAPI create_context_ovr(HDC dc)
    {
        // TODO: Derive HWND from HDC to create swapchain
        return reinterpret_cast<HGLRC>(0xDEADBEEF); // Dummy context handle
    }

    BOOL WINAPI delete_context_ovr(HGLRC context)
    {
        return TRUE;
    }

    HGLRC WINAPI get_current_context_ovr()
    {
        return g_current_context;
    }

    HDC WINAPI get_current_dc_ovr()
    {
        return g_current_dc;
    }

    BOOL WINAPI make_current_ovr(HDC dc, HGLRC context)
    {
        g_current_dc = dc;
        return TRUE;
    }

    BOOL WINAPI share_lists_ovr(HGLRC, HGLRC)
    {
        return TRUE;
    }

    std::once_flag g_install_flag;

	void install_overrides()
	{
	    std::call_once(g_install_flag, []()
	    {
	        gl::register_override("glVertex3f", reinterpret_cast<PROC>(&gl_vertex3f_ovr));
            // TODO: more OpenGL overrides

            // Override wgl for app to work
            // Return success and try to do nothing
	        gl::register_override("wglChoosePixelFormat", reinterpret_cast<PROC>(&choose_pixel_format_ovr));
	        gl::register_override("wglDescribePixelFormat", reinterpret_cast<PROC>(&describe_pixel_format_ovr));
	        gl::register_override("wglGetPixelFormat", reinterpret_cast<PROC>(&get_pixel_format_ovr));
	        gl::register_override("wglSetPixelFormat", reinterpret_cast<PROC>(&set_pixel_format_ovr));
	        gl::register_override("wglSwapBuffers", reinterpret_cast<PROC>(&swap_buffers_ovr));
	        gl::register_override("wglCreateContext", reinterpret_cast<PROC>(&create_context_ovr));
	        gl::register_override("wglDeleteContext", reinterpret_cast<PROC>(&delete_context_ovr));
	        gl::register_override("wglGetCurrentContext", reinterpret_cast<PROC>(&get_current_context_ovr));
	        gl::register_override("wglGetCurrentDC", reinterpret_cast<PROC>(&get_current_dc_ovr));
	        gl::register_override("wglMakeCurrent", reinterpret_cast<PROC>(&make_current_ovr));
	        gl::register_override("wglShareLists", reinterpret_cast<PROC>(&share_lists_ovr));
	    });
	}
}
