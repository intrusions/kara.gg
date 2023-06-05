#include "Bypass.hpp"
#include "Offset.hpp"
#include "Vector3.hpp"
#include "Utils.hpp"

/* constructor - destructor */
Bypass::Bypass()
	: m_hProcess(nullptr)
	, m_processId(0)
	, m_modBaseAddr(0)
	, m_engineAddr(0)
	, m_mutex() {}

Bypass::~Bypass() {
	if (m_hProcess)
		CloseHandle(m_hProcess);
}


/* memory function */
bool    Bypass::attach() noexcept {
	
	HWND	wHandle = FindWindowA(NULL, WIN_NAME);

	if (wHandle) {
		if (GetWindowThreadProcessId(wHandle, &m_processId)) {
			m_hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_processId);

			if (m_hProcess)
				return (true);
		}
	}
	return (false);
}

bool	Bypass::getModuleBaseAddress(const char* modName) noexcept {

	HANDLE	hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, m_processId);
	if (!hSnap)
		return (false);

	MODULEENTRY32 modEntry;
	modEntry.dwSize = sizeof(modEntry);

	if (Module32First(hSnap, &modEntry)) {
		do {
			if (!_stricmp(modEntry.szModule, modName)) {
				
				if (modName == CLIENT_MODNAME)
					m_modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
				else if (modName == ENGINE_MODNAME)
					m_engineAddr = (uintptr_t)modEntry.modBaseAddr;
				
				break;
			}
		} while (Module32Next(hSnap, &modEntry));
	}
	CloseHandle(hSnap);

	return (m_modBaseAddr ? true : false);
}


/*  parsing config file */
bool	Bypass::ParseConfigFile(std::string cfgName) {

	std::ifstream	cfg(cfgName);
	uint32_t		delimiterPos;

	std::string		key;
	std::string		value;

	std::variant<int, float>	convertedValue;

	if (cfg.is_open()) {

		std::string		line;
		while (std::getline(cfg, line)) {
			
			delimiterPos = line.find('=');
			key = line.substr(0, delimiterPos);
			value = line.substr(delimiterPos + 1);

			char*	end;
			float	floatValue = std::strtof(value.c_str(), &end);

			if (*end == '\0')
				convertedValue = static_cast<int>(floatValue);
			else
				convertedValue = floatValue;

			m_cfg[key] = convertedValue;
		}
		return (true);
	}
	return (false);
}


/* rpm/wpm function */
template<typename T>
inline T		&Bypass::m_readProcessMemory(uintptr_t lpBaseAddress) noexcept {

	T	lpBuffer = { };
	ReadProcessMemory(m_hProcess, reinterpret_cast<const void*>(lpBaseAddress), &lpBuffer, sizeof(T), NULL);
	return (lpBuffer);
}

template<typename T>
inline void		Bypass::m_writeProcessMemory(uintptr_t lpBaseAddress, T lpBuffer) noexcept {
	WriteProcessMemory(m_hProcess, (LPVOID)lpBaseAddress, &lpBuffer, sizeof(lpBuffer), NULL);
}


/* cheating function */
void	Bypass::m_glow() noexcept {

	uintptr_t	player;
	uintptr_t	glowManager;
	uintptr_t	otherPlayer;
	
	uint32_t	pTeam;
	uint32_t	otherPlayerTeam;
	uint32_t	glow;
	const uint8_t	MAXPLAYERS_CFG = std::get<int>(m_cfg["MAXPLAYERS"]);

	while (m_mutex.glowIsActive) {
		std::this_thread::sleep_for(std::chrono::milliseconds(2));

		player = m_readProcessMemory<uintptr_t>(m_modBaseAddr + offsets::dwLocalPlayer);
		glowManager = m_readProcessMemory<uintptr_t>(m_modBaseAddr + offsets::dwGlowObjectManager);
		pTeam = m_readProcessMemory<uint32_t>(player + offsets::m_iTeamNum);

		for (uint8_t i = 0; i < MAXPLAYERS_CFG; i++) {

			otherPlayer = m_readProcessMemory<uintptr_t>(m_modBaseAddr + offsets::dwEntityList + (i * 0x10));

			if (otherPlayer) {

				otherPlayerTeam = m_readProcessMemory<uint32_t>(otherPlayer + offsets::m_iTeamNum);
				glow = m_readProcessMemory<uint32_t>(otherPlayer + offsets::m_iGlowIndex);

				if (otherPlayerTeam != pTeam) {
					m_writeProcessMemory(glowManager + ((glow * 0x38) + 0x8), 1.f);		//r
					m_writeProcessMemory(glowManager + ((glow * 0x38) + 0xC), 0.0f);	//g
					m_writeProcessMemory(glowManager + ((glow * 0x38) + 0x10), 0.0f);	//b
					m_writeProcessMemory(glowManager + ((glow * 0x38) + 0x14), 1.f);	//a
				}
				m_writeProcessMemory(glowManager + ((glow * 0x38) + 0x27), true);
				m_writeProcessMemory(glowManager + ((glow * 0x38) + 0x28), true);
			}
		}
	}
}

void	Bypass::m_trig() noexcept {

	uintptr_t player;
	uintptr_t crosshair;
	uintptr_t playerInCrosshair;
	
	uint16_t playerTeam;
	uint16_t pTeam;
	const uint8_t	MAXPLAYERS_CFG = std::get<int>(m_cfg["MAXPLAYERS"]);
	const uint8_t	K_TRIG_CFG = std::get<int>(m_cfg["K_TRIGLOCK"]);


	while (m_mutex.trigIsActive) {
		if (GetAsyncKeyState(K_TRIG_CFG)) {
			std::this_thread::sleep_for(std::chrono::milliseconds(2));

			player = m_readProcessMemory<uintptr_t>(m_modBaseAddr + offsets::dwLocalPlayer);
			crosshair = m_readProcessMemory<uintptr_t>(player + offsets::m_iCrosshairId);

			if (crosshair && crosshair < MAXPLAYERS_CFG) {

				playerInCrosshair = m_readProcessMemory<uintptr_t>(m_modBaseAddr + offsets::dwEntityList + (crosshair - 1) * 16);

				if (playerInCrosshair) {

					playerTeam = m_readProcessMemory<uint16_t>(playerInCrosshair + offsets::m_iTeamNum);
					pTeam = m_readProcessMemory<uint16_t>(player + offsets::m_iTeamNum);

					if (playerTeam != pTeam) {
						m_writeProcessMemory(m_modBaseAddr + offsets::dwForceAttack, 5);
						std::this_thread::sleep_for(std::chrono::milliseconds(12));
						m_writeProcessMemory(m_modBaseAddr + offsets::dwForceAttack, 4);
					}
				}
			}
		}
	}
}

void	Bypass::m_radar() noexcept {

	uintptr_t	entity;
	uintptr_t	player;
	
	uint32_t	pTeam;
	uint32_t	enemyTeam;

	const uint8_t	MAXPLAYERS_CFG = std::get<int>(m_cfg["MAXPLAYERS"]);

	while (m_mutex.radarIsActive) {
		std::this_thread::sleep_for(std::chrono::milliseconds(2));

		player = m_readProcessMemory<uintptr_t>(m_modBaseAddr + offsets::dwLocalPlayer);
		pTeam = m_readProcessMemory<uint32_t>(player + offsets::m_iTeamNum);

		for (uint8_t i = 0; i < MAXPLAYERS_CFG; i++) {

			entity = m_readProcessMemory<uintptr_t>(m_modBaseAddr + offsets::dwEntityList + (i * 0x10));
			enemyTeam = m_readProcessMemory<uint32_t>(entity + offsets::m_iTeamNum);

			if (enemyTeam == pTeam)
				continue;

			m_writeProcessMemory(entity + offsets::m_bSpotted, true);
		}
	}
}

void	Bypass::m_aimbot() noexcept {

	uintptr_t	lPlayer;
	uintptr_t	clientState;
	uintptr_t	player;
	uintptr_t	boneMatrix;
	
	int32_t			pTeam;
	int32_t			lPlayerId;
	const uint8_t	MAXPLAYERS_CFG = std::get<int>(m_cfg["MAXPLAYERS"]);
	const uint8_t	K_AIMBOT_CFG = std::get<int>(m_cfg["K_AIMLOCK"]);
	
	float			fov;
	float			bestFov;
	const float		AIMBOT_SMOOTH_CFG = std::get<float>(m_cfg["AIMBOT_SMOOTH"]);

	Vector3		localEyePosition;
	Vector3		viewAngles;
	Vector3		aimPunch;
	Vector3		bestAngle;
	Vector3		playerHeadPosition;
	Vector3		angle;

	while (m_mutex.aimbotIsActive) {
		std::this_thread::sleep_for(std::chrono::milliseconds(2));

		if (!GetAsyncKeyState(K_AIMBOT_CFG))
			continue;

		lPlayer = m_readProcessMemory<uintptr_t>(m_modBaseAddr + offsets::dwLocalPlayer);
		pTeam = m_readProcessMemory<int32_t>(lPlayer + offsets::m_iTeamNum);

		localEyePosition = m_readProcessMemory<Vector3>(lPlayer + offsets::m_vecOrigin)
							+ m_readProcessMemory<Vector3>(lPlayer + offsets::m_vecViewOffset);

		clientState = m_readProcessMemory<uintptr_t>(m_engineAddr + offsets::dwClientState);

		lPlayerId = m_readProcessMemory<int32_t>(clientState + offsets::dwClientState_GetLocalPlayer);

		viewAngles = m_readProcessMemory<Vector3>(clientState + offsets::dwClientState_ViewAngles);
		
		aimPunch = m_readProcessMemory<Vector3>(lPlayer + offsets::m_aimPunchAngle) * 2;

		bestFov = std::get<float>(m_cfg["AIMBOT_FOV"]);
		bestAngle = Vector3{ };

		for (uint8_t i = 0; i <= MAXPLAYERS_CFG; i++) {

			player = m_readProcessMemory<uintptr_t>(m_modBaseAddr + offsets::dwEntityList + i * 0x10);

			if (m_readProcessMemory<int32_t>(player + offsets::m_iTeamNum) == pTeam)
				continue;

			if (m_readProcessMemory<bool>(player + offsets::m_bDormant))
				continue;

			if (!m_readProcessMemory<int32_t>(player + offsets::m_iHealth))
				continue;

			if (m_readProcessMemory<int32_t>(player + offsets::m_bSpottedByMask) & (1 << lPlayerId)) {

				boneMatrix = m_readProcessMemory<uintptr_t>(player + offsets::m_dwBoneMatrix);

				playerHeadPosition = Vector3{
					m_readProcessMemory<float>(boneMatrix + 0x30 * 8 + 0x0C),
					m_readProcessMemory<float>(boneMatrix + 0x30 * 8 + 0x1C),
					m_readProcessMemory<float>(boneMatrix + 0x30 * 8 + 0x2C)
				};

				angle = CalculateAngle(
					localEyePosition,
					playerHeadPosition,
					viewAngles + aimPunch
				);

				fov = std::hypot(angle.x, angle.y);

				if (fov < bestFov) {
					bestFov = fov;
					bestAngle = angle;
				}
			}
		}
		if (!bestAngle.IsZero()) {
			m_writeProcessMemory(clientState + offsets::dwClientState_ViewAngles, viewAngles + bestAngle / AIMBOT_SMOOTH_CFG);
		}
	}
}

void	Bypass::m_skinChanger() noexcept {
	
	uintptr_t						lPlayer;
	uintptr_t						weapon;
	std::array<unsigned long, 8>	weapons;
	bool							shouldUpdate;
	short							paint;

	while (true) {
		
		lPlayer = m_readProcessMemory<uintptr_t>(m_modBaseAddr + offsets::dwLocalPlayer);
		weapons = m_readProcessMemory<std::array<unsigned long, 8>>(lPlayer + offsets::m_hMyWeapons);
		
		for (auto &handle : weapons) {
		
			weapon = m_readProcessMemory<uintptr_t>((m_modBaseAddr + offsets::dwEntityList + (handle & 0xFFF) * 0x10) - 0x10);
			
			if (!weapon)
				continue;

			if (paint = getWeaponPaint(m_readProcessMemory<short>(weapon + offsets::m_iItemDefinitionIndex))) {

				shouldUpdate = m_readProcessMemory<int32_t>(weapon + offsets::m_nFallbackPaintKit) != paint;

				m_writeProcessMemory<int32_t>(weapon + offsets::m_iItemIDHigh, -1);
				
				m_writeProcessMemory<int32_t>(weapon + offsets::m_nFallbackPaintKit, paint);
				m_writeProcessMemory<float>(weapon + offsets::m_flFallbackWear, 0.1f);

				if (shouldUpdate)
					m_writeProcessMemory<int32_t>(m_readProcessMemory<uintptr_t>(m_engineAddr + offsets::dwClientState) + 0x174, -1);
			}
		}
	}
}


/* main function */
void	Bypass::startMultiThreading() noexcept {
	
	bool	refreshMenu;

	printMenu(&m_mutex);

	/* skins changer part's */
	std::thread skinChangerThread(&Bypass::m_skinChanger, this);
	skinChangerThread.detach();

	while (true) {

		refreshMenu = false;

		/* Aimbot part's */
		if (GetAsyncKeyState(std::get<int>(m_cfg["K_AIMBOT"])) & 0x8000) {
			refreshMenu = true;
			m_mutex.aimbotIsActive = !m_mutex.aimbotIsActive;

			if (m_mutex.aimbotIsActive) {
				m_mutex.aimbotIsActive = true;
				std::thread aimbotThread(&Bypass::m_aimbot, this);
				aimbotThread.detach();
			}
		}

		/* Trigger part's */
		if (GetAsyncKeyState(std::get<int>(m_cfg["K_TRIG"])) & 0x8000) {
			refreshMenu = true;
			m_mutex.trigIsActive = !m_mutex.trigIsActive;

			if (m_mutex.trigIsActive) {
				m_mutex.trigIsActive = true;
				std::thread triggerThread(&Bypass::m_trig, this);
				triggerThread.detach();
			}
		}

		/* Radar hack part's */
		if (GetAsyncKeyState(std::get<int>(m_cfg["K_RADAR"])) & 0x8000) {
				refreshMenu = true;
				m_mutex.radarIsActive = !m_mutex.radarIsActive;

			if (m_mutex.radarIsActive) {
				m_mutex.radarIsActive = true;
				std::thread radarThread(&Bypass::m_radar, this);
				radarThread.detach();
			}
		}

		/* Glow part's */
		if (GetAsyncKeyState(std::get<int>(m_cfg["K_GLOW"])) & 0x8000) {
			refreshMenu = true;
			m_mutex.glowIsActive = !m_mutex.glowIsActive;

			if (m_mutex.glowIsActive) {
				m_mutex.glowIsActive = true;
				std::thread glowThread(&Bypass::m_glow, this);
				glowThread.detach();
			} 
		}

		/* menu part's */
		if (refreshMenu)
			printMenu(&m_mutex);

		Sleep(200);
	}
}
