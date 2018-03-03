#pragma once
#include "mengineColor.h"
#include "mengineTypes.h"
#include "mengineInternalComponents.h"

#define BUTTON_ENTITY_MASK ButtonComponent::GetComponentMask() | TextureRenderingComponent::GetComponentMask()
#define TEXT_BOX_ENTITY_MASK RectangleRenderingComponent::GetComponentMask() | TextBoxComponent::GetComponentMask() | ButtonComponent::GetComponentMask()

namespace MEngine
{
	EntityID CreateButton(int32_t posX, int32_t posY, int32_t width, int32_t height, std::function<void()> callback,
		TextureID texture = INVALID_MENGINE_TEXTURE_ID, const std::string& Text = "");

	EntityID CreateTextBox(int32_t posX, int32_t posY, int32_t width, int32_t height, bool editable = false,
		const std::string& text = "", const ColorData& backgroundColor = PredefinedColors::Colors[PredefinedColors::TRANSPARENT],
		const ColorData& borderColor = PredefinedColors::Colors[PredefinedColors::TRANSPARENT]);
}