#pragma once
#include "interface/mengineSystemManager.h"
#include "interface/mengineSystem.h"

namespace MEngineSystemManager
{
	constexpr float DEFAULT_TIME_STEP			= MEngine::MENGINE_TIME_STEP_FPS_15;
	constexpr float DEFAULT_SIMULATION_SPEED	= MEngine::MENGINE_TIME_STEP_FPS_15;

	void Initialize();
	void Shutdown();
	void Update();
}