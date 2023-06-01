#pragma once

/* libs */
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <thread>
#include <atomic>
#include <iomanip>

/* game infos */
#define WIN_NAME		"Counter-Strike: Global Offensive - Direct3D 9"
#define CLIENT_MODNAME	"client.dll"
#define ENGINE_MODNAME	"engine.dll"

/* ingame settings */
#define MAXPLAYERS		64
#define AIMBOT_FOV		1.5f

/* keys settings */
#define K_AIMBOT		0x37
#define K_GLOW			0x38
#define K_TRIG			0x39
#define K_RADAR			0x30


class Bypass {

	private:
		HANDLE		m_hProcess = NULL;
		DWORD		m_processId = 0;
		uintptr_t	m_modBaseAddr = 0;
		uintptr_t	m_engineAddr = 0;

	public:
		Bypass();
		virtual ~Bypass();


		/* memory function */
		bool	attach(void);
		bool	getModuleBaseAddress(const char* modName);


		/* rpm/wpm function */
		template<typename T>
		void	m_writeProcessMemory(uintptr_t lpBaseAddress, T lpBuffer);
		template<typename T>
		T		&m_readProcessMemory(uintptr_t lpBaseAddress);


		/* multithreading init */
		void	startMultiThreading(void);


		/* cheating function */
		void	m_aimbot(std::atomic<bool> &isActive);
		void	m_glow(std::atomic<bool> &isActive);
		void	m_trig(std::atomic<bool> &isActive);
		void	m_radar(std::atomic<bool> &isActive);
};

