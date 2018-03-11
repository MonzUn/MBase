#pragma once
#include "Interface/MEngineSystemManager.h"
#include "Interface/MEngineSystem.h"

namespace MEngineSystemManager
{
	constexpr float DEFAULT_TIME_STEP			= MEngine::MENGINE_TIME_STEP_FPS_15;
	constexpr float DEFAULT_SIMULATION_SPEED	= MEngine::MENGINE_TIME_STEP_FPS_15;

	void Initialize();
	void Shutdown();
	void Update();
}