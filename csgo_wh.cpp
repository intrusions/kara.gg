#pragma warning(disable:4996)

#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <comdef.h>
#include "csgo.hpp"

#define MODULENAME "client.dll"
#define GAMENAME "Counter-Strike: Global Offensive"
#define MAXPLAYERS 64

uintptr_t GetBase(DWORD dwPID, char* modulename);
uintptr_t ReadMem(HANDLE ProcessHandle, uintptr_t Address);
template<typename T>
void WriteMem(HANDLE ProcessHandle, uintptr_t Address, T Value);

using namespace std;

int main() {
    DWORD PID;

    HWND windowHandle = FindWindowA(0, GAMENAME);
    GetWindowThreadProcessId(windowHandle, &PID);

    HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, false, PID);

    uintptr_t base = GetBase(PID, (char*)MODULENAME);

    BOOL wallhack = FALSE;
    BOOL trigger = FALSE;
    

    cout << "F1 : WallHack ON" << endl;
    cout << "F2 : WallHack OFF" << endl;
    cout << "F3 : TRIGGER ON" << endl;
    cout << "F4 : TRIGGER OFF" << endl;

    while (true) 
    {
        // -------------------------WALHACK----------------------------
        if (GetAsyncKeyState(VK_F1))
            wallhack = TRUE;
        else if (GetAsyncKeyState(VK_F2))
            wallhack = FALSE;

        if (wallhack == TRUE)
        {
            uintptr_t glowManager = ReadMem(processHandle, base + hazedumper::signatures::dwGlowObjectManager);
            uintptr_t player = ReadMem(processHandle, base + hazedumper::signatures::dwLocalPlayer); 
            if (player == NULL)
            {
                break;
            }
            int pTeam = ReadMem(processHandle, player + hazedumper::netvars::m_iTeamNum); 
            
            for (USHORT i = 0; i < MAXPLAYERS; i++) 
            {
                uintptr_t otherPlayer = ReadMem(processHandle, base + hazedumper::signatures::dwEntityList + i * 16);
                if (otherPlayer != NULL) 
                {
                    int otherPlayerTeam = ReadMem(processHandle, otherPlayer + hazedumper::netvars::m_iTeamNum);
                    int glow = ReadMem(processHandle, otherPlayer + hazedumper::netvars::m_iGlowIndex);
                    if (otherPlayerTeam != pTeam) 
                    {
                        WriteMem(processHandle, glowManager + ((glow * 56) + 4), (float)2);     // Red
                        WriteMem(processHandle, glowManager + ((glow * 56) + 8), (float)0);     // Green
                        WriteMem(processHandle, glowManager + ((glow * 56) + 12), (float)0);    // Blue
                        WriteMem(processHandle, glowManager + ((glow * 56) + 16), (float)0.5);  // Alpha
                    }
                    WriteMem(processHandle, glowManager + ((glow * 0x38) + 0x24), true);
                    WriteMem(processHandle, glowManager + ((glow * 0x38) + 0x25), false);

                }
            }
        }
        // -------------------------TRIGGER----------------------------
        if (GetAsyncKeyState(VK_F3))
            trigger = TRUE;
        else if (GetAsyncKeyState(VK_F4))
            trigger = FALSE;
        
        if (trigger == TRUE)
        {
            if (GetAsyncKeyState(0x43))
            {
                uintptr_t player = ReadMem(processHandle, base + hazedumper::signatures::dwLocalPlayer); 
                uintptr_t crosshair = ReadMem(processHandle, player + hazedumper::netvars::m_iCrosshairId); 
                if (crosshair != 0 && crosshair < MAXPLAYERS)
                {
                    uintptr_t playerInCrosshair = ReadMem(processHandle, base + hazedumper::signatures::dwEntityList + (crosshair - 1) * 16);
                    if (playerInCrosshair != NULL)
                    {
                        int playerTeam = ReadMem(processHandle, playerInCrosshair + hazedumper::netvars::m_iTeamNum);
                        int pTeam = ReadMem(processHandle, player + hazedumper::netvars::m_iTeamNum); 
                        if (playerTeam != pTeam) 
                        {
                            WriteMem(processHandle, base + hazedumper::signatures::dwForceAttack, 5); 
                            Sleep(12);
                            WriteMem(processHandle, base + hazedumper::signatures::dwForceAttack, 4); 
                        }
                    }
                }
            }
        }

    }
}

uintptr_t ReadMem(HANDLE ProcessHandle, uintptr_t Address) 
{
    uintptr_t read;
    ReadProcessMemory(ProcessHandle, (LPVOID)Address, &read, sizeof(read), NULL);
    return read;
}

template<typename T>
void WriteMem(HANDLE ProcessHandle, uintptr_t Address, T Value) 
{
    WriteProcessMemory(ProcessHandle, (LPVOID)Address, &Value, sizeof(Value), NULL);
}

uintptr_t GetBase(DWORD dwPID, char* moduleName) 
{
    uintptr_t moduleBase = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, dwPID);
    if (snapshot != INVALID_HANDLE_VALUE) 
    {

        MODULEENTRY32 moduleEntry;
        moduleEntry.dwSize = sizeof(MODULEENTRY32);
        if (Module32First(snapshot, &moduleEntry))
        {
            do
            {
                _bstr_t moduleEntryName(moduleEntry.szModule);
                if (strcmp(moduleEntryName, moduleName) == 0)
                {
                    moduleBase = (uintptr_t)moduleEntry.modBaseAddr;
                    break;
                }
            } while (Module32Next(snapshot, &moduleEntry));
        }
        CloseHandle(snapshot);
    }
    return moduleBase;
}