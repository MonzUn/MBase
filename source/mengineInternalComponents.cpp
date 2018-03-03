#include "interface/mengineInternalComponents.h"
#include "mengineInternalComponentsInternal.h"
#include "interface/mengineComponentManager.h"
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
	delete text;
	delete Callback;
}

void TextBoxComponent::Destroy()
{
	delete text;
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
	RectangleRenderingComponent::Register(RectangleRenderingComponent(), "RectangleRenderingComponent");
	TextureRenderingComponent::Register(TextureRenderingComponent(), "TextureRenderingComponent");
	ButtonComponent::Register(ButtonComponent(), "ButtonComponent");
	TextBoxComponent::Register(TextBoxComponent(), "TextBoxComponent");
}