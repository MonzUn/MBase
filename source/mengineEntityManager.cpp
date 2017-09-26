#include "interface/mengineEntityManager.h"
#include "mengineEntityManagerInternal.h"
#include "interface/mengineLog.h"
#include "interface/mengineObject.h"

using namespace MEngineEntityManager;

std::vector<MEngineObject*> Entities;
std::vector<MEngineEntityID> RecycledIDs;

#define MENGINE_LOG_CATEGORY_ENTITY_MANAGER "MEngineEntityManager"

// Forward declaration
MEngineEntityID GetNextEntityID();

MEngineEntityID MEngineEntityManager::RegisterNewEntity(MEngineObject* entity)
{
	MEngineEntityID ID = GetNextEntityID();
	entity->EntityID = ID;

	Entities.push_back(entity);
	return ID;
}

void MEngineEntityManager::DestroyEntity(MEngineEntityID entityID)
{
	MEngineObject* object = Entities[entityID];
	if (object != nullptr)
	{
		delete object;
		object = nullptr;
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
			MLOG_WARNING("Attempted to destroy entity with ID " << entityID << " but the entity with that ID has already been destroyed", MENGINE_LOG_CATEGORY_ENTITY_MANAGER);
		else
			MLOG_WARNING("Attempted to destroy entity with ID " << entityID << " but no entity with that ID exists", MENGINE_LOG_CATEGORY_ENTITY_MANAGER);
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