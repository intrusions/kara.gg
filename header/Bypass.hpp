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
#include <map>
#include <fstream>
#include <string>
#include <variant>


/* game infos */
#define WIN_NAME		"Counter-Strike: Global Offensive - Direct3D 9"
#define CLIENT_MODNAME	"client.dll"
#define ENGINE_MODNAME	"engine.dll"
#define SETTINGS_CFG	"settings.cfg"

class Bypass {

	private:
		HANDLE		m_hProcess;
		DWORD		m_processId;
		uintptr_t	m_modBaseAddr;
		uintptr_t	m_engineAddr;
		Mutex		m_mutex;
		
		std::map<std::string, std::variant<int, float>>	m_cfg;

	public:
		Bypass();
		virtual ~Bypass();


		/* memory function */
		bool	attach(void) noexcept;
		bool	getModuleBaseAddress(const char* modName) noexcept;

		/* parsing cfg file */
		bool	ParseConfigFile(std::string cfgName);

		/* rpm/wpm function */
		template<typename T>
		inline void		m_writeProcessMemory(uintptr_t lpBaseAddress, T lpBuffer) noexcept;
		
		template<typename T>
		inline T		&m_readProcessMemory(uintptr_t lpBaseAddress) noexcept;


		/* multithreading init */
		void	startMultiThreading(void) noexcept;


		/* cheating function */
		void	m_aimbot(void) noexcept;
		void	m_glow(void) noexcept;
		void	m_trig(void) noexcept;
		void	m_radar(void) noexcept;
		void	m_skinChanger(void) noexcept;
};
