#include "interface/mengineConsole.h"
#include "mengineConsoleInternal.h"
#include <MUtilityLog.h>
#include <algorithm>
#include <cctype>
#include <unordered_map>

#define LOG_CATEGROY_CONSOLE "MEngineConsole"

typedef std::unordered_map<std::string, MEngineConsoleCallback> CommandMap;

constexpr char DELIMITER = ' ';

namespace MEngineConsole
{
	CommandMap* m_Commands = nullptr;
}

using namespace MEngine;
using namespace MEngineConsole;

// ---------- INTERNAL ----------

bool MEngine::RegisterCommand(const std::string& commandName, MEngineConsoleCallback callback)
{
	std::string commandNameLower = commandName;
	std::transform(commandNameLower.begin(), commandNameLower.end(), commandNameLower.begin(), std::tolower);

	bool result = true;
	if (m_Commands->find(commandNameLower) != m_Commands->end())
	{
		MLOG_WARNING("Attempted to register multiple commands using the same name; name = " << commandNameLower, LOG_CATEGROY_CONSOLE);
		result = false;
	}

	if(result)
		m_Commands->emplace(commandNameLower, callback);

	return result;
}

bool MEngine::UnregisterCommand(std::string& commandName)
{
	std::string commandNameLower = commandName;
	std::transform(commandNameLower.begin(), commandNameLower.end(), commandNameLower.begin(), std::tolower);

	bool result = false;
	auto iterator = m_Commands->find(commandNameLower);
	if (iterator != m_Commands->end())
	{
		m_Commands->erase(iterator);
		result = true;
	}
	else
		MLOG_WARNING("Attempted to unregister command \"" << commandNameLower << "\" but no such command is registered", LOG_CATEGROY_CONSOLE);

	return result;
}

void MEngine::UnregisterAllCommands()
{
	m_Commands->clear();
}

bool MEngine::ExecuteCommand(const std::string& command, std::string* outResponse)
{
	std::string commandLower = command;
	std::transform(commandLower.begin(), commandLower.end(), commandLower.begin(), std::tolower);

	// Find command name
	int32_t firstSpacePosition = static_cast<int32_t>(commandLower.find_first_of(DELIMITER));
	std::string commandName = ((firstSpacePosition == -1) ? commandLower : commandLower.substr(0, firstSpacePosition));

	bool result = false;
	auto iterator = m_Commands->find(commandName);
	if (iterator != m_Commands->end())
	{
		// Create parameter list
		std::string commandParameterString = commandLower.substr(firstSpacePosition + 1); // +1 so that the space doesn't get included in the string
		int32_t parameterCount = static_cast<int32_t>(std::count(commandLower.begin(), commandLower.end(), DELIMITER));

		std::string* parameters = ((parameterCount > 0) ? new std::string[parameterCount] : nullptr);
		int32_t offset = firstSpacePosition;
		for (int i = 0; i < parameterCount; ++i)
		{
			parameters[i] = commandLower.substr(offset + 1); // +1 so that the space doesn't get included in the string
			offset = static_cast<int32_t>(commandParameterString.find_first_of(DELIMITER, offset));
		}

		// Execute
		result = iterator->second(parameters, parameterCount, outResponse);
		delete[] parameters;
	}
	else
		MLOG_WARNING("Attempted to execute command \"" << commandName << "\" but no such command exists", LOG_CATEGROY_CONSOLE);

	return result;
}

// ---------- INTERNAL ----------

void MEngineConsole::Initialize()
{
	m_Commands	= new CommandMap();
}

void MEngineConsole::shutdown()
{
	delete m_Commands;
}