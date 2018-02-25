#pragma once
#include "mengineTypes.h"
#include "mengineComponent.h"
#include <stdint.h>

namespace MEngineComponentManager // TODODB: Make thread safe
{
	MEngineComponentMask RegisterComponentType(const MEngine::Component& templateComponent, uint32_t templateComponentSize, uint32_t maxCount, const char* componentName);
	bool UnregisterComponentType(MEngineComponentMask componentType);

	MUtility::Byte* GetComponentBuffer(MEngineComponentMask componentType, int32_t& outComponentCount);
}