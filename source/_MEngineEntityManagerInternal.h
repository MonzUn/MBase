#pragma once
#include "Interface/MEngineEntityManager.h"
#include <vector>

namespace MEngineEntityManager
{
	void Initialize();
	void Shutdown();

	void UpdateComponentIndex(MEngine::EntityID ID, MEngine::ComponentMask componentType, uint32_t newComponentIndex);
}