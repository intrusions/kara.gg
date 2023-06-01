#include "Bypass.hpp"
#include "Offset.hpp"
#include "Vector3.hpp"
#include "Utils.hpp"

/* constructor - destructor */
Bypass::Bypass() {}

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
void	Bypass::m_glow(std::atomic<bool> &isActive) {
	while (isActive) {

		uintptr_t	player = m_readProcessMemory<uintptr_t>(m_modBaseAddr + offsets::dwLocalPlayer);
		uintptr_t	glowManager = m_readProcessMemory<uintptr_t>(m_modBaseAddr + offsets::dwGlowObjectManager);
		uint32_t	pTeam = m_readProcessMemory<uint32_t>(player + offsets::m_iTeamNum);

		for (uint8_t i = 0; i < MAXPLAYERS; i++) {

			uintptr_t	otherPlayer = m_readProcessMemory<uintptr_t>(m_modBaseAddr + offsets::dwEntityList + (i * 0x10));

			if (otherPlayer) {

				uint32_t	otherPlayerTeam = m_readProcessMemory<uint32_t>(otherPlayer + offsets::m_iTeamNum);
				uint32_t	glow = m_readProcessMemory<uint32_t>(otherPlayer + offsets::m_iGlowIndex);

				if (otherPlayerTeam != pTeam) {
					m_writeProcessMemory(glowManager + ((glow * 0x38) + 4), 2.0f);
					m_writeProcessMemory(glowManager + ((glow * 0x38) + 8), 0.0f);
					m_writeProcessMemory(glowManager + ((glow * 0x38) + 12), 0.0f);
					m_writeProcessMemory(glowManager + ((glow * 0x38) + 16), 0.5f);
				}
				m_writeProcessMemory(glowManager + ((glow * 0x38) + 0x24), true);
				m_writeProcessMemory(glowManager + ((glow * 0x38) + 0x25), false);
			}
		}
	}
}

void	Bypass::m_trig(std::atomic<bool> &isActive) {
	while (isActive) {

		if (GetAsyncKeyState(0x43)) {

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

void	Bypass::m_radar(std::atomic<bool> &isActive) {
	while (isActive) {

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

void	Bypass::m_aimbot(std::atomic<bool> &isActive) {
	while (isActive) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		if (!GetAsyncKeyState(VK_LBUTTON))
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
			m_writeProcessMemory(clientState + offsets::dwClientState_ViewAngles, viewAngles + bestAngle / 3.f);
		}
	}
}


/* main function */
void	Bypass::startMultiThreading(void) {
	
	std::atomic<bool> trigIsActive(false);
	std::atomic<bool> radarIsActive(false);
	std::atomic<bool> aimbotIsActive(false);
	std::atomic<bool> glowIsActive(false);

	bool	refreshMenu;

	printCheatIsActive(aimbotIsActive, glowIsActive, trigIsActive, radarIsActive);

	while (true) {
		
		refreshMenu = 0;

		/* Aimbot part's */
		if (GetAsyncKeyState(K_AIMBOT) & 0x8000) {

			if (!aimbotIsActive) {
				aimbotIsActive = true;
				std::thread aimbotThread(&Bypass::m_aimbot, this, std::ref(aimbotIsActive));
				aimbotThread.detach();
			} else {
				aimbotIsActive = false;
			}
			refreshMenu = 1;
		}

		/* Trigger part's */
		if (GetAsyncKeyState(K_TRIG) & 0x8000) {

			if (!trigIsActive) {
				
				trigIsActive = true;
				std::thread triggerThread(&Bypass::m_trig, this, std::ref(trigIsActive));
				triggerThread.detach();
			} else {
				trigIsActive = false;
			}
			refreshMenu = 1;
		}

		/* Radar hack part's */
		if (GetAsyncKeyState(K_RADAR) & 0x8000) {

			if (!radarIsActive) {

				radarIsActive = true;
				std::thread radarThread(&Bypass::m_radar, this, std::ref(radarIsActive));
				radarThread.detach();
			} else {
				radarIsActive = false;
			}
			refreshMenu = 1;
		}

		/* Glow ESP part's */
		if (GetAsyncKeyState(K_GLOW) & 0x8000) {

			if (!glowIsActive) {

				glowIsActive = true;
				std::thread glowThread(&Bypass::m_glow, this, std::ref(glowIsActive));
				glowThread.detach();
			} else {
				glowIsActive = false;
			}
			refreshMenu = 1;
		}

		/* menu part's */
		if (refreshMenu)
			printCheatIsActive(aimbotIsActive, glowIsActive, trigIsActive, radarIsActive);

		Sleep(200);
	}
}