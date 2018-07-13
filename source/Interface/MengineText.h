#pragma once
#include "MengineTypes.h"
#include "MengineColor.h"
#include <string>

namespace MEngine // TODODB: Create a way to draw text without using the component system (for easy access to text rendering)
{
	constexpr int32_t DEFAULT_TEXT_SIZE = 20;
	const MEngine::ColorData DEFAULT_TEXT_COLOR = PredefinedColors::Colors[PredefinedColors::BLACK];

	FontID CreateFont_(const std::string& relativeFontPath, int32_t fontSize = DEFAULT_TEXT_SIZE, const ColorData& textColor = DEFAULT_TEXT_COLOR); // Underscore postfix avoids conflict with Windows.h macro CreateFont
	bool DestroyFont(FontID ID);

	// Size is returned as uint16_t, int32_t is used so that -1 can be returned in case of an error
	int32_t GetTextWidth(FontID ID, const char* text);
	int32_t GetTextHeight(FontID ID, const char* text);
	int32_t GetLineHeight(FontID ID);

	bool IsFontIDValid(FontID ID);
	bool IsCharASCII(char character);
	bool IsStringASCII(const char* string);
}