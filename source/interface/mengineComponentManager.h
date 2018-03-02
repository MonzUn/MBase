#pragma once
#include "mengineTypes.h"
#include "mengineComponent.h"
#include <stdint.h>

namespace MEngine // TODODB: Make thread safe
{
	ComponentMask RegisterComponentType(const MEngine::Component& templateComponent, uint32_t templateComponentSize, uint32_t maxCount, const char* componentName);
	bool UnregisterComponentType(ComponentMask componentType);

	MUtility::Byte* GetComponentBuffer(ComponentMask componentType, int32_t* outComponentCount = nullptr);
}