#pragma once
#include "mengineTypes.h"
#include "mengineInternalComponents.h"

#define BUTTON_ENTITY_MASK ButtonComponent::GetComponentMask() | TextureRenderingComponent::GetComponentMask()
namespace MEngine
{
	EntityID CreateButton(int32_t posX, int32_t posY, int32_t width, int32_t height, std::function<void()> callback,
		TextureID texture = INVALID_MENGINE_TEXTURE_ID, const std::string& Text = "");
}