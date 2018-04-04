#pragma once
#include "MEngineTypes.h"
#include "MEngineComponent.h"
#include <MUtilityTypes.h>
#include <stdint.h>
#include <vector>

namespace MEngine
{
	// TODODB: Fix stack/heap corruption caused by entities being created but never removed
	enum class MaskMatchMode
	{
		Exact,		// The mask must contain only the components described by the comparison mask
		Partial,	// The mask must have the components described by the comparison mask but may have additional components as well
		Any,		// The mask must have at least one of the components described in the comparison mask
	};

	EntityID CreateEntity();
	bool DestroyEntity(EntityID entityID);

	ComponentMask AddComponentsToEntity(EntityID ID, ComponentMask componentMask); // Returns a bitmask containing all component types that could not be added to the entity
	ComponentMask RemoveComponentsFromEntity(EntityID ID, ComponentMask componentMask); // Returns a bitmask containing all component types that could not be removed from the entity

	void GetEntitiesMatchingMask(ComponentMask componentMask, std::vector<EntityID>& outEntities, MaskMatchMode matchMode = MaskMatchMode::Partial);

	MEngine::Component* GetComponent(EntityID ID, ComponentMask componentMask);
	ComponentMask GetComponentMask(EntityID ID);

	bool IsEntityIDValid(EntityID ID);
}