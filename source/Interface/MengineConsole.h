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

	struct ConsoleCommand
	{
		ConsoleCommand(const std::string& commandName, const MEngineConsoleCallback& callback, const std::string& description)
			: CommandName(commandName), Callback(callback), Description(description) {}

		std::string CommandName;
		MEngineConsoleCallback Callback;
		std::string Description;
	};

	bool InitializeConsole(FontID inputFontID, FontID outputFontID);

	bool RegisterCommand(const std::string& commandName, MEngineConsoleCallback callback, const std::string& description = "");
	bool UnregisterCommand(std::string& commandName);
	void UnregisterAllCommands();
	bool ExecuteCommand(const std::string& command, std::string* outResponse);
	void MarkCommandLogRead();

	std::string GetFullCommandLog();
	std::string GetUnreadCommandLog();

	bool SetConsoleFont(FontID ID, ConsoleFont fontToSet = ConsoleFont::Both);
	bool SetConsoleActive(bool active);
}