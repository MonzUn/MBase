#include "interface/mengineEntityManager.h"
#include "mengineEntityManagerInternal.h"
#include "mengineComponentManagerInternal.h"
#include <MUtilityIDBank.h>
#include <MUtilityIntrinsics.h>
#include <MUtilityLog.h>
#include <MUtilityMath.h>
#include <MUtilityPlatformDefinitions.h>
#include <cassert>

using namespace MEngineEntityManager;
using MUtility::MUtilityIDBank;

#define LOG_CATEGORY_ENTITY_MANAGER "MEngineEntityManager"

// ---------- LOCAL ----------

namespace MEngineEntityManager
{
	uint32_t CalcComponentIndiceListIndex(MEngineEntityID entityID, MEngineComponentMask entityComponentMask, MEngineComponentMask componentType);

	std::vector<MEngineEntityID>*		m_Entities;
	std::vector<MEngineComponentMask>*	m_ComponentMasks;
	std::vector<std::vector<uint32_t>>* m_ComponentIndices;
	MUtilityIDBank* m_IDBank;
}

// ---------- INTERFACE ----------

MEngineEntityID MEngineEntityManager::CreateEntity() // TODODB: Take component mask and add the components described by the mask
{
	MEngineEntityID ID = m_IDBank->GetID();
	m_Entities->push_back(ID);
	m_ComponentMasks->push_back(MUtility::EMPTY_BITSET);
	m_ComponentIndices->push_back(std::vector<uint32_t>());

	return ID;
}

bool MEngineEntityManager::DestroyEntity(MEngineEntityID entityID)
{
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if (!m_IDBank->IsIDActive(entityID))
	{
		MLOG_WARNING("Attempted to destroy entity using an inactive entity ID; ID = " << entityID, LOG_CATEGORY_ENTITY_MANAGER);
		return false;
	}
#endif

	for (int i = 0; i < m_Entities->size(); ++i)
	{
		if ((*m_Entities)[i] == i)
		{
			RemoveComponentsFromEntity((*m_ComponentMasks)[entityID], entityID);

			m_Entities->erase(m_Entities->begin() + i);
			m_ComponentMasks->erase(m_ComponentMasks->begin() + i);
			m_ComponentIndices->erase(m_ComponentIndices->begin() + i);
			m_IDBank->ReturnID(entityID);
			return true;
		}
	}

	MLOG_ERROR("Failed to find entity with ID " << entityID << " even though it is marked as active", LOG_CATEGORY_ENTITY_MANAGER);
	return false;
}

MEngineComponentMask MEngineEntityManager::AddComponentsToEntity(MEngineComponentMask componentMask, MEngineEntityID entityID)
{
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if (componentMask == INVALID_MENGINE_COMPONENT_MASK)
	{
		MLOG_WARNING("Attempted to add component(s) to entity using an invalid component mask; mask = " << MUtility::BitSetToString(componentMask), LOG_CATEGORY_ENTITY_MANAGER);
		return componentMask;
	}
	else if (!m_IDBank->IsIDActive(entityID))
	{
		MLOG_WARNING("Attempted to add components to an entity that doesn't exist; ID = " << entityID, LOG_CATEGORY_ENTITY_MANAGER);
		return componentMask;
	}
#endif

	std::vector<uint32_t>& componentIndices = (*m_ComponentIndices)[entityID];
	while (componentMask != MUtility::EMPTY_BITSET)
	{
		MEngineComponentMask singleComponentMask = 1ULL << (MUtility::GetHighestSetBit(componentMask) - 1);
		uint32_t componentIndiceListIndex = CalcComponentIndiceListIndex(entityID, (*m_ComponentMasks)[entityID], singleComponentMask);
		componentIndices.insert(componentIndices.begin() + componentIndiceListIndex, MEngineComponentManager::AllocateComponent(singleComponentMask, entityID));
		(*m_ComponentMasks)[entityID] |= singleComponentMask;

		componentMask &= ~(1ULL << MUtility::GetHighestSetBitIndex(componentMask));
	}

	return MUtility::EMPTY_BITSET;
}

MEngineComponentMask MEngineEntityManager::RemoveComponentsFromEntity(MEngineComponentMask componentMask, MEngineEntityID entityID)
{
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if (componentMask == INVALID_MENGINE_COMPONENT_MASK)
	{
		MLOG_WARNING("Attempted to removed component(s) from entity using an invalid component mask; mask = " << MUtility::BitSetToString(componentMask), LOG_CATEGORY_ENTITY_MANAGER);
		return componentMask;
	}
	else if (!m_IDBank->IsIDActive(entityID))
	{
		MLOG_WARNING("Attempted to remove component(s) from an entity that doesn't exist; ID = " << entityID, LOG_CATEGORY_ENTITY_MANAGER);
		return componentMask;
	}
#endif

	MEngineComponentMask failedComponents = 0ULL;
	std::vector<uint32_t>& componentIndices = (*m_ComponentIndices)[entityID];
	for(int i = 0; i < MUtility::PopCount(componentMask); ++i)
	{
		MEngineComponentMask singleComponentMask = 1ULL << (MUtility::GetHighestSetBit(componentMask) - 1);
		uint32_t componentIndiceListIndex = CalcComponentIndiceListIndex(entityID, (*m_ComponentMasks)[entityID], singleComponentMask);
		if (MEngineComponentManager::ReturnComponent(singleComponentMask, componentIndices[componentIndiceListIndex]))
		{
			componentIndices.erase(componentIndices.begin() + componentIndiceListIndex);
			(*m_ComponentMasks)[entityID] &= ~(1ULL << MUtility::GetHighestSetBitIndex(componentMask));
		}
		else
			failedComponents &= (1ULL << MUtility::GetHighestSetBitIndex(componentMask));
		
		componentMask &= ~(1ULL << MUtility::GetHighestSetBitIndex(componentMask));
	}

	return failedComponents;
}

MEngine::Component* MEngineEntityManager::GetComponentForEntity(MEngineComponentMask componentType, MEngineEntityID entityID)
{
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if (componentType == INVALID_MENGINE_COMPONENT_MASK)
	{
		MLOG_WARNING("Attempted to get component for entity using an invalid component mask; mask = " << MUtility::BitSetToString(componentType), LOG_CATEGORY_ENTITY_MANAGER);
		return nullptr;
	}
	else if (!m_IDBank->IsIDActive(entityID))
	{
		MLOG_WARNING("Attempted to get component for an entity that doesn't exist; ID = " << entityID, LOG_CATEGORY_ENTITY_MANAGER);
		return nullptr;
	}
	else if (MUtility::PopCount(componentType) != 1)
	{
		MLOG_WARNING("Attempted to get component for an entity using a component mask containing more or less than one component; mask = " << MUtility::BitSetToString(componentType), LOG_CATEGORY_ENTITY_MANAGER);
		return nullptr;
	}
	else if (((*m_ComponentMasks)[entityID] & componentType) == 0)
	{
		MLOG_WARNING("Attempted to get component of type " << MUtility::BitSetToString(componentType) << " for an entity that lacks that component type; entity component mask = " << MUtility::BitSetToString((*m_ComponentMasks)[entityID]), LOG_CATEGORY_ENTITY_MANAGER);
		return nullptr;
	}
#endif
	uint32_t componentIndex = (*m_ComponentIndices)[entityID][CalcComponentIndiceListIndex(entityID, (*m_ComponentMasks)[entityID], componentType)];
	return MEngineComponentManager::GetComponent(componentType, componentIndex);
}

// ---------- INTERNAL ----------

void MEngineEntityManager::Initialize()
{
	m_Entities			= new std::vector<MEngineEntityID>();
	m_ComponentMasks		= new std::vector<MEngineComponentMask>();
	m_ComponentIndices	= new std::vector<std::vector<uint32_t>>();
	m_IDBank				= new MUtilityIDBank();
}

void MEngineEntityManager::Shutdown()
{
	delete m_Entities;
	delete m_ComponentMasks;
	delete m_ComponentIndices;
	delete m_IDBank;
}

void MEngineEntityManager::UpdateComponentIndex(MEngineEntityID ID, MEngineComponentMask componentType, uint32_t newComponentIndex)
{
	uint32_t componentIndexListIndex = CalcComponentIndiceListIndex(ID, (*m_ComponentMasks)[ID], componentType);
	(*m_ComponentIndices)[ID][componentIndexListIndex] = newComponentIndex;
}

// ---------- LOCAL ----------

uint32_t MEngineEntityManager::CalcComponentIndiceListIndex(MEngineEntityID entityID, MEngineComponentMask entityComponentMask, MEngineComponentMask componentType)
{
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if(MUtility::PopCount(componentType) != 1)
		MLOG_ERROR("A component meask containing more or less than i set bit was supplied; only the highest set bit will be considered", LOG_CATEGORY_ENTITY_MANAGER);
#endif
	
	uint32_t componentTypeBitIndex = MUtilityMath::FastLog2(componentType); // Find the index of the bit signifying the component type
	uint64_t shiftedMask = componentTypeBitIndex != 0 ? ((*m_ComponentMasks)[entityID] << (MEngineComponentManager::MAX_COMPONENTS - componentTypeBitIndex)) : 0; // Shift away all bits above the component type bit index
	return static_cast<uint32_t>(MUtility::PopCount(shiftedMask)); // Return the number of set bits left in the shifted mask
}