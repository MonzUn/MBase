#pragma once
#include <functional>
#include <string.h>

typedef std::function<bool(const std::string*, int32_t, std::string*)> MEngineConsoleCallback;

namespace MEngine
{
	bool RegisterCommand(const std::string& commandName, MEngineConsoleCallback callback);
	bool UnregisterCommand(std::string& commandName);
	void UnregisterAllCommands();

	bool ExecuteCommand(const std::string& command, std::string* outResponse);
}