#pragma once
#include "mengineColor.h"
#include "mengineComponent.h"
#include "MUtilityTypes.h"
#include <MUtilityMacros.h>
#include <functional>
#include <string>

namespace MEngine // TODODB: Find a cache friendly way to not have to store positions and other shared data in each and every component
{
	class RectangleRenderingComponent : public ComponentBase<RectangleRenderingComponent>
	{
	public:
		int32_t PosX	= 0;
		int32_t PosY	= 0;
		int32_t Width	= 0;
		int32_t Height	= 0;
		ColorData BorderColor	= ColorData(PredefinedColors::TRANSPARENT);
		ColorData FillColor		= ColorData(PredefinedColors::TRANSPARENT);
	};

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