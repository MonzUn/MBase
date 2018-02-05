#pragma once
#include "interface/mengineSystem.h"

namespace MEngineSystem
{
	constexpr float DEFAULT_TIME_STEP			= MENGINE_TIME_STEP_FPS_15;
	constexpr float DEFAULT_SIMULATION_SPEED	= MENGINE_TIME_STEP_FPS_15;

	void Update();
	void Shutdown();
}