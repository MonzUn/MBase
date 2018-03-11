#include "Interface/MEngineInternalComponents.h"
#include "MEngineInternalComponentsInternal.h"
#include "Interface/MengineComponentManager.h"
#include <MUtilityLog.h>
#include <MUtilityMacros.h>
#include <vector>

namespace MEngineInternalComponents
{
	void RegisterComponentsTypes();

	std::vector<MEngine::ComponentMask>* m_ComponentMasks;
}

using namespace MEngine;

// ---------- COMPONENTS ----------

void ButtonComponent::Destroy()
{
	delete Callback;
}

void TextComponent::Destroy()
{
	delete Text;
}

// ---------- INTERNAL ----------

void MEngineInternalComponents::Initialize()
{
	m_ComponentMasks = new std::vector<ComponentMask>();
	RegisterComponentsTypes();
}

void MEngineInternalComponents::Shutdown()
{
	for (int i = 0; i < m_ComponentMasks->size(); ++i)
	{
		MEngine::UnregisterComponentType((*m_ComponentMasks)[i]);
	}
	delete m_ComponentMasks;
}

// ---------- LOCAL ----------

void MEngineInternalComponents::RegisterComponentsTypes()
{
	PosSizeComponent::Register(PosSizeComponent(), "PosSizeComponent");
	RectangleRenderingComponent::Register(RectangleRenderingComponent(), "RectangleRenderingComponent");
	TextureRenderingComponent::Register(TextureRenderingComponent(), "TextureRenderingComponent");
	ButtonComponent::Register(ButtonComponent(), "ButtonComponent");
	TextComponent::Register(TextComponent(), "TextBoxComponent");
}