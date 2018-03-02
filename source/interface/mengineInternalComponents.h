#pragma once
#include "mengineComponent.h"
#include "MUtilityTypes.h"
#include <MUtilityMacros.h>
#include <functional>
#include <string>

namespace MEngine
{
	class TextureRenderingComponent : public ComponentBase<TextureRenderingComponent>
	{
	public:
		int32_t PosX	= 0;
		int32_t PosY	= 0;
		int32_t Width	= 0;
		int32_t Height	= 0;
		bool RenderIgnore = false;
		TextureID TextureID = INVALID_MENGINE_TEXTURE_ID;
	};

	class ButtonComponent : public ComponentBase<ButtonComponent>
	{
	public:
		int32_t PosX	= 0;
		int32_t PosY	= 0;
		int32_t Width	= 0;
		int32_t Height	= 0;
		std::string text = "";
		std::function<void()> Callback; // TODODB: Attempt to make it possible to use any parameters and return type
	};
}