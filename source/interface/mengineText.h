#pragma once
#include <string>

namespace MEngineText
{
	void SetFont(const std::string& relativeFontPath);
	void DrawText(int32_t posX, int32_t posY, const std::string& text);
}