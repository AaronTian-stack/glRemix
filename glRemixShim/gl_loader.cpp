#include "gl_loader.h"

#include <mutex>
#include <string>
#include <tsl/robin_map.h>
#include <vector>

namespace glremix::gl
{
    std::once_flag g_initialize_flag;

	// Both wgl and OpenGL functions may be called from multiple threads hence the mutexes

    std::mutex g_override_mutex;
    tsl::robin_map<std::string, PROC> g_overrides; // Function pointers for our wgl overrides

    std::mutex g_export_mutex;
    tsl::robin_map<std::string, PROC> g_exports; // All other OpenGL functions

	void initialize()
	{
	    std::call_once(g_initialize_flag, []
	    {
	        register_core_wrappers();
	        register_wgl_wrappers();
	    });
	}

	void register_override(const char *name, PROC proc)
	{
	    if (name == nullptr)
	    {
	        return;
	    }

	    std::scoped_lock lock(g_override_mutex);
	    if (proc == nullptr)
	    {
	        g_overrides.erase(name);
	        return;
	    }

	    g_overrides[name] = proc;
	}

	PROC find_override(const char *name)
	{
	    if (name == nullptr)
	    {
	        return nullptr;
	    }

	    std::scoped_lock lock(g_override_mutex);
	    if (g_overrides.contains(name))
	    {
	        return g_overrides[name];
	    }

	    return nullptr;
	}

	void register_export(const char *name, PROC proc)
	{
	    if (name == nullptr || proc == nullptr)
	    {
	        return;
	    }

	    std::scoped_lock lock(g_export_mutex);
	    g_exports[name] = proc;
	}

	PROC find_export(const char *name)
	{
	    if (name == nullptr)
	    {
	        return nullptr;
	    }

	    std::scoped_lock lock(g_export_mutex);
	    if (g_exports.contains(name))
	    {
	        return g_exports[name];
	    }

	    return nullptr;
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
