#pragma once
#include "MEngineSystem.h"

namespace MEngine
{
	constexpr float MENGINE_TIME_STEP_FPS_120	= 0.00833333f;
	constexpr float MENGINE_TIME_STEP_FPS_60	= 0.01666667f;
	constexpr float MENGINE_TIME_STEP_FPS_30	= 0.03333333f;
	constexpr float MENGINE_TIME_STEP_FPS_15	= 0.06666666f;

	constexpr uint32_t MENGINE_MIN_SYSTEM_PRIORITY = 100;
	constexpr uint32_t MENGINE_MAX_SYSTEM_PRIORITY = 10000;

	SystemID RegisterSystem(System* system);
	bool UnregisterSystem(SystemID ID);

	GameModeID CreateGameMode(); // TODODB: Make a function for removing game a game mode

	bool AddSystemToGameMode(GameModeID gameModeID, SystemID systemID, uint32_t priority); // TODODB Make a function for removing a system from a game mode
	bool RequestGameModeChange(GameModeID newGameModeID); // The requested game mode will be activated at the start of next frame
}