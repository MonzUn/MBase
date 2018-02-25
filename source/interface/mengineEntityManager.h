#pragma once
#include "mengineTypes.h"
#include "mengineComponent.h"
#include <MUtilityTypes.h>
#include <stdint.h>

namespace MEngineEntityManager
{
	MEngineEntityID CreateEntity();
	bool DestroyEntity(MEngineEntityID entityID);

	MEngineComponentMask AddComponentsToEntity(MEngineComponentMask componentMask, MEngineEntityID entityID); // Returns a bitmask containing all component types that could not be added to the entity
	MEngineComponentMask RemoveComponentsFromEntity(MEngineComponentMask componentMask, MEngineEntityID entityID); // Returns a bitmask containing all component types that could not be removed from the entity

	MEngine::Component* GetComponentForEntity(MEngineComponentMask componentMask, MEngineEntityID entityID);
}