#include "Interface/MEngineConsole.h"
#include "MEngineConsoleInternal.h"
#include "Interface/MEngineColor.h"
#include "Interface/MEngineEntityManager.h"
#include "Interface/MEngineEntityFactory.h"
#include "Interface/MEngineGraphics.h"
#include "Interface/MEngineInput.h"
#include "Interface/MengineSystemManager.h"
#include "Interface/MEngineText.h"
#include "Interface/MEngineTypes.h"
#include "Interface/MengineUtility.h"
#include <MUtilityIDBank.h>
#include <MUtilityLog.h>
#include <algorithm>
#include <cctype>
#include <unordered_map>

#define LOG_CATEGROY_CONSOLE "MEngineConsole"

using namespace MEngine;
using MUtility::MUtilityIDBank;

typedef std::unordered_map<std::string, MEngine::ConsoleCommand> CommandMap;
typedef std::unordered_map<SystemID, std::vector<CommandID>> SystemCommandMap;
typedef std::unordered_map<GameModeID, std::vector<CommandID>> GameModeCommandMap;

constexpr char DELIMITER = ' ';
constexpr int32_t INPUT_TEXTBOX_HEIGHT = 25;

void CreateComponents();
void DestroyComponents();
bool ExecuteHelpCommand(const std::string* parameters, int32_t parameterCount, std::string* outResponse);
bool IsCommandGlobal(CommandID id);
bool IsCommandSystemCoupled(CommandID id);
bool IsCommandGameModeCoupled(CommandID id);

namespace MEngine
{
	CommandMap*	m_Commands				= nullptr;
	MUtilityIDBank*	m_CommandIDBank		= nullptr;	
	std::string* m_StoredLogMessages	= nullptr;
	std::string* m_CommandLog			= nullptr;
	uint64_t m_CommandLogLastReadIndex	= 0;
	MEngine::EntityID m_BackgroundID	= MENGINE_INVALID_ENTITY_ID;
	MEngine::EntityID m_OutputTextboxID	= MENGINE_INVALID_ENTITY_ID;
	MEngine::EntityID m_InputTextboxID	= MENGINE_INVALID_ENTITY_ID;
	MEngine::FontID m_InputFont			= MENGINE_INVALID_FONT_ID;
	MEngine::FontID m_OutputFont		= MENGINE_INVALID_FONT_ID;
	bool m_IsActive = true;
	bool m_InitializedByHost = false;
	int32_t m_OutputTextBoxOriginalHeight = -1;
}

using namespace MEngine;
using namespace MEngineConsole;
using namespace PredefinedColors;

// ---------- INTERFACE ----------

bool MEngine::InitializeConsole(FontID inputFontID, FontID outputFontID)
{
	bool result = false;
	if (!m_InitializedByHost)
	{
		m_InitializedByHost = true;
		m_InputFont			= inputFontID;
		m_OutputFont		= outputFontID;
		CreateComponents();

		RegisterGlobalCommand("help", MEngineConsoleCallback(ExecuteHelpCommand), "Displays a list of all commands"); // TODODB: Add short and long descriptions and use the short one here
	}
	else
		MLOG_WARNING("Attempted to initialize console multiple times", LOG_CATEGROY_CONSOLE);

	return result;
}

CommandID MEngine::RegisterGlobalCommand(const std::string& commandName, MEngineConsoleCallback callback, const std::string& description)
{
	std::string commandNameLower = commandName;
	std::transform(commandNameLower.begin(), commandNameLower.end(), commandNameLower.begin(), std::tolower);
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if (m_Commands->find(commandName) != m_Commands->end())
	{
		MLOG_WARNING("Attempted to register multiple commands using the same name; name = " << commandNameLower, LOG_CATEGROY_CONSOLE);
		return MENGINE_INVALID_COMMAND_ID;
	}
#endif

	CommandID commandID = m_CommandIDBank->GetID();
	m_Commands->emplace(commandNameLower, ConsoleCommand(commandID, commandName, callback, description));
	return commandID;
}

MEngine::CommandID MEngine::RegisterSystemCommand(SystemID ID, const std::string& commandName, MEngineConsoleCallback callback, const std::string& description)
{
	std::string commandNameLower = commandName;
	std::transform(commandNameLower.begin(), commandNameLower.end(), commandNameLower.begin(), std::tolower);
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if (m_Commands->find(commandName) != m_Commands->end())
	{
		MLOG_WARNING("Attempted to register multiple commands using the same name; name = " << commandNameLower, LOG_CATEGROY_CONSOLE);
		return MENGINE_INVALID_COMMAND_ID;
	}
#endif

	CommandID commandID = m_CommandIDBank->GetID();
	auto& iterator = m_Commands->emplace(commandNameLower, ConsoleCommand(commandID, commandName, callback, description));
	iterator.first->second.CoupledSystem = ID;
	return commandID;
}

MEngine::CommandID MEngine::RegisterGameModeCommand(GameModeID ID, const std::string& commandName, MEngineConsoleCallback callback, const std::string& description)
{
	std::string commandNameLower = commandName;
	std::transform(commandNameLower.begin(), commandNameLower.end(), commandNameLower.begin(), std::tolower);
#if COMPILE_MODE == COMPILE_MODE_DEBUG
	if (m_Commands->find(commandName) != m_Commands->end())
	{
		MLOG_WARNING("Attempted to register multiple commands using the same name; name = " << commandNameLower, LOG_CATEGROY_CONSOLE);
		return MENGINE_INVALID_COMMAND_ID;
	}
#endif

	CommandID commandID = m_CommandIDBank->GetID();
	auto& iterator = m_Commands->emplace(commandNameLower, ConsoleCommand(commandID, commandName, callback, description));
	iterator.first->second.CoupledGameMode = ID;
	return commandID;
}

bool MEngine::UnregisterCommand(CommandID ID)
{
	if (!IsCommandIDValid(ID))
	{
		MLOG_WARNING("Attempted to unregister command using an invalid ID; id = " << ID, LOG_CATEGROY_CONSOLE);
		return false;
	}

	for (auto nameAndCommandIt = m_Commands->begin(); nameAndCommandIt != m_Commands->end();)
	{
		if (nameAndCommandIt->second.ID == ID)
		{
			m_CommandIDBank->ReturnID(nameAndCommandIt->second.ID);
			m_Commands->erase(nameAndCommandIt);
			return true;
		}
	}

	MLOG_ERROR("Failed to unregister command using a valid ID; ID = " << ID, LOG_CATEGROY_CONSOLE);
	return false;
}

bool MEngine::UnregisterSystemCommands(SystemID ID)
{
	if (!IsSystemIDValid(ID))
	{
		MLOG_WARNING("Attempted to unregister system coupled command using an invalid system ID; ID = " << ID, LOG_CATEGROY_CONSOLE);
		return false;
	}

	auto nameAndCommandIt = m_Commands->begin();
	while (nameAndCommandIt != m_Commands->end())
	{
		if (nameAndCommandIt->second.CoupledSystem == ID)
		{
			m_CommandIDBank->ReturnID(nameAndCommandIt->second.ID);
			nameAndCommandIt = m_Commands->erase(nameAndCommandIt);
		}
		else
			++nameAndCommandIt;
	}

	return true;
}

bool MEngine::UnregisterGameModeCommands(GameModeID ID)
{
	if (!IsGameModeIDValid(ID))
	{
		MLOG_WARNING("Attempted to unregister game mode coupled command using an invalid game mode ID; ID = " << ID, LOG_CATEGROY_CONSOLE);
		return false;
	}

	auto nameAndCommandIt = m_Commands->begin();
	while (nameAndCommandIt != m_Commands->end())
	{
		if (nameAndCommandIt->second.CoupledGameMode == ID)
		{
			m_CommandIDBank->ReturnID(nameAndCommandIt->second.ID);
			nameAndCommandIt = m_Commands->erase(nameAndCommandIt);
		}
		else
			++nameAndCommandIt;
	}

	return true;
}

bool MEngine::ExecuteCommand(const std::string& command, std::string* outResponse)
{
	if (command == "")
		return false;

	std::string commandLower = command;
	std::transform(commandLower.begin(), commandLower.end(), commandLower.begin(), std::tolower);

	// Find command name
	int32_t firstSpacePosition = static_cast<int32_t>(commandLower.find_first_of(DELIMITER));
	std::string commandName = ((firstSpacePosition == -1) ? commandLower : commandLower.substr(0, firstSpacePosition));

	bool result = false;
	auto commandIterator = m_Commands->find(commandName);
	if (commandIterator != m_Commands->end())
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

		if (parameterCount == 1 && (parameters[0] == "h" || parameters[0] == "-h" || parameters[0] == "-help" || parameters[0] == "help")) // TODODB: Add support for - and -- for paramters
		{
			if(outResponse != nullptr)
				*outResponse = commandIterator->second.Description;
			else
				MLOG_WARNING("Attempted to get description of command \"" << commandName << "\" but the response string was null", LOG_CATEGROY_CONSOLE);
		}
		else // Execute command
			result = commandIterator->second.Callback(parameters, parameterCount, outResponse);

		delete[] parameters;
	}
	else if(outResponse != nullptr)
		*outResponse = '\"' + commandName + "\" is not a valid command";
		
	return result;
}

void MEngine::MarkCommandLogRead()
{
	m_CommandLogLastReadIndex = m_CommandLog->empty() ? 0 : m_CommandLog->size() - 1; // -1 for conversion to index from size
}

std::string MEngine::GetFullCommandLog()
{
	return *m_CommandLog;
}

std::string MEngine::GetUnreadCommandLog()
{
	return m_CommandLogLastReadIndex < (m_CommandLog->size() - 1) ? m_CommandLog->substr(m_CommandLogLastReadIndex) : "";
}

bool MEngine::SetConsoleFont(FontID ID, ConsoleFont fontToSet)
{
	if (!m_InitializedByHost)
	{
		MLOG_WARNING("Attempted to set font before initializing the console", LOG_CATEGROY_CONSOLE);
		return false;
	}

	bool result = !IsFontIDValid(ID);
	if (result)
	{
		switch (fontToSet)
		{
			case MEngine::ConsoleFont::Input:
			{
				m_InputFont = ID;
			} break;
			case MEngine::ConsoleFont::Output:
			{
				m_OutputFont = ID;
			} break;
			case MEngine::ConsoleFont::Both:
			{
				m_InputFont = ID;
				m_OutputFont = ID;
			} break;

			default:
				break;
		}

		CreateComponents();
	}
	else
		MLOG_WARNING("Attempted to set console font using an invalid font ID; ID = " << ID, LOG_CATEGROY_CONSOLE);

	return result;
}

bool MEngine::SetConsoleActive(bool active)
{
	if (!m_InitializedByHost)
	{
		MLOG_WARNING("Attempted to acitvate/deactivate console without it being initialized", LOG_CATEGROY_CONSOLE);
		return false;
	}

	if (m_IsActive == active)
		return false;

	RectangleRenderingComponent* mainBackground = static_cast<RectangleRenderingComponent*>(GetComponent(m_BackgroundID, RectangleRenderingComponent::GetComponentMask()));
	TextComponent* outputText = static_cast<TextComponent*>(GetComponent(m_OutputTextboxID, TextComponent::GetComponentMask()));
	RectangleRenderingComponent* outputTextBackground = static_cast<RectangleRenderingComponent*>(GetComponent(m_OutputTextboxID, RectangleRenderingComponent::GetComponentMask()));
	TextComponent* inputText = static_cast<TextComponent*>(GetComponent(m_InputTextboxID, TextComponent::GetComponentMask()));
	RectangleRenderingComponent* inputTextbackground = static_cast<RectangleRenderingComponent*>(GetComponent(m_InputTextboxID, RectangleRenderingComponent::GetComponentMask()));
	ButtonComponent* inputButton = static_cast<ButtonComponent*>(GetComponent(m_InputTextboxID, ButtonComponent::GetComponentMask()));
	if (active)
	{
		mainBackground->RenderIgnore		= false;
		outputText->RenderIgnore			= false;
		outputTextBackground->RenderIgnore	= false;
		inputText->RenderIgnore				= false;
		inputTextbackground->RenderIgnore	= false;
		inputButton->IsActive				= true;
		inputText->StartEditing();
	}
	else
	{
		mainBackground->RenderIgnore		= true;
		outputText->RenderIgnore			= true;
		outputTextBackground->RenderIgnore	= true;
		inputText->RenderIgnore				= true;
		inputTextbackground->RenderIgnore	= true;
		inputButton->IsActive				= false;
		inputText->StopEditing();
	}
	m_IsActive = active;
	return true;
}

bool MEngine::IsCommandIDValid(CommandID ID)
{
	return m_CommandIDBank->IsIDActive(ID);
}

bool MEngine::CommandExists(const std::string& commandName)
{
	return m_Commands->find(commandName) != m_Commands->end();
}

// ---------- INTERNAL ----------

void MEngineConsole::Initialize()
{
	m_Commands = new CommandMap();
	m_CommandIDBank = new MUtilityIDBank();
	m_StoredLogMessages = new std::string();
	m_CommandLog = new std::string();
}

void MEngineConsole::shutdown()
{
	delete m_Commands;
	delete m_CommandIDBank;
	delete m_StoredLogMessages;
	delete m_CommandLog;
	DestroyComponents();
}

void MEngineConsole::Update()
{
	if (m_InitializedByHost)
	{
		// Sore new log messages for the next time the console is opened
		MUtilityLog::GetUnreadMessages(*m_StoredLogMessages);
			
		if (m_IsActive)
		{
			const TextComponent* inputText = static_cast<const TextComponent*>(GetComponent(m_InputTextboxID, TextComponent::GetComponentMask()));
			TextComponent* outputText = static_cast<TextComponent*>(GetComponent(m_OutputTextboxID, TextComponent::GetComponentMask()));
			int32_t initialTextHeight = GetTextHeight(outputText->FontID, outputText->Text->c_str());
			int32_t initialTextLength = static_cast<int32_t>(outputText->Text->length());

			// Handle commands
			if (*inputText->Text != "" && IsInputString(inputText->Text) && KeyReleased(MKEY_MAIN_ENTER) || KeyReleased(MKEY_NUMPAD_ENTER))
			{
				std::string response;
				ExecuteCommand(*inputText->Text, &response);
				*m_StoredLogMessages += *inputText->Text;
				*m_StoredLogMessages += response;

				std::string addition;
				if (!outputText->Text->empty())
					addition += '\n';
				addition += "\n> " + *inputText->Text + '\n' + " - " + response;
				*m_StoredLogMessages += addition;
				*m_CommandLog += addition;

				*inputText->Text = "";
			}

			*outputText->Text += *m_StoredLogMessages;

			if (outputText->Text->length() > initialTextLength)
			{
				// Move the console along with the output text
				PosSizeComponent* outputTextPos = static_cast<PosSizeComponent*>(GetComponent(m_OutputTextboxID, PosSizeComponent::GetComponentMask()));
				int32_t AddedTextHeight = GetTextHeight(outputText->FontID, outputText->Text->c_str()) - initialTextHeight;
				int32_t lineHeight = GetLineHeight(outputText->FontID);

				if (AddedTextHeight > 0)
				{
					if (initialTextHeight + AddedTextHeight > outputTextPos->Height)
					{
						int32_t leftOverSpace = std::max(outputTextPos->Height - initialTextHeight, 0);
						int32_t leftOverRows = leftOverSpace / lineHeight;
						int32_t scrollLines = (AddedTextHeight / lineHeight) - leftOverRows;
						outputText->ScrolledLinesCount += scrollLines; // TODODB: Make the console only scroll if it's currently bottomed out
					}
				}
			}
			*m_StoredLogMessages = "";
		}

		if (KeyReleased(MKEY_GRAVE) && WindowHasFocus()) // TODODB: Check against some action here when such a system has been built 
			SetConsoleActive(!m_IsActive);
	}
}

// ---------- LOCAL ----------

void CreateComponents()
{
	DestroyComponents();
	
	m_OutputTextBoxOriginalHeight = GetWindowHeight() / 3;
	int32_t fullWidth = GetWindowWidth();

	m_BackgroundID = CreateEntity();
	AddComponentsToEntity(m_BackgroundID, PosSizeComponent::GetComponentMask() | RectangleRenderingComponent::GetComponentMask());

	PosSizeComponent* posSize = static_cast<PosSizeComponent*>(GetComponent(m_BackgroundID, PosSizeComponent::GetComponentMask()));
	posSize->PosX = 0;
	posSize->PosY = 0;
	posSize->PosZ = 1U;
	posSize->Width = fullWidth;
	posSize->Height = m_OutputTextBoxOriginalHeight;

	RectangleRenderingComponent* background = static_cast<RectangleRenderingComponent*>(GetComponent(m_BackgroundID, RectangleRenderingComponent::GetComponentMask()));
	background->FillColor = ColorData(0, 128, 0, 128);

	m_OutputTextboxID = CreateTextBox(0, 0, fullWidth, m_OutputTextBoxOriginalHeight - INPUT_TEXTBOX_HEIGHT, m_OutputFont, 0U, "", TextAlignment::TopLeft, TextBoxFlags::Scrollable);
	m_InputTextboxID = CreateTextBox(0, m_OutputTextBoxOriginalHeight - INPUT_TEXTBOX_HEIGHT, fullWidth, INPUT_TEXTBOX_HEIGHT, m_InputFont, 0U, "", MEngine::TextAlignment::BottomLeft, TextBoxFlags::Editable, Colors[TRANSPARENT], Colors[BLUE]);

	SetConsoleActive(false);
}

void DestroyComponents()
{
	if(m_BackgroundID != MENGINE_INVALID_ENTITY_ID)
		DestroyEntity(m_BackgroundID);
	if (m_OutputTextboxID != MENGINE_INVALID_ENTITY_ID)
		DestroyEntity(m_OutputTextboxID);
	if (m_InputTextboxID != MENGINE_INVALID_ENTITY_ID)
		DestroyEntity(m_InputTextboxID);
}

bool ExecuteHelpCommand(const std::string* parameters, int32_t parameterCount, std::string* outResponse)
{
	bool result = false;
	if (parameterCount == 0)
	{
		if (outResponse != nullptr)
		{
			if (m_Commands->size() > 0)
			{
				*outResponse += "Available commands:\n";
				for (auto&& nameAndCommandIterator : *m_Commands)
				{
					*outResponse += '*' + nameAndCommandIterator.first + '\n';
				}
				*outResponse += "\nFor command descriptions; use -h parameter on the desired command";
			}
			else
				*outResponse = "No commands available";

			result = true;
		}
		else
			MLOG_WARNING("Executed \"help\" command using null response string", LOG_CATEGROY_CONSOLE);
	}
	else if(outResponse != nullptr)
		*outResponse = "Wrong number of parameters supplied";

	return result;
}

bool IsCommandGlobal(CommandID ID)
{
	if (!IsCommandIDValid(ID))
	{
		MLOG_WARNING("Attempted to check if command was global using an invalid command ID; ID = " << ID, LOG_CATEGROY_CONSOLE);
		return false;
	}

	for (auto& nameAndCommand : *m_Commands)
	{
		if (nameAndCommand.second.ID == ID)
		{
			return nameAndCommand.second.CoupledSystem == MENGINE_INVALID_SYSTEM_ID && nameAndCommand.second.CoupledGameMode == MENGINE_INVALID_GAME_MODE_ID;
		}
	}

	MLOG_ERROR("Failed to find command using a valid command ID; ID = " << ID, LOG_CATEGROY_CONSOLE);
	return false;
}

bool IsCommandSystemCoupled(CommandID ID)
{
	if (!IsCommandIDValid(ID))
	{
		MLOG_WARNING("Attempted to check if command was system coupled using an invalid command ID; ID = " << ID, LOG_CATEGROY_CONSOLE);
		return false;
	}

	for (auto& nameAndCommand : *m_Commands)
	{
		if (nameAndCommand.second.ID == ID)
		{
			return nameAndCommand.second.CoupledSystem != MENGINE_INVALID_SYSTEM_ID;
		}
	}

	MLOG_ERROR("Failed to find command using a valid command ID; ID = " << ID, LOG_CATEGROY_CONSOLE);
	return false;
}

bool IsCommandGameModeCoupled(CommandID ID)
{
	if (!IsCommandIDValid(ID))
	{
		MLOG_WARNING("Attempted to check if command was game mode coupled using an invalid command ID; ID = " << ID, LOG_CATEGROY_CONSOLE);
		return false;
	}

	for (auto& nameAndCommand : *m_Commands)
	{
		if (nameAndCommand.second.ID == ID)
		{
			return nameAndCommand.second.CoupledGameMode != MENGINE_INVALID_GAME_MODE_ID;
		}
	}

	MLOG_ERROR("Failed to find command using a valid command ID; ID = " << ID, LOG_CATEGROY_CONSOLE);
	return false;
}