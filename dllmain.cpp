#ifdef _WIN32
#define _QUANGBT_DLL_LIB
#endif
#ifdef _WIN64
#undef _QUANGBT_DLL_LIB
#define _QUANGBT_DLL_LIB
#endif
#ifdef _QUANGBT_DLL_LIB

// dllmain.cpp : Defines the entry point for the DLL application.
#include "Windows.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

#endif