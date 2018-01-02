#include "interface/mengineEntityManager.h"
#include "interface/mengineObject.h"
#include "mengineEntityManagerInternal.h"
#include <MUtilityLog.h>

using namespace MEngineEntityManager;

namespace MEngineEntityManager
{
	std::vector<MEngineObject*> Entities;
	std::vector<MEngineEntityID> RecycledIDs;
}

#define MUTILITY_LOG_CATEGORY_ENTITY_MANAGER "MEngineEntityManager"

// Forward declaration
MEngineEntityID GetNextEntityID();

MEngineEntityID MEngineEntityManager::RegisterNewEntity(MEngineObject* entity)
{
	MEngineEntityID ID = GetNextEntityID();
	entity->EntityID = ID;

	if (ID < Entities.size())
		Entities[ID] = entity;
	else
		Entities.push_back(entity);
	return ID;
}

void MEngineEntityManager::DestroyEntity(MEngineEntityID entityID)
{
	MEngineObject* object = Entities[entityID];
	if (object != nullptr)
	{
		delete object;
		Entities[entityID] = nullptr;
		RecycledIDs.push_back(entityID);
	}
	else
	{
		bool isRecycled = false;
		for (int i = 0; i < RecycledIDs.size(); ++i)
		{
			if (RecycledIDs[i] == entityID)
			{
				isRecycled = true;
				break;
			}
		}

		if (isRecycled)
			MLOG_WARNING("Attempted to destroy entity with ID " << entityID << " but the entity with that ID has already been destroyed", MUTILITY_LOG_CATEGORY_ENTITY_MANAGER);
		else
			MLOG_WARNING("Attempted to destroy entity with ID " << entityID << " but no entity with that ID exists", MUTILITY_LOG_CATEGORY_ENTITY_MANAGER);
	}
}

const std::vector<MEngineObject*>& MEngineEntityManager::GetEntities()
{
	return Entities;
}

MEngineEntityID GetNextEntityID()
{
	static MEngineEntityID nextID = 0;

	MEngineEntityID recycledID = -1;
	if (RecycledIDs.size() > 0)
	{
		recycledID = RecycledIDs.back();
		RecycledIDs.pop_back();
	}

	return recycledID >= 0 ? recycledID : nextID++;
}