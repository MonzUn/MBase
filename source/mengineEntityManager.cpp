#include "interface/mengineEntityManager.h"
#include "interface/mengineObject.h"
#include "mengineEntityManagerInternal.h"
#include <MUtilityIDBank.h>
#include <MUtilityLog.h>

using namespace MEngineEntityManager;

namespace MEngineEntityManager
{
	std::vector<MEngineObject*>* Entities;
	MUtilityIDBank* IDBank;
}

#define MUTILITY_LOG_CATEGORY_ENTITY_MANAGER "MEngineEntityManager"

// Forward declaration
MEngineEntityID GetNextEntityID();

MEngineEntityID MEngineEntityManager::RegisterNewEntity(MEngineObject* entity)
{
	MEngineEntityID ID = GetNextEntityID();
	entity->EntityID = ID;

	if (ID < Entities->size())
		(*Entities)[ID] = entity;
	else
		Entities->push_back(entity);
	return ID;
}

void MEngineEntityManager::DestroyEntity(MEngineEntityID entityID)
{
	MEngineObject* object = (*Entities)[entityID];
	if (object != nullptr)
	{
		delete object;
		(*Entities)[entityID] = nullptr;
		IDBank->ReturnID(entityID);
	}
	else
	{
		if (IDBank->IsIDRecycled(entityID))
			MLOG_WARNING("Attempted to destroy entity with ID " << entityID << " but the entity with that ID has already been destroyed", MUTILITY_LOG_CATEGORY_ENTITY_MANAGER);
		else
			MLOG_WARNING("Attempted to destroy entity with ID " << entityID << " but no entity with that ID exists", MUTILITY_LOG_CATEGORY_ENTITY_MANAGER);
	}
}

const std::vector<MEngineObject*>& MEngineEntityManager::GetEntities()
{
	return *Entities;
}

MEngineEntityID GetNextEntityID()
{
	return IDBank->GetID();
}

void MEngineEntityManager::Initialize()
{
	Entities = new std::vector<MEngineObject*>();
	IDBank = new MUtilityIDBank();
}

void MEngineEntityManager::Shutdown()
{
	delete Entities;
	delete IDBank;
}