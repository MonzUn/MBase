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
		ConsoleCommand(CommandID id, const std::string& commandName, const MEngineConsoleCallback& callback, const std::string& description)
			: ID(id), CommandName(commandName), Callback(callback), Description(description) {}

		CommandID ID;
		std::string CommandName;
		MEngineConsoleCallback Callback;
		std::string Description;
		SystemID CoupledSystem		= MENGINE_INVALID_SYSTEM_ID;
		GameModeID CoupledGameMode	= MENGINE_INVALID_GAME_MODE_ID;
	};

	bool InitializeConsole(FontID inputFontID, FontID outputFontID);

	CommandID RegisterGlobalCommand(const std::string& commandName, MEngineConsoleCallback callback, const std::string& description = "");
	CommandID RegisterSystemCommand(SystemID ID, const std::string& commandName, MEngineConsoleCallback callback, const std::string& description = "");
	CommandID RegisterGameModeCommand(GameModeID ID, const std::string& commandName, MEngineConsoleCallback callback, const std::string& description = "");
	bool UnregisterCommand(CommandID ID);
	bool UnregisterSystemCommands(SystemID ID);
	bool UnregisterGameModeCommands(GameModeID ID);

	bool ExecuteCommand(const std::string& command, std::string* outResponse);
	void MarkCommandLogRead();

	std::string GetFullCommandLog();
	std::string GetUnreadCommandLog();

	bool SetConsoleFont(FontID ID, ConsoleFont fontToSet = ConsoleFont::Both);
	bool SetConsoleActive(bool active);

	bool IsCommandIDValid(CommandID ID);
	bool CommandExists(const std::string& commandName);
}