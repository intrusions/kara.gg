#include "Bypass.hpp"
#include "Offset.hpp"
#include "Vector3.hpp"
#include "Utils.hpp"

/* constructor - destructor */
Bypass::Bypass()
	: m_hProcess(nullptr)
	, m_processId(0)
	, m_modBaseAddr(0)
	, m_engineAddr(0) {}

Bypass::~Bypass() {
	if (m_hProcess)
		CloseHandle(m_hProcess);
}


/* memory function */
bool    Bypass::attach() {
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

bool	Bypass::getModuleBaseAddress(const char* modName)
{
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

	if (m_modBaseAddr)
		return (true);
	return (false);
}


/* rpm/wpm function */
template<typename T>
T		&Bypass::m_readProcessMemory(uintptr_t lpBaseAddress) {

	T	lpBuffer = { };
	ReadProcessMemory(m_hProcess, reinterpret_cast<const void*>(lpBaseAddress), &lpBuffer, sizeof(T), NULL);
	return (lpBuffer);
}

template<typename T>
void	Bypass::m_writeProcessMemory(uintptr_t lpBaseAddress, T lpBuffer) {
	WriteProcessMemory(m_hProcess, (LPVOID)lpBaseAddress, &lpBuffer, sizeof(lpBuffer), NULL);
}


/* cheating function */
void	Bypass::m_glow() {
	while (m_mutex.glowIsActive) {

		uintptr_t	player = m_readProcessMemory<uintptr_t>(m_modBaseAddr + offsets::dwLocalPlayer);
		uintptr_t	glowManager = m_readProcessMemory<uintptr_t>(m_modBaseAddr + offsets::dwGlowObjectManager);
		uint32_t	pTeam = m_readProcessMemory<uint32_t>(player + offsets::m_iTeamNum);

		for (uint8_t i = 0; i < MAXPLAYERS; i++) {

			uintptr_t	otherPlayer = m_readProcessMemory<uintptr_t>(m_modBaseAddr + offsets::dwEntityList + (i * 0x10));

			if (otherPlayer) {

				uint32_t	otherPlayerTeam = m_readProcessMemory<uint32_t>(otherPlayer + offsets::m_iTeamNum);
				uint32_t	glow = m_readProcessMemory<uint32_t>(otherPlayer + offsets::m_iGlowIndex);

				if (otherPlayerTeam != pTeam) {
					m_writeProcessMemory(glowManager + ((glow * 0x38) + 0x8), 1.f);
					m_writeProcessMemory(glowManager + ((glow * 0x38) + 0xC), 0.0f);
					m_writeProcessMemory(glowManager + ((glow * 0x38) + 0x10), 0.0f);
					m_writeProcessMemory(glowManager + ((glow * 0x38) + 0x14), 1.f);
				}
				m_writeProcessMemory(glowManager + ((glow * 0x38) + 0x27), true);
				m_writeProcessMemory(glowManager + ((glow * 0x38) + 0x28), true);
			}
		}
	}
}

void	Bypass::m_trig() {
	while (m_mutex.trigIsActive) {
		if (GetAsyncKeyState(K_TRIGLOCK)) {

			uintptr_t player = m_readProcessMemory<uintptr_t>(m_modBaseAddr + offsets::dwLocalPlayer);
			uintptr_t crosshair = m_readProcessMemory<uintptr_t>(player + offsets::m_iCrosshairId);

			if (crosshair && crosshair < MAXPLAYERS) {

				uintptr_t playerInCrosshair = m_readProcessMemory<uintptr_t>(m_modBaseAddr + offsets::dwEntityList + (crosshair - 1) * 16);

				if (playerInCrosshair) {

					uint16_t playerTeam = m_readProcessMemory<uint16_t>(playerInCrosshair + offsets::m_iTeamNum);
					uint16_t pTeam = m_readProcessMemory<uint16_t>(player + offsets::m_iTeamNum);

					if (playerTeam != pTeam) {
						m_writeProcessMemory(m_modBaseAddr + offsets::dwForceAttack, 5);
						Sleep(12);
						m_writeProcessMemory(m_modBaseAddr + offsets::dwForceAttack, 4);
					}
				}
			}
		}
	}
}

void	Bypass::m_radar() {
	while (m_mutex.radarIsActive) {

		uintptr_t	player = m_readProcessMemory<uintptr_t>(m_modBaseAddr + offsets::dwLocalPlayer);
		uint32_t	pTeam = m_readProcessMemory<uint32_t>(player + offsets::m_iTeamNum);

		for (uint8_t i = 0; i < MAXPLAYERS; i++) {

			uintptr_t	entity = m_readProcessMemory<uintptr_t>(m_modBaseAddr + offsets::dwEntityList + (i * 0x10));
			uint32_t	enemyTeam = m_readProcessMemory<uint32_t>(entity + offsets::m_iTeamNum);

			if (enemyTeam == pTeam)
				continue;

			m_writeProcessMemory(entity + offsets::m_bSpotted, true);
		}
	}
}

void	Bypass::m_aimbot() {
	while (m_mutex.aimbotIsActive) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		if (!GetAsyncKeyState(K_AIMLOCK))
			continue;

		const uintptr_t	&lPlayer = m_readProcessMemory<uintptr_t>(m_modBaseAddr + offsets::dwLocalPlayer);
		const int32_t	&pTeam = m_readProcessMemory<int32_t>(lPlayer + offsets::m_iTeamNum);

		const Vector3	localEyePosition = m_readProcessMemory<Vector3>(lPlayer + offsets::m_vecOrigin)
							+ m_readProcessMemory<Vector3>(lPlayer + offsets::m_vecViewOffset);

		const uintptr_t	&clientState = m_readProcessMemory<uintptr_t>(m_engineAddr + offsets::dwClientState);

		const int32_t	&lPlayerId = m_readProcessMemory<int32_t>(clientState + offsets::dwClientState_GetLocalPlayer);

		const Vector3	&viewAngles = m_readProcessMemory<Vector3>(clientState + offsets::dwClientState_ViewAngles);
		
		const Vector3	&aimPunch = m_readProcessMemory<Vector3>(lPlayer + offsets::m_aimPunchAngle) * 2;

		float	bestFov = AIMBOT_FOV;
		Vector3	bestAngle = Vector3{ };

		for (uint8_t i = 1; i <= MAXPLAYERS; i++) {

			const uintptr_t player = m_readProcessMemory<uintptr_t>(m_modBaseAddr + offsets::dwEntityList + i * 0x10);

			if (m_readProcessMemory<int32_t>(player + offsets::m_iTeamNum) == pTeam)
				continue;

			if (m_readProcessMemory<bool>(player + offsets::m_bDormant))
				continue;

			if (!m_readProcessMemory<int32_t>(player + offsets::m_iHealth))
				continue;

			if (m_readProcessMemory<int32_t>(player + offsets::m_bSpottedByMask) & (1 << lPlayerId)) {

				const uintptr_t boneMatrix = m_readProcessMemory<uintptr_t>(player + offsets::m_dwBoneMatrix);

				const Vector3 playerHeadPosition = Vector3{
					m_readProcessMemory<float>(boneMatrix + 0x30 * 8 + 0x0C),
					m_readProcessMemory<float>(boneMatrix + 0x30 * 8 + 0x1C),
					m_readProcessMemory<float>(boneMatrix + 0x30 * 8 + 0x2C)
				};

				const Vector3 angle = CalculateAngle(
					localEyePosition,
					playerHeadPosition,
					viewAngles + aimPunch
				);

				const float fov = std::hypot(angle.x, angle.y);

				if (fov < bestFov) {
					bestFov = fov;
					bestAngle = angle;
				}
			}
		}
		if (!bestAngle.IsZero()) {
			m_writeProcessMemory(clientState + offsets::dwClientState_ViewAngles, viewAngles + bestAngle / AIMBOT_SMOOTH);
		}
	}
}

void	Bypass::m_skinChanger() {
	
	while (true) {
		uintptr_t	lPlayer = m_readProcessMemory<uintptr_t>(m_modBaseAddr + offsets::dwLocalPlayer);
		auto		weapons = m_readProcessMemory<std::array<unsigned long, 8>>(lPlayer + offsets::m_hMyWeapons);
		
		for (auto &handle : weapons) {
		
			uintptr_t weapon = m_readProcessMemory<uintptr_t>((m_modBaseAddr + offsets::dwEntityList + (handle & 0xFFF) * 0x10) - 0x10);
			
			if (!weapon)
				continue;

			if (short paint = getWeaponPaint(m_readProcessMemory<short>(weapon + offsets::m_iItemDefinitionIndex))) {

				bool shouldUpdate = m_readProcessMemory<int32_t>(weapon + offsets::m_nFallbackPaintKit) != paint;

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
void	Bypass::startMultiThreading() {
	
	bool	refreshMenu;

	printMenu(&m_mutex);

	/* skins changer part's */
	std::thread skinChangerThread(&Bypass::m_skinChanger, this);
	skinChangerThread.detach();

	while (true) {

		refreshMenu = false;

		/* Aimbot part's */
		if (GetAsyncKeyState(K_AIMBOT) & 0x8000) {
			refreshMenu = true;
			m_mutex.aimbotIsActive = !m_mutex.aimbotIsActive;

			if (m_mutex.aimbotIsActive) {
				m_mutex.aimbotIsActive = true;
				std::thread aimbotThread(&Bypass::m_aimbot, this);
				aimbotThread.detach();
			}
		}

		/* Trigger part's */
		if (GetAsyncKeyState(K_TRIG) & 0x8000) {
			refreshMenu = true;
			m_mutex.trigIsActive = !m_mutex.trigIsActive;

			if (m_mutex.trigIsActive) {
				m_mutex.trigIsActive = true;
				std::thread triggerThread(&Bypass::m_trig, this);
				triggerThread.detach();
			}
		}

		/* Radar hack part's */
		if (GetAsyncKeyState(K_RADAR) & 0x8000) {
				refreshMenu = true;
				m_mutex.radarIsActive = !m_mutex.radarIsActive;

			if (m_mutex.radarIsActive) {
				m_mutex.radarIsActive = true;
				std::thread radarThread(&Bypass::m_radar, this);
				radarThread.detach();
			}
		}

		/* Glow part's */
		if (GetAsyncKeyState(K_GLOW) & 0x8000) {
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
