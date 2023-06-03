#pragma once

#include "Mutex.hpp"
#include "Weapons.hpp"
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <thread>
#include <atomic>
#include <iomanip>
#include <array>

/* game infos */
#define WIN_NAME		"Counter-Strike: Global Offensive - Direct3D 9"
#define CLIENT_MODNAME	"client.dll"
#define ENGINE_MODNAME	"engine.dll"

/* ingame settings */
#define MAXPLAYERS		32
#define AIMBOT_FOV		1.5f
#define AIMBOT_SMOOTH	3.f

/* keys settings */
#define K_AIMBOT		0x37
#define K_GLOW			0x38
#define K_TRIG			0x39
#define K_RADAR			0x30
#define K_AIMLOCK		0x01
#define	K_TRIGLOCK		0x43


class Bypass {

	private:
		HANDLE			m_hProcess;
		DWORD			m_processId;
		uintptr_t		m_modBaseAddr;
		uintptr_t		m_engineAddr;
		Mutex			m_mutex;

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
		void	m_aimbot(void);
		void	m_glow(void);
		void	m_trig(void);
		void	m_radar(void);
		void	m_skinChanger(void);
};
