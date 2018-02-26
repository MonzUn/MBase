#pragma once
#include <string>

namespace MEngine
{
	void SetFont(const std::string& relativeFontPath);
	void DrawText(int32_t posX, int32_t posY, const std::string& text);
	void DrawTextWithCaret(int32_t posX, int32_t posY, const std::string& text, uint16_t CaretIndex);
}