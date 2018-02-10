#pragma once
#include <string>

namespace MEngineUtility
{
	const std::string& GetExecutablePath();

	bool WindowHasFocus();
	bool WindowIsHovered();
}