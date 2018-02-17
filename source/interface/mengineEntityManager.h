#pragma once
#include <MUtilityTypes.h>
#include <stdint.h>

#define INVALID_MENGINE_ENTITY_ID -1

class MEngineObject;

namespace MEngineEntityManager
{
	typedef MUtilityID MEngineEntityID;

	MEngineEntityID RegisterNewEntity(MEngineObject* entity);
	void DestroyEntity(MEngineEntityID entityID);
}