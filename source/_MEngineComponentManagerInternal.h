#pragma once
#include "Interface/MEngineTypes.h"
#include "Interface/MEngineComponent.h"
#include <MUtilityByte.h>

namespace MEngineComponentManager
{
	constexpr uint32_t MAX_COMPONENTS = sizeof(MEngine::ComponentMask) * MUtility::BITS_PER_BYTE;

	void Initialize();
	void Shutdown();

	uint32_t AllocateComponent(MEngine::ComponentMask componentType, MEngine::EntityID owner); // TODODB: See if we can get rid of the owner argument (Required for updated the owner if the component is moved when another component is returned)
	bool ReturnComponent(MEngine::ComponentMask componentType, uint32_t componentIndex);

	MEngine::Component* GetComponent(MEngine::ComponentMask componentType, uint32_t componentIndex);
}