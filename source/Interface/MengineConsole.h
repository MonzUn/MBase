#pragma once
#include "MEngineTypes.h"
#include <functional>
#include <string.h>

typedef std::function<bool(const std::string*, int32_t, std::string*)> MEngineConsoleCallback;

namespace MEngine
{
	enum class ConsoleFont
	{
		Input,
		Output,
		Both,
	};
	bool InitializeConsole(MEngineFontID inputFontID, MEngineFontID outputFontID);

	bool RegisterCommand(const std::string& commandName, MEngineConsoleCallback callback);
	bool UnregisterCommand(std::string& commandName);
	void UnregisterAllCommands();
	bool ExecuteCommand(const std::string& command, std::string* outResponse);

	bool SetFont(MEngineFontID ID, ConsoleFont fontToSet = ConsoleFont::Both);
	bool SetConsoleActive(bool active);
}