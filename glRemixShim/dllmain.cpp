#include "gl_hooks.h"
#include "gl_loader.h"

BOOL APIENTRY DllMain(HMODULE h_module, DWORD ul_reason_for_call, LPVOID lp_reserved)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(h_module);
            //glRemix::gl::initialize();
            glRemix::hooks::install_overrides();
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH: break;
        case DLL_PROCESS_DETACH:
            if (glRemix::gl::g_renderer_process)
            {
                TerminateProcess(glRemix::gl::g_renderer_process, 0);
                CloseHandle(glRemix::gl::g_renderer_process);
            }
            break;
    }
    return TRUE;
}
