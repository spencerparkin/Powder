#include "ModuleMain.h"
#include "PathResolver.h"
#include <Windows.h>

BOOL WINAPI DllMain(_In_ HINSTANCE hinstDLL, _In_ DWORD fdwReason, _In_ LPVOID lpvReserved)
{
    using namespace Powder;

    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            char modulePathBuffer[1024];
            DWORD modulePathBufferSize = sizeof(modulePathBuffer);
            DWORD size = ::GetModuleFileNameA(hinstDLL, modulePathBuffer, modulePathBufferSize);
            if (size > 0)
                pathResolver.FindBaseDirectoryUsingModulePath(modulePathBuffer);
            break;
        }
        case DLL_THREAD_ATTACH:
        {
            break;
        }
        case DLL_THREAD_DETACH:
        {
            break;
        }
        case DLL_PROCESS_DETACH:
        {
            break;
        }
    }

    return TRUE;
}