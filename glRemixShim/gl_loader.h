#pragma once

#include "framework.h"

namespace glremix::gl
{
	void initialize();
	void register_override(const char* name, PROC proc);
	PROC find_override(const char* name);
	void register_export(const char* name, PROC proc);
	PROC find_export(const char* name);
	void report_missing_function(const char* name);

	void register_core_wrappers();
	void register_wgl_wrappers();
}
