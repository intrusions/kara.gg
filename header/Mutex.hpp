#pragma once
#include <atomic>

struct Mutex {
	std::atomic<bool> trigIsActive { false };
	std::atomic<bool> radarIsActive { false };
	std::atomic<bool> aimbotIsActive { false };
	std::atomic<bool> glowIsActive { false };
};