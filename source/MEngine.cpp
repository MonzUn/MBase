#include "Interface/MEngine.h"
#include "MEngineGraphicsInternal.h"
#include "MEngineInputInternal.h"
#include "MEngineGlobalSystems.h"
#include "MEngineSystemManagerInternal.h"
#include <MUtilityLog.h>
#include <MUtilitySystem.h>
#include <SDL.h>
#include <cassert>
#include <iostream>

#define LOG_CATEGORY_GENERAL "MEngine"

// TOODDB: Add scrollbar to scrollable textboxes

namespace MEngine
{
	bool m_Initialized		= false;
	bool m_QuitRequested	= false;
}

bool MEngine::Initialize(const char* applicationName, int32_t windowWidth, int32_t windowHeight)
{
	assert(!IsInitialized() && "Calling Initialize after initialization has already been performed");

	assert(MUtilityLog::IsInitialized() && "MutilityLog has not been initialized; call MUtilityLog::Initialize before MEngine::Initialize");

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		MLOG_ERROR("Initialization failed; SDL_Init Error: " + std::string(SDL_GetError()), LOG_CATEGORY_GENERAL);
		return false;
	}

	if (!MEngineGraphics::Initialize(applicationName, windowWidth, windowHeight)) // TODODB: Make this initialize as all the other internal global systems
	{
		MLOG_ERROR("Failed to initialize MEngineGraphics", LOG_CATEGORY_GENERAL);
		MUtilityLog::Shutdown();
		return false;
	}

	MEngineGlobalSystems::Start(applicationName);

	MLOG_INFO("MEngine initialized successfully", LOG_CATEGORY_GENERAL);

	m_Initialized = true;
	return true;
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