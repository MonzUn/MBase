#pragma once
#include "interface/mengineTypes.h"
#include "interface/mengineComponent.h"
#include <MUtilityByte.h>

namespace MEngineComponentManager
{
	constexpr uint32_t MAX_COMPONENTS = sizeof(MEngineComponentMask) * MUtility::BITS_PER_BYTE;

	void Initialize();
	void Shutdown();

	uint32_t AllocateComponent(MEngineComponentMask componentType, MEngineEntityID owner); // TODODB: See if we can get rid of the owner argument (Required for updated the owner if the component is moved when another component is returned)
	bool ReturnComponent(MEngineComponentMask componentType, uint32_t componentIndex);

	MEngine::Component* GetComponent(MEngineComponentMask componentType, uint32_t componentIndex);
}