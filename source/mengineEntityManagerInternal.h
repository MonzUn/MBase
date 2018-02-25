#pragma once
#include "interface/mengineEntityManager.h"
#include <vector>

namespace MEngineEntityManager
{
	void Initialize();
	void Shutdown();

	void UpdateComponentIndex(MEngineEntityID ID, MEngineComponentMask componentType, uint32_t newComponentIndex);
}