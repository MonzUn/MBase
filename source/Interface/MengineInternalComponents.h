#pragma once
#include "MEngineColor.h"
#include "MEngineComponent.h"
#include "MEngineInput.h"
#include <MUtilityLog.h>
#include <MUtilityTypes.h>
#include <MUtilityMacros.h>
#include <functional>
#include <string>

namespace MEngine
{
	enum class TextBoxEditFlags : uint32_t
	{
		None = 0,

		Editable = 1 << 0,
		OverwriteOnDefaultTextMatch = 1 << 1,
		ResetToDefaultWhenEmpty = 1 << 2,
	};

	// TODODB: Can we hide this using a custom underlying type defined in MUtility?
	TextBoxEditFlags operator|(const TextBoxEditFlags& lhs, const TextBoxEditFlags& rhs);
	TextBoxEditFlags operator&(const TextBoxEditFlags& lhs, const TextBoxEditFlags& rhs);
	TextBoxEditFlags& operator|=(TextBoxEditFlags& lhs, const TextBoxEditFlags& rhs);
	TextBoxEditFlags& operator&=(TextBoxEditFlags& lhs, const TextBoxEditFlags& rhs);
	bool operator==(const TextBoxEditFlags& lhs, const TextBoxEditFlags& rhs);
	bool operator!=(const TextBoxEditFlags& lhs, const TextBoxEditFlags& rhs);
	bool operator==(const TextBoxEditFlags& lhs, const uint32_t& rhs);
	bool operator!=(const TextBoxEditFlags& lhs, const uint32_t& rhs);
	bool operator==(const uint32_t& lhs, const TextBoxEditFlags& rhs);
	bool operator!=(const uint32_t& lhs, const TextBoxEditFlags& rhs);

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
		const std::string* DefaultText = nullptr;
		TextAlignment Alignment;
		bool RenderIgnore = false;
		TextBoxEditFlags EditFlags = TextBoxEditFlags::None;

		void StartEditing() const // TODODB: When we can use any parameter for button callbacks; move this to the relevant system instead
		{
			if (Text == nullptr)
			{
				MLOG_WARNING("Attempted to edit textcomponent text while it was null", "TextComponent"); // TODODB: Set proper log category when this code has been moved into a system
				return;
			}

			if ((EditFlags & TextBoxEditFlags::Editable) != 0)
			{
				// Removed the text if it is the default text
				if (((EditFlags & TextBoxEditFlags::OverwriteOnDefaultTextMatch) != 0) && DefaultText != nullptr && *Text == *DefaultText)
					*Text = "";

				MEngine::StartTextInput(Text);
			}
		};

		void StopEditing() const // TODODB: See if there are any additional places we should call this (direct calls to MEngineInput was used before this was created)
		{
			if ((EditFlags & TextBoxEditFlags::Editable) != 0 && MEngine::IsInputString(Text))
			{
				// Return the text to the default if it is left empty
				if ((EditFlags & TextBoxEditFlags::ResetToDefaultWhenEmpty) != 0 && *Text == "")
					*Text = *DefaultText;

				MEngine::StopTextInput();
			}
		}
	};
}