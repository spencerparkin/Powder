#include "ModuleMain.h"
#include "PathResolver.h"
#include <Windows.h>

namespace Powder
{
    void* moduleHandle = nullptr;
}

BOOL WINAPI DllMain(_In_ HINSTANCE hinstDLL, _In_ DWORD fdwReason, _In_ LPVOID lpvReserved)
{
    using namespace Powder;

    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            moduleHandle = hinstDLL;
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