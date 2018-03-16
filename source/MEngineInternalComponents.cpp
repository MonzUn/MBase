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

// ---------- FLAGS ----------

TextBoxFlags MEngine::operator|(const TextBoxFlags& lhs, const TextBoxFlags& rhs) { return static_cast<const TextBoxFlags>(static_cast<const uint32_t>(lhs) | static_cast<const uint32_t>(rhs)); }
TextBoxFlags MEngine::operator&(const TextBoxFlags& lhs, const TextBoxFlags& rhs) { return static_cast<const TextBoxFlags>(static_cast<const uint32_t>(lhs) & static_cast<const uint32_t>(rhs)); }
TextBoxFlags& MEngine::operator|=(TextBoxFlags& lhs, const TextBoxFlags& rhs) { return lhs = (lhs | rhs); }
TextBoxFlags& MEngine::operator&=(TextBoxFlags& lhs, const TextBoxFlags& rhs) { return lhs = (lhs & rhs); }
bool MEngine::operator==(const TextBoxFlags& lhs, const TextBoxFlags& rhs) { return static_cast<const uint32_t>(lhs) == static_cast<const uint32_t>(rhs); };
bool MEngine::operator!=(const TextBoxFlags& lhs, const TextBoxFlags& rhs) { return static_cast<const uint32_t>(lhs) != static_cast<const uint32_t>(rhs); };
bool MEngine::operator==(const TextBoxFlags& lhs, const uint32_t& rhs) { return static_cast<const uint32_t>(lhs) == rhs; };
bool MEngine::operator!=(const TextBoxFlags& lhs, const uint32_t& rhs) { return static_cast<const uint32_t>(lhs) != rhs; };
bool MEngine::operator==(const uint32_t& lhs, const TextBoxFlags& rhs) { return lhs == static_cast<const uint32_t>(rhs); };
bool MEngine::operator!=(const uint32_t& lhs, const TextBoxFlags& rhs) { return lhs != static_cast<const uint32_t>(rhs); };

// ---------- COMPONENTS ----------

void ButtonComponent::Destroy()
{
	delete Callback;
}

void TextComponent::Destroy()
{
	delete Text;
	delete DefaultText;
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