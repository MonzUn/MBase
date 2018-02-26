#include "interface/mengineComponentManager.h"
#include "componentBuffer.h"
#include "mengineComponentManagerInternal.h"
#include <MUtilityLog.h>
#include <MUtilityMath.h>
#include <vector>

#define LOG_CATEGORY_COMPONENT_MANAGER "ComponentManager"

namespace MEngineComponentManager
{
	std::vector<MEngine::ComponentBuffer>* m_Buffers;
	MUtility::MUtilityBitMaskIDBank m_IDBank;
}

// ---------- INTERFACE ----------

MEngineComponentMask MEngineComponentManager::RegisterComponentType(const MEngine::Component& templateComponent, uint32_t templateComponentSize, uint32_t maxCount, const char* componentName)
{
	MEngineComponentMask componentMask = m_IDBank.GetID();
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if (componentMask == INVALID_MENGINE_COMPONENT_MASK)
	{
		MLOG_WARNING("Ran out of component mask IDs when attempting to add component \"" << componentName << '"', LOG_CATEGORY_COMPONENT_MANAGER);
		return INVALID_MENGINE_COMPONENT_MASK;
	}
#endif
	m_Buffers->emplace_back(templateComponent, templateComponentSize, maxCount, componentName, componentMask);

	return componentMask;
}

bool MEngineComponentManager::UnregisterComponentType(MEngineComponentMask componentType) // TODODB: Destroy active components
{
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if (componentType == INVALID_MENGINE_COMPONENT_MASK)
	{
		MLOG_WARNING("Attempted to unregister an invalid component mask; componentMask = " << MUtility::BitSetToString(componentType), LOG_CATEGORY_COMPONENT_MANAGER);
		return false;
	}
	else if (!m_IDBank.IsIDActive(componentType))
	{
		MLOG_WARNING("Attempted to unregister an inactive component mask; componentMask = " << MUtility::BitSetToString(componentType), LOG_CATEGORY_COMPONENT_MANAGER);
		return false;
	}
#endif

	for (int i = 0; i < m_Buffers->size(); ++i)
	{
		if ((*m_Buffers)[i].ComponentMask == componentType)
		{
			if (m_IDBank.ReturnID(componentType))
			{
				m_Buffers->erase(m_Buffers->begin() + i);
				return true;
			}
			else
			{
				MLOG_WARNING("Failed to return the component mask for component \"" << (*m_Buffers)[i].ComponentName << "\"; the component type will not be unregistered", LOG_CATEGORY_COMPONENT_MANAGER);
				return false;
			}
		}
	}

	MLOG_ERROR("Failed to find the component buffer for component mask " << MUtility::BitSetToString(componentType), LOG_CATEGORY_COMPONENT_MANAGER);
	return false;
}

MUtility::Byte* MEngineComponentManager::GetComponentBuffer(MEngineComponentMask componentType, int32_t& outComponentCount)
{
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if (componentType == INVALID_MENGINE_COMPONENT_MASK)
	{
		MLOG_WARNING("Attempted to get buffer using an invalid component mask; componentMask = " << MUtility::BitSetToString(componentType), LOG_CATEGORY_COMPONENT_MANAGER);
		outComponentCount = -1;
		return nullptr;
	}
	else if (!m_IDBank.IsIDActive(componentType))
	{
		MLOG_WARNING("Attempted to get buffer using an inactive component mask; componentMask = " << MUtility::BitSetToString(componentType), LOG_CATEGORY_COMPONENT_MANAGER);
		outComponentCount = -1;
		return nullptr;
	}
#endif

	for (int i = 0; i < m_Buffers->size(); ++i)
	{
		if ((*m_Buffers)[i].ComponentMask == componentType)
		{
			outComponentCount = static_cast<int32_t>((*m_Buffers)[i].GetCount());
			return (*m_Buffers)[i].GetBuffer();
		}
	}

	MLOG_ERROR("Failed to find the component buffer for component mask " << MUtility::BitSetToString(componentType), LOG_CATEGORY_COMPONENT_MANAGER);
	outComponentCount = -1;
	return nullptr;
}

// ---------- INTERNAL ----------

void MEngineComponentManager::Initialize()
{
	m_Buffers = new std::vector<MEngine::ComponentBuffer>();
}

void MEngineComponentManager::Shutdown()
{
	delete m_Buffers;
}

uint32_t MEngineComponentManager::AllocateComponent(MEngineComponentMask componentType, MEngineEntityID owner)
{
	uint32_t componentBufferIndex = MUtilityMath::FastLog2(componentType);
	return (*m_Buffers)[componentBufferIndex].AllocateComponent(owner);
}

bool MEngineComponentManager::ReturnComponent(MEngineComponentMask componentType, uint32_t componentIndex)
{
	uint32_t componentBufferIndex = MUtilityMath::FastLog2(componentType);
	return (*m_Buffers)[componentBufferIndex].ReturnComponent(componentIndex);
}

MEngine::Component* MEngineComponentManager::GetComponent(MEngineComponentMask componentType, uint32_t componentIndex)
{
	uint32_t componentBufferIndex = MUtilityMath::FastLog2(componentType);
	return (*m_Buffers)[componentBufferIndex].GetComponent(componentIndex);
}