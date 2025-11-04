#include "gl_hooks.h"

#include "framework.h"

#include <gl_loader.h>

#include <GL/gl.h>

#include <mutex>
#include <tsl/robin_map.h>

namespace glRemix::hooks
{
    // If forwarding a call is needed for whatever reason, you can write a function to load the true DLL,
    // then use GetProcAddress or wglGetProcAddress to store the real function pointer. But we are not
    // using OpenGL functions, so this does not apply. Might be useful for DLL chaining though.

    struct FakePixelFormat
    {
        PIXELFORMATDESCRIPTOR descriptor{};
        int id = 1;
    };

    struct FakeContext
    {
        HDC last_dc = nullptr;
    };

    // WGL/OpenGL might be called from multiple threads
    std::mutex g_mutex;

    // wglSetPixelFormat will only be called once per context
    // Or if there are multiple contexts they can share the same format since they're fake anyway...
    // TODO: Make the above assumption?
    tsl::robin_map < HDC, FakePixelFormat> g_pixel_formats;

    thread_local HGLRC g_current_context = nullptr;
    thread_local HDC g_current_dc = nullptr;

    FakePixelFormat create_default_pixel_format(const PIXELFORMATDESCRIPTOR *requested)
    {
        if (requested == nullptr)
        {
			// Standard 32-bit color 24-bit depth 8-bit stencil double buffered format
            return
            {
                .descriptor = 
                {
                    .nSize = sizeof(PIXELFORMATDESCRIPTOR),
                    .nVersion = 1,
                    .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
                    .iPixelType = PFD_TYPE_RGBA,
                    .cColorBits = 32,
                    .cDepthBits = 24,
                    .cStencilBits = 8,
                    .iLayerType = PFD_MAIN_PLANE
                }
            };
        }
        FakePixelFormat result;
        result.descriptor = *requested; 
        result.descriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        result.descriptor.nVersion = 1;
        return result;
    }

    void APIENTRY gl_begin_ovr(GLenum mode)
    {
        glRemix::GLBeginCommand payload{mode};
        g_recorder.Record(glRemix::GLCommandType::GL_BEGIN, &payload, sizeof(payload));
    }

    void APIENTRY gl_end_ovr(void)
    {
        glRemix::GLEndCommand payload{}; // init with default 0 value
        g_recorder.Record(glRemix::GLCommandType::GL_END, &payload, sizeof(payload));
    }

    void APIENTRY gl_vertex3f_ovr(GLfloat x, GLfloat y, GLfloat z)
    {
        // Example override that does nothing.
        // You can put a breakpoint here.
        glRemix::GLVertex3fCommand payload{x, y, z};
        g_recorder.Record(glRemix::GLCommandType::GL_VERTEX3F, &payload, sizeof(payload));
    }

    BOOL WINAPI swap_buffers_ovr(HDC)
    {
        g_recorder.EndFrame();
        g_recorder.StartFrame();

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
        // TODO: Derive HWND from HDC for swapchain creation
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
            gl::register_hook("glBegin", reinterpret_cast<PROC>(&gl_begin_ovr));
            gl::register_hook("glEnd", reinterpret_cast<PROC>(&gl_end_ovr));
	        // gl::register_hook("glVertex3f", reinterpret_cast<PROC>(&gl_vertex3f_ovr));
            // TODO: Add more OpenGL overrides
            // Just use the name and make sure the signature matches.
            // You can find exports in the generated gl_wrappers.inl, but extensions are not going to be in there.

			// There will need be a caching system in the renderer for display lists, since the app may use them instead of recording direct calls.
			// A mapping to store recorded commands per display list ID should suffice.

			// FOR SHIM TEAM:
			// The shim should generate a monotonic integer for the ID in glGenLists (which will also be returned to the host app)
			// such that the renderer can construct a mapping for glCallList calls.
			// This same idea applies to any resource creation call that returns an ID (textures for example).
			// ex: glxgears only records geometry once and then replays it every frame via display lists.

            // Override WGL for app to work. Return success and try to do nothing.
	        gl::register_hook("wglChoosePixelFormat", reinterpret_cast<PROC>(&choose_pixel_format_ovr));
	        gl::register_hook("wglDescribePixelFormat", reinterpret_cast<PROC>(&describe_pixel_format_ovr));
	        gl::register_hook("wglGetPixelFormat", reinterpret_cast<PROC>(&get_pixel_format_ovr));
	        gl::register_hook("wglSetPixelFormat", reinterpret_cast<PROC>(&set_pixel_format_ovr));
	        gl::register_hook("wglSwapBuffers", reinterpret_cast<PROC>(&swap_buffers_ovr));
	        gl::register_hook("wglCreateContext", reinterpret_cast<PROC>(&create_context_ovr));
	        gl::register_hook("wglDeleteContext", reinterpret_cast<PROC>(&delete_context_ovr));
	        gl::register_hook("wglGetCurrentContext", reinterpret_cast<PROC>(&get_current_context_ovr));
	        gl::register_hook("wglGetCurrentDC", reinterpret_cast<PROC>(&get_current_dc_ovr));
	        gl::register_hook("wglMakeCurrent", reinterpret_cast<PROC>(&make_current_ovr));
	        gl::register_hook("wglShareLists", reinterpret_cast<PROC>(&share_lists_ovr));
	    });
	}
}
