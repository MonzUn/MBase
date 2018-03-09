#pragma once
#include "mengineTypes.h"
#include <string>

namespace MEngine // TODODB: Create a way to draw text without using the component system (for easy access to text rendering)
{
	MEngineFontID CreateFont(const std::string& relativeFontPath);
	bool DestroyFont(MEngineFontID ID);

	// Size is returned as uint16_t, int32_t is used so that -1 can be returned in case of an error
	int32_t GetTextWidth(MEngineFontID ID, const char* text);
	int32_t GetTextHeight(MEngineFontID ID, const char* text);
}