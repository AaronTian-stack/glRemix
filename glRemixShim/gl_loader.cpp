#include "gl_loader.h"

#include <mutex>
#include <string>
#include <tsl/robin_map.h>

namespace glremix::gl
{
	// Both WGL and OpenGL functions may be called from multiple threads hence the mutex
    std::mutex g_hook_mutex;

	// Function pointers for our custom hook implementations
	tsl::robin_map<std::string, PROC> g_hooks;

	std::once_flag g_initialize_flag;
	void initialize()
	{
		std::call_once(g_initialize_flag, []
		{
			// TODO: Setup IPC
		});
	}

	void register_hook(const char *name, PROC proc)
	{
	    if (name == nullptr)
	    {
	        return;
	    }

	    std::scoped_lock lock(g_hook_mutex);
	    if (proc == nullptr)
	    {
	        g_hooks.erase(name);
	        return;
	    }

	    g_hooks[name] = proc;
	}

	PROC find_hook(const char *name)
	{
	    if (name == nullptr)
	    {
	        return nullptr;
	    }

	    std::scoped_lock lock(g_hook_mutex);
	    if (g_hooks.contains(name))
	    {
	        return g_hooks[name];
	    }

	    return nullptr;
	}

	void report_successful_function(const char *name)
	{
		if (name == nullptr)
		{
			return;
		}

		char buffer[256];
		std::snprintf(buffer, sizeof(buffer), "glRemix INFO: success! found override for OpenGL symbol: %s\n", name);
		OutputDebugStringA(buffer);
	}

    void report_missing_function(const char *name)
	{
		if (name == nullptr)
		{
			return;
		}

		char buffer[256];
		std::snprintf(buffer, sizeof(buffer), "glRemix ERROR: missing OpenGL symbol: %s\n", name);
		OutputDebugStringA(buffer);
	}
}
