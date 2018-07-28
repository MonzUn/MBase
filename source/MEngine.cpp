#include "Interface/MEngine.h"
#include "interface/MengineConsole.h"
#include "MEngineGraphicsInternal.h"
#include "MEngineInputInternal.h"
#include "MEngineGlobalSystems.h"
#include "MEngineSystemManagerInternal.h"
#include <MUtilityLog.h>
#include <MUtilityString.h>
#include <MUtilitySystem.h>
#include <SDL.h>
#include <cassert>
#include <iostream>

#define LOG_CATEGORY_GENERAL "MEngine"

// TODODB: Fix MEngine being spelled with small e in interface file names
// TOODDB: Add scrollbar to scrollable textboxes

namespace MEngine
{
	void RegisterMEngineCommands();
	bool ExecuteSetLogOutputModeCommand(const std::string* parameters, int32_t parameterCount, std::string* outResponse);

	bool m_Initialized		= false;
	bool m_QuitRequested	= false;
}

bool MEngine::Initialize(const char* applicationName, InitFlags initFlags)
{
	assert(!IsInitialized() && "Calling Initialize after initialization has already been performed");

	assert(MUtilityLog::IsInitialized() && "MutilityLog has not been initialized; call MUtilityLog::Initialize before MEngine::Initialize");

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		MLOG_ERROR("Initialization failed; SDL_Init Error: " + std::string(SDL_GetError()), LOG_CATEGORY_GENERAL);
		return false;
	}

	MEngineGlobalSystems::Start(applicationName, initFlags);
	RegisterMEngineCommands();

	MLOG_INFO("MEngine initialized successfully", LOG_CATEGORY_GENERAL);

	m_Initialized = true;
	return true;
}

bool MEngine::CreateWindow_(const char* windowTitle, int32_t windowPosX , int32_t windowPosY, int32_t windowWidth, int32_t windowHeight)
{
	bool result = MEngineGraphics::Initialize(windowTitle, windowPosX, windowPosY, windowWidth, windowHeight); // TODODB: Make this initialize as all the other internal global systems
	if (!result)
		MLOG_ERROR("Failed to initialize MEngineGraphics", LOG_CATEGORY_GENERAL);
	return result;
}

void MEngine::Shutdown()
{
	assert(IsInitialized() && "Calling Shutdown before initialization has been performed");

	MEngineGlobalSystems::Stop();

	m_Initialized = false;
	SDL_Quit();

	MLOG_INFO("MEngine terminated gracefully", LOG_CATEGORY_GENERAL);
}

bool MEngine::IsInitialized()
{
	return m_Initialized;
}

bool MEngine::ShouldQuit()
{
	return m_QuitRequested;
}

void MEngine::Update()
{
	MEngineGlobalSystems::PreEventUpdate();
	SDL_Event event;
	while (SDL_PollEvent(&event) != 0)
	{
		if (event.type == SDL_QUIT)
		{
			m_QuitRequested = true;
			break;
		}

		MEngineInput::HandleEvent(event);
	};
	MEngineGlobalSystems::PostEventUpdate();

	MEngineGlobalSystems::PreSystemsUpdate();
	MEngineSystemManager::Update();
	MEngineGlobalSystems::PostSystemsUpdate();

	MUtilityLog::ClearUnreadMessages();
}

void MEngine::Render()
{
	MEngineGraphics::Render();
}

// ---------- INTERNAL ----------

void MEngine::RegisterMEngineCommands()
{
	RegisterGlobalCommand("SetLogOutputMode", &ExecuteSetLogOutputModeCommand, "Sets when logs are written to file; 0 = On shutdown, 1 = Immediately after each log entry");
}

bool MEngine::ExecuteSetLogOutputModeCommand(const std::string* parameters, int32_t parameterCount, std::string* outResponse)
{
	bool result = false;
	if (parameterCount == 1)
	{
		if (MUtility::IsStringNumber(*parameters))
		{
			int32_t parameter = std::stoi(parameters->c_str());
			if (parameter >= 0 && parameter <= 1)
			{
				MUtilityLog::SetOutputTrigger(MUtilityLogOutputTrigger(parameter));
				if(outResponse != nullptr)
					*outResponse = "Log output trigger has been set";
			}
			else if(outResponse != nullptr)
				*outResponse = "The parameter must be in the range 0 - 1";
		}
		else if(outResponse != nullptr)
			*outResponse = "The parameter must be a number";
	}
	else if(outResponse != nullptr)
		*outResponse = "Wrong number of parameters supplied";

	return result;
}