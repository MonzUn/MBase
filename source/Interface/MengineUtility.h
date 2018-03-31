#pragma once
#include <string>
#include "MengineTypes.h"

namespace MEngine
{
	const std::string& GetApplicationName();
	const std::string& GetExecutablePath();
	const MEngine::InitFlags GetInitFlags();

	bool WindowHasFocus();
	bool WindowIsHovered();
}