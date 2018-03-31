#pragma once
#include "Interface/Mengine.h"

namespace MEngineUtility
{
	void Initialize(const char* applicationName, MEngine::InitFlags initFlags);
	void Shutdown();
	void Update();
}