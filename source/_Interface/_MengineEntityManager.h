#pragma once
#include "MEngineTypes.h"
#include "MEngineComponent.h"
#include <MUtilityTypes.h>
#include <stdint.h>
#include <vector>

namespace MEngine
{
	// TODODB: Remove "entity" from the getter names (Always need an entityID anyway)
	enum class MaskMatchMode
	{
		Exact,		// The mask must contain only the components described by the comparison mask
		Partial,	// The mask must have the components described by the comparison mask but may have additional components as well
		Any,		// The mask must have at least one of the components described in the comparison mask
	};

	EntityID CreateEntity();
	bool DestroyEntity(EntityID entityID);

	ComponentMask AddComponentsToEntity(ComponentMask componentMask, EntityID entityID); // Returns a bitmask containing all component types that could not be added to the entity
	ComponentMask RemoveComponentsFromEntity(ComponentMask componentMask, EntityID entityID); // Returns a bitmask containing all component types that could not be removed from the entity

	void GetEntitiesMatchingMask(ComponentMask componentMask, std::vector<EntityID>& outEntities, MaskMatchMode matchMode = MaskMatchMode::Partial);

	MEngine::Component* GetComponentForEntity(ComponentMask componentMask, EntityID entityID);
	ComponentMask GetComponentMask(EntityID ID);
}