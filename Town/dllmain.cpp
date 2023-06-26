// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

typedef void(__thiscall* SendFunction)(DWORD, uint8_t*, uint32_t);

void GameProcMainProcess()
{
    if (GetAsyncKeyState(VK_F2) & 1)
    {
        uint8_t bTown[2] =
        {
            0x48,
            0x00
        };

        SendFunction pSendFunction = (SendFunction)0x4A5C40;
        pSendFunction(*reinterpret_cast<DWORD*>(0xF50FD0), bTown, sizeof(bTown));
    }
}

void PatchGameProcMainThread()
{
    BYTE byPatch1[] =
    {
        0x60,                               //pushad
        0xBA, 0x00, 0x00, 0x00, 0x00,       //mov edx,00000000 <-- Hook Main Thread Address
        0xFF, 0xD2,                         //call edx
        0x61,                               //popad
        0xBA, 0x00, 0x00, 0x00, 0x00,       //mov edx,00000000 <-- Main Thread Address
        0x83, 0xC2, 0x5,                    //add edx,05
        0xFF, 0xE2                          //jmp edx
    };

    DWORD pGameProcMainProcess = (DWORD)(LPVOID*)GameProcMainProcess;
    memcpy(byPatch1 + 2, &pGameProcMainProcess, sizeof(pGameProcMainProcess));

    DWORD pMainThread = 0x45B34A;
    memcpy(byPatch1 + 10, &pMainThread, sizeof(pMainThread));

    LPVOID pPatchAddress = VirtualAllocEx(GetCurrentProcess(), 0, sizeof(byPatch1), MEM_COMMIT, PAGE_EXECUTE_READWRITE);

    if (pPatchAddress == 0)
        return;

    WriteProcessMemory(GetCurrentProcess(), (LPVOID)pPatchAddress, &byPatch1[0], sizeof(byPatch1), 0);

    BYTE byPatch2[] =
    {
        0xE9, 0x00, 0x00, 0x00, 0x00
    };

    DWORD iCallDifference = ((DWORD)pPatchAddress - 0x45B34A - 5);
    memcpy(byPatch2 + 1, &iCallDifference, sizeof(iCallDifference));

    DWORD iOldProtection;
    VirtualProtectEx(GetCurrentProcess(), (LPVOID)0x45B34A, sizeof(byPatch2), PAGE_EXECUTE_READWRITE, &iOldProtection);
    WriteProcessMemory(GetCurrentProcess(), (LPVOID)0x45B34A, &byPatch2[0], sizeof(byPatch2), 0);
    VirtualProtectEx(GetCurrentProcess(), (LPVOID)0x45B34A, sizeof(byPatch2), iOldProtection, &iOldProtection);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        {
            PatchGameProcMainThread();
        }
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

