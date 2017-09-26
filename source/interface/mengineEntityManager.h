#pragma once
#include <stdint.h>

#define INVALID_MENGINE_ENTITY_ID -1

class MEngineObject;

namespace MEngineEntityManager
{
	typedef int32_t MEngineEntityID;

	MEngineEntityID RegisterNewEntity(MEngineObject* entity);
	void DestroyEntity(MEngineEntityID entityID);
}