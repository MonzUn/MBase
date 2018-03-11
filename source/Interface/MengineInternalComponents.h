#pragma once
#include "MEngineColor.h"
#include "MEngineComponent.h"
#include "MEngineInput.h"
#include "MUtilityTypes.h"
#include <MUtilityMacros.h>
#include <functional>
#include <string>

namespace MEngine
{
	class PosSizeComponent : public ComponentBase<PosSizeComponent>
	{
	public:
		int32_t		PosX	= 0;
		int32_t		PosY	= 0;
		uint32_t	PosZ	= ~0U;
		int32_t		Width	= 0;
		int32_t		Height	= 0;
	};

	class RectangleRenderingComponent : public ComponentBase<RectangleRenderingComponent>
	{
	public:
		bool IsFullyTransparent() const { return BorderColor.IsFullyTransparent() && FillColor.IsFullyTransparent(); }

		ColorData BorderColor	= ColorData(PredefinedColors::TRANSPARENT);
		ColorData FillColor		= ColorData(PredefinedColors::TRANSPARENT);
		bool RenderIgnore		= false;
	};

	class TextureRenderingComponent : public ComponentBase<TextureRenderingComponent>
	{
	public:
		bool RenderIgnore	= false;
		TextureID TextureID = INVALID_MENGINE_TEXTURE_ID;
	};

	class ButtonComponent : public ComponentBase<ButtonComponent>
	{
	public:
		void Destroy() override;

		bool IsTriggered = false;
		std::function<void()>* Callback = nullptr; // TODODB: Attempt to make it possible to use any parameters and return type
	};

	// TODODB: Add text rendering alignment
	class TextComponent : public ComponentBase<TextComponent>
	{
	public:
		void Destroy() override;
		MEngineFontID FontID = INVALID_MENGINE_FONT_ID;
		std::string* Text = nullptr;
		TextAlignment Alignment;
		bool RenderIgnore = false;

		void StartEditing() const // TODODB: When we can use any parameter for button callbacks; move this to the relevant system instead
		{
			MEngine::StartTextInput(Text);
		};

		void StopEditing() const // TODODB: See if there are any additional places we should call this (direct calls to MEngineInput was used before this was created)
		{
			if (MEngine::IsInputString(Text))
				MEngine::StopTextInput();
		}
	};
}