#include "interface/mengineInternalComponents.h"
#include "mengineInternalComponentsInternal.h"
#include "interface/mengineComponentManager.h"
#include <MUtilityLog.h>
#include <MUtilityMacros.h>
#include <vector>

namespace MEngineInternalComponents
{
	void RegisterComponentsTypes();

	std::vector<MEngineComponentMask>* m_ComponentMasks;
}

// ---------- INTERNAL ----------

void MEngineInternalComponents::Initialize()
{
	m_ComponentMasks = new std::vector<MEngineComponentMask>();
	RegisterComponentsTypes();
}

void MEngineInternalComponents::Shutdown()
{
	for (int i = 0; i < m_ComponentMasks->size(); ++i)
	{
		MEngineComponentManager::UnregisterComponentType((*m_ComponentMasks)[i]);
	}
	delete m_ComponentMasks;
}

// ---------- LOCAL ----------

void MEngineInternalComponents::RegisterComponentsTypes()
{
	MEngine::TextureRenderingComponent::Register(MEngine::TextureRenderingComponent(), "TextureRenderingComponent");
}