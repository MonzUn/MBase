#pragma once
#include "mengineTypes.h"
#include "mengineComponent.h"
#include <MUtilityTypes.h>
#include <stdint.h>
#include <vector>

namespace MEngine
{
	EntityID CreateEntity();
	bool DestroyEntity(EntityID entityID);

	ComponentMask AddComponentsToEntity(ComponentMask componentMask, EntityID entityID); // Returns a bitmask containing all component types that could not be added to the entity
	ComponentMask RemoveComponentsFromEntity(ComponentMask componentMask, EntityID entityID); // Returns a bitmask containing all component types that could not be removed from the entity

	void GetEntitiesMatchingMask(ComponentMask componentMask, std::vector<EntityID>& outEntities, bool requireFullMatch = false);

	MEngine::Component* GetComponentForEntity(ComponentMask componentMask, EntityID entityID);
}