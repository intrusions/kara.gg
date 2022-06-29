#pragma warning(disable:4996)

#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <comdef.h>
#include "csgo_offsets.hpp"

#define MODULENAME "client.dll"
#define GAMENAME "Counter-Strike: Global Offensive"
#define MAXPLAYERS 64

using namespace std;


uintptr_t GetBase(DWORD dw_pid, char* module_name);
uintptr_t ReadMem(HANDLE process_handle, uintptr_t address);
template<typename T>
void WriteMem(HANDLE process_handle, uintptr_t address, T value);


int main() {
	DWORD PID;

	HWND window_handle = FindWindowA(0, GAMENAME);
	GetWindowThreadProcessId(window_handle, &PID);
	HANDLE process_handle = OpenProcess(PROCESS_ALL_ACCESS, false, PID);
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

		if (wallhack)
		{
			uintptr_t glow_manager = ReadMem(process_handle, base + hazedumper::signatures::dwGlowObjectManager);
			uintptr_t player = ReadMem(process_handle, base + hazedumper::signatures::dwLocalPlayer); 
			if (!player)
				break;
			int p_team = ReadMem(process_handle, player + hazedumper::netvars::m_iTeamNum); 
			for (USHORT i = 0; i < MAXPLAYERS; i++) 
			{
				uintptr_t other_player = ReadMem(process_handle, base + hazedumper::signatures::dwEntityList + i * 16);
				if (other_player)
				{
					int other_player_team = ReadMem(process_handle, other_player + hazedumper::netvars::m_iTeamNum);
					int glow = ReadMem(process_handle, other_player + hazedumper::netvars::m_iGlowIndex);
					if (other_player_team != p_team) 
					{
						WriteMem(process_handle, glow_manager + ((glow * 56) + 4), (float)2);     // Red
						WriteMem(process_handle, glow_manager + ((glow * 56) + 8), (float)0);     // Green
						WriteMem(process_handle, glow_manager + ((glow * 56) + 12), (float)0);    // Blue
						WriteMem(process_handle, glow_manager + ((glow * 56) + 16), (float)0.5);  // Alpha
					}
					WriteMem(process_handle, glow_manager + ((glow * 0x38) + 0x24), true);
					WriteMem(process_handle, glow_manager + ((glow * 0x38) + 0x25), false);

				}
			}
		}

		// -------------------------TRIGGER----------------------------
		if (GetAsyncKeyState(VK_F3))
			trigger = TRUE;
		else if (GetAsyncKeyState(VK_F4))
			trigger = FALSE;
		
		if (trigger)
		{
			if (GetAsyncKeyState(0x43))
			{
				uintptr_t player = ReadMem(process_handle, base + hazedumper::signatures::dwLocalPlayer); 
				uintptr_t crosshair = ReadMem(process_handle, player + hazedumper::netvars::m_iCrosshairId); 
				if (crosshair && crosshair < MAXPLAYERS)
				{
					uintptr_t player_in_crosshair = ReadMem(process_handle, base + hazedumper::signatures::dwEntityList + (crosshair - 1) * 16);
					if (player_in_crosshair != NULL)
					{
						int playerTeam = ReadMem(process_handle, player_in_crosshair + hazedumper::netvars::m_iTeamNum);
						int p_team = ReadMem(process_handle, player + hazedumper::netvars::m_iTeamNum); 
						if (playerTeam != p_team) 
						{
							WriteMem(process_handle, base + hazedumper::signatures::dwForceAttack, 5); 
							Sleep(12);
							WriteMem(process_handle, base + hazedumper::signatures::dwForceAttack, 4); 
						}
					}
				}
			}
		}
	}
}

uintptr_t ReadMem(HANDLE process_handle, uintptr_t address) 
{
	uintptr_t read;
	ReadProcessMemory(process_handle, (LPVOID)address, &read, sizeof(read), NULL);
	return (read);
}

template<typename T>
void WriteMem(HANDLE process_handle, uintptr_t address, T value) 
{
	WriteProcessMemory(process_handle, (LPVOID)address, &value, sizeof(value), NULL);
}

uintptr_t GetBase(DWORD dw_pid, char* module_name) 
{
	uintptr_t module_base = 0;
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, dw_pid);
	if (snapshot != INVALID_HANDLE_VALUE) 
	{
		MODULEENTRY32 moduleEntry;
		moduleEntry.dwSize = sizeof(MODULEENTRY32);
		if (Module32First(snapshot, &moduleEntry))
		{
			do
			{
				_bstr_t moduleEntryName(moduleEntry.szModule);
				if (!(strcmp(moduleEntryName, module_name)))
				{
					module_base = (uintptr_t)moduleEntry.modBaseAddr;
					break;
				}
			}
			while (Module32Next(snapshot, &moduleEntry));
		}
		CloseHandle(snapshot);
	}
	return (module_base);
}