#include "Interface/MEngineEntityManager.h"
#include "Interface/MEngineSettings.h"
#include "MEngineEntityManagerInternal.h"
#include "MEngineComponentManagerInternal.h"
#include <MUtilityBitset.h>
#include <MUtilityIDBank.h>
#include <MUtilityIntrinsics.h>
#include <MUtilityLog.h>
#include <MUtilityMath.h>
#include <MUtilityPlatformDefinitions.h>
#include <cassert>

#define LOG_CATEGORY_ENTITY_MANAGER "MEngineEntityManager"

using namespace MEngine;
using namespace MEngineEntityManager;
using MUtility::MUtilityIDBank;

// TODODB: Rework these namespaces in all files so that it's easy to debug and find get the correct context when coding using local functions and variables
namespace MEngineEntityManager
{
	int32_t GetEntityIndex(EntityID ID);
	uint32_t CalcComponentIndiceListIndex(ComponentMask entityComponentMask, ComponentMask componentType);
	ComponentMask RemoveComponentsFromEntityByIndex(ComponentMask componentMask, int32_t entityIndex);
}

namespace
{
	std::vector<EntityID>*		m_Entities;
	std::vector<ComponentMask>*	m_ComponentMasks;
	std::vector<std::vector<uint32_t>>* m_ComponentIndices;
	MUtilityIDBank<EntityID>* m_EntityIDBank;
}

// ---------- INTERFACE ----------

EntityID MEngine::CreateEntity() // TODODB: Take component mask and add the components described by the mask
{
	EntityID ID = m_EntityIDBank->GetID();
	m_Entities->push_back(ID);
	m_ComponentMasks->push_back(MUtility::EMPTY_BITSET);
	m_ComponentIndices->push_back(std::vector<uint32_t>());

	return ID;
}

bool MEngine::DestroyEntity(EntityID ID)
{
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if (!m_EntityIDBank->IsIDActive(ID))
	{
		if(Settings::HighLogLevel)
			MLOG_WARNING("Attempted to destroy entity using an inactive entity ID; ID = " << ID, LOG_CATEGORY_ENTITY_MANAGER);

		return false;
	}
#endif

	int32_t entityIndex = GetEntityIndex(ID);
	if (entityIndex >= 0)
	{
		if ((*m_Entities)[entityIndex] == ID)
		{
			RemoveComponentsFromEntityByIndex((*m_ComponentMasks)[entityIndex], entityIndex);

			m_Entities->erase(m_Entities->begin() + entityIndex);
			m_ComponentMasks->erase(m_ComponentMasks->begin() + entityIndex);
			m_ComponentIndices->erase(m_ComponentIndices->begin() + entityIndex);
			m_EntityIDBank->ReturnID(ID);
			return true;
		}
	}
	return false;
}

ComponentMask MEngine::AddComponentsToEntity(EntityID ID, ComponentMask componentMask)
{
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if (componentMask == MENGINE_INVALID_COMPONENT_MASK)
	{
		MLOG_WARNING("Attempted to add component(s) to entity using an invalid component mask; mask = " << MUtility::BitSetToString(componentMask), LOG_CATEGORY_ENTITY_MANAGER);
		return componentMask;
	}
	else if (!m_EntityIDBank->IsIDActive(ID))
	{
		MLOG_WARNING("Attempted to add components to an entity that doesn't exist; ID = " << ID, LOG_CATEGORY_ENTITY_MANAGER);
		return componentMask;
	}
#endif

	int32_t entityIndex = GetEntityIndex(ID);
	std::vector<uint32_t>& componentIndices = (*m_ComponentIndices)[entityIndex];
	while (componentMask != MUtility::EMPTY_BITSET)
	{
		ComponentMask singleComponentMask = MUtility::GetHighestSetBit(componentMask);
		uint32_t componentIndiceListIndex = CalcComponentIndiceListIndex((*m_ComponentMasks)[entityIndex], singleComponentMask);
		componentIndices.insert(componentIndices.begin() + componentIndiceListIndex, MEngineComponentManager::AllocateComponent(singleComponentMask, ID));
		(*m_ComponentMasks)[entityIndex] |= singleComponentMask;

		componentMask &= ~MUtility::GetHighestSetBit(componentMask);
	}

	return MUtility::EMPTY_BITSET;
}

ComponentMask MEngine::RemoveComponentsFromEntity(EntityID ID, ComponentMask componentMask)
{
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if (componentMask == MENGINE_INVALID_COMPONENT_MASK)
	{
		MLOG_WARNING("Attempted to removed component(s) from entity using an invalid component mask; mask = " << MUtility::BitSetToString(componentMask), LOG_CATEGORY_ENTITY_MANAGER);
		return componentMask;
	}
	else if (!m_EntityIDBank->IsIDActive(ID))
	{
		MLOG_WARNING("Attempted to remove component(s) from an entity that doesn't exist; ID = " << ID, LOG_CATEGORY_ENTITY_MANAGER);
		return componentMask;
	}
#endif

	int32_t entityIndex = GetEntityIndex(ID);
	if(entityIndex >= 0)
	{
		if((*m_Entities)[entityIndex] == ID)
			return RemoveComponentsFromEntityByIndex(componentMask, entityIndex);
	}

	return componentMask;
}

void MEngine::GetEntitiesMatchingMask(ComponentMask componentMask, std::vector<EntityID>& outEntities, MaskMatchMode matchMode)
{
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if (componentMask == MENGINE_INVALID_COMPONENT_MASK)
	{
		MLOG_WARNING("Attempted to get components matching an invalid component mask", LOG_CATEGORY_ENTITY_MANAGER);
		return;
	}
#endif

	for (int i = 0; i < m_Entities->size(); ++i)
	{
		ComponentMask currentMask = (*m_ComponentMasks)[i];
		bool isMatch = false;
		switch (matchMode)
		{
			case MaskMatchMode::Any:
			{
				isMatch = ((currentMask & componentMask) != 0);
			} break;

			case MaskMatchMode::Partial:
			{
				isMatch = ((currentMask & componentMask) == componentMask);
			} break;

			case MaskMatchMode::Exact:
			{
				isMatch = ((currentMask & componentMask) == componentMask && MUtility::PopCount(componentMask) == MUtility::PopCount(currentMask));
			} break;

		default:
			MLOG_ERROR("Received unknown matchMode", LOG_CATEGORY_ENTITY_MANAGER);
			break;
		}

		if (isMatch)
			outEntities.push_back((*m_Entities)[i]);	
	}
}

MEngine::Component* MEngine::GetComponent(EntityID ID, ComponentMask componentType)
{
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if (componentType == MENGINE_INVALID_COMPONENT_MASK)
	{
		MLOG_WARNING("Attempted to get component for entity using an invalid component mask; mask = " << MUtility::BitSetToString(componentType), LOG_CATEGORY_ENTITY_MANAGER);
		return nullptr;
	}
	else if (!m_EntityIDBank->IsIDActive(ID))
	{
		MLOG_WARNING("Attempted to get component for an entity that doesn't exist; ID = " << ID, LOG_CATEGORY_ENTITY_MANAGER);
		return nullptr;
	}
	else if (MUtility::PopCount(componentType) != 1)
	{
		MLOG_WARNING("Attempted to get component for an entity using a component mask containing more or less than one component; mask = " << MUtility::BitSetToString(componentType), LOG_CATEGORY_ENTITY_MANAGER);
		return nullptr;
	}
#endif

	int32_t entityIndex = GetEntityIndex(ID);
	if (entityIndex >= 0)
	{
#if COMPILE_MODE == COMPILE_MODE_DEBUG
		if (((*m_ComponentMasks)[entityIndex] & componentType) == 0)
		{
			MLOG_WARNING("Attempted to get component of type " << MUtility::BitSetToString(componentType) << " for an entity that lacks that component type; entity component mask = " << MUtility::BitSetToString((*m_ComponentMasks)[ID]), LOG_CATEGORY_ENTITY_MANAGER);
			return nullptr;
		}
#endif

		uint32_t componentIndex = (*m_ComponentIndices)[entityIndex][CalcComponentIndiceListIndex((*m_ComponentMasks)[entityIndex], componentType)];
		return MEngineComponentManager::GetComponent(componentType, componentIndex);
	}

	return nullptr;
}

ComponentMask MEngine::GetComponentMask(EntityID ID)
{
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if (!m_EntityIDBank->IsIDActive(ID))
	{
		MLOG_WARNING("Attempted to get component mask from an entity that doesn't exist; ID = " << ID, LOG_CATEGORY_ENTITY_MANAGER);
		return MENGINE_INVALID_COMPONENT_MASK;
	}
#endif

	return (*m_ComponentMasks)[GetEntityIndex(ID)];
}

bool MEngine::IsEntityIDValid(EntityID ID)
{
	return m_EntityIDBank->IsIDActive(ID);
}

// ---------- INTERNAL ----------

void MEngineEntityManager::Initialize()
{
	m_Entities			= new std::vector<EntityID>();
	m_ComponentMasks	= new std::vector<ComponentMask>();
	m_ComponentIndices	= new std::vector<std::vector<uint32_t>>();
	m_EntityIDBank		= new MUtilityIDBank<EntityID>();
}

void MEngineEntityManager::Shutdown()
{
	delete m_Entities;
	delete m_ComponentMasks;
	delete m_ComponentIndices;
	delete m_EntityIDBank;
}

void MEngineEntityManager::UpdateComponentIndex(EntityID ID, ComponentMask componentType, uint32_t newComponentIndex)
{
	int32_t entityIndex = GetEntityIndex(ID);
	if (entityIndex >= 0)
	{
		uint32_t componentIndexListIndex = CalcComponentIndiceListIndex((*m_ComponentMasks)[entityIndex], componentType);
		(*m_ComponentIndices)[entityIndex][componentIndexListIndex] = newComponentIndex;
		return;
	}
}

// ---------- LOCAL ----------

int32_t MEngineEntityManager::GetEntityIndex(EntityID ID) // TODODB: This function does not scale well; need to get rid of it or redesign
{
	for (int i = 0; i < m_Entities->size(); ++i)
	{
		if ((*m_Entities)[i] == ID)
		{
			return i;
		}
	}

	MLOG_ERROR("Failed to find entity with ID " << ID << " even though it is marked as active", LOG_CATEGORY_ENTITY_MANAGER);
	return -1;
}

uint32_t MEngineEntityManager::CalcComponentIndiceListIndex(ComponentMask entityComponentMask, ComponentMask componentType)
{
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if(MUtility::PopCount(componentType) != 1)
		MLOG_ERROR("A component meask containing more or less than 1 set bit was supplied; only the highest set bit will be considered", LOG_CATEGORY_ENTITY_MANAGER);
#endif
	
	uint32_t componentTypeBitIndex = MUtilityMath::FastLog2(componentType); // Find the index of the bit signifying the component type
	uint64_t shiftedMask = componentTypeBitIndex != 0 ? (entityComponentMask << (MEngineComponentManager::MAX_COMPONENTS - componentTypeBitIndex)) : 0; // Shift away all bits above the component type bit index
	return static_cast<uint32_t>(MUtility::PopCount(shiftedMask)); // Return the number of set bits left in the shifted mask
}

ComponentMask MEngineEntityManager::RemoveComponentsFromEntityByIndex(ComponentMask componentMask, int32_t entityIndex)
{
	ComponentMask failedComponents = 0ULL;
	std::vector<uint32_t>& componentIndices = (*m_ComponentIndices)[entityIndex];
	while (componentMask != MUtility::EMPTY_BITSET)
	{
		ComponentMask singleComponentMask = MUtility::GetHighestSetBit(componentMask);
		uint32_t componentIndiceListIndex = CalcComponentIndiceListIndex((*m_ComponentMasks)[entityIndex], singleComponentMask);
		if (MEngineComponentManager::ReturnComponent(singleComponentMask, componentIndices[componentIndiceListIndex]))
		{
			componentIndices.erase(componentIndices.begin() + componentIndiceListIndex);
			(*m_ComponentMasks)[entityIndex] &= ~MUtility::GetHighestSetBit(componentMask);
		}
		else
			failedComponents &= MUtility::GetHighestSetBit(componentMask);

		componentMask &= ~MUtility::GetHighestSetBit(componentMask);
	}

	return failedComponents;
}