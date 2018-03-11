#pragma once
#include <string>

namespace MEngine
{
	const std::string& GetExecutablePath();

	bool WindowHasFocus();
	bool WindowIsHovered();
}