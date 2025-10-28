#include "framework.h"
#include "gl_hooks.h"
#include "gl_loader.h"

BOOL APIENTRY DllMain(HMODULE h_module, DWORD ul_reason_for_call, LPVOID lp_reserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(h_module);
        glremix::gl::initialize();
        glremix::hooks::install_overrides();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
	default:
        break;
    }
    return TRUE;
}
