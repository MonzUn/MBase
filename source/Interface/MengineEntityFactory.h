#pragma once
#include "MEngineColor.h"
#include "MEngineTypes.h"
#include "MEngineInternalComponents.h"

#define BUTTON_ENTITY_MASK (PosSizeComponent::GetComponentMask() | TextureRenderingComponent::GetComponentMask() | ButtonComponent::GetComponentMask() | TextComponent::GetComponentMask())
#define TEXT_BOX_ENTITY_MASK (PosSizeComponent::GetComponentMask() | RectangleRenderingComponent::GetComponentMask() | TextComponent::GetComponentMask())
#define TEXT_BOX_EDITABLE_ENTITY_MASK (PosSizeComponent::GetComponentMask() | RectangleRenderingComponent::GetComponentMask() | ButtonComponent::GetComponentMask() | TextComponent::GetComponentMask())

constexpr uint32_t MENGINE_DEFAULT_UI_BUTTON_DEPTH	= 50;
constexpr uint32_t MENGINE_DEFAULT_UI_TEXTBOX_DEPTH = 25;

const MEngine::TextBoxFlags TEXT_BOX_EDIT_OVERWRITE_RESET_FLAG = (MEngine::TextBoxFlags::Editable | MEngine::TextBoxFlags::OverwriteOnDefaultTextMatch | MEngine::TextBoxFlags::ResetToDefaultWhenEmpty); // TODODB: Why can't this be a constexpr (Causes error: "Expression must have constant value")? Everything seems to be const...

namespace MEngine
{
	EntityID CreateButton(int32_t posX, int32_t posY, int32_t width, int32_t height, std::function<void()> callback,
		uint32_t posZ = MENGINE_DEFAULT_UI_BUTTON_DEPTH, TextureID texture = TextureID::Invalid(),
		FontID fontID = FontID::Invalid(), const std::string& Text = "", TextAlignment textAlignment = TextAlignment::CenterCentered);

	EntityID CreateTextBox(int32_t posX, int32_t posY, int32_t width, int32_t height, FontID fontID, uint32_t posZ = MENGINE_DEFAULT_UI_TEXTBOX_DEPTH,
		const std::string& text = "", TextAlignment alignment = TextAlignment::CenterLeft, TextBoxFlags editFlags = TextBoxFlags::None, const ColorData& backgroundColor = PredefinedColors::Colors[PredefinedColors::TRANSPARENT],
		const ColorData& borderColor = PredefinedColors::Colors[PredefinedColors::TRANSPARENT]);

	bool ShowButton(EntityID ID);
	bool HideButton(EntityID ID);
	
	bool ShowTextBox(EntityID ID);
	bool HideTextBox(EntityID ID);
}