#pragma once

#include "framework.h"

namespace glremix::gl
{
	void initialize();

	// Add function pointer to hooks map using name as key
	void register_hook(const char* name, PROC proc);

	// Try to return hooked function pointer using name as hook map lookup
	PROC find_hook(const char* name);

    // Print out function name successfully found
	void report_successful_function(const char* name);

	// Print out missing function name to debug output
	void report_missing_function(const char* name);
}
