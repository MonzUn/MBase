#pragma once
#include "mengineTypes.h"
#include "mengineInternalComponents.h"

namespace MEngine
{
	EntityID CreateButton(int32_t posX, int32_t posY, int32_t width, int32_t height, std::function<void()> callback, TextureID texture, const std::string& Text = "");
}