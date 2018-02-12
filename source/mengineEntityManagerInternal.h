#pragma once
#include "interface/mengineEntityManager.h"
#include <vector>

namespace MEngineEntityManager
{
	void Initialize();
	void Shutdown();

	const std::vector<MEngineObject*>& GetEntities();
}