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

namespace MEngine
{
	bool m_Initialized = false;
	bool m_QuitRequested = false;
}

bool MEngine::Initialize(const char* appName, int32_t windowWidth, int32_t windowHeight)
{
	assert(!IsInitialized() && "Calling SDLWrapper::Initialize but it has already been initialized");

	MUtilityLog::Initialize();

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		MLOG_ERROR("MEngine initialization failed; SDL_Init Error: " + std::string(SDL_GetError()), LOG_CATEGORY_GENERAL);
		MUtilityLog::Shutdown();
		return false;
	}

	if (!MEngineGraphics::Initialize(appName, windowWidth, windowHeight)) // TODODB: Make this initialize as all the other internal global systems
	{
		MLOG_ERROR("Failed to initialize MEngineGraphics", LOG_CATEGORY_GENERAL);
		MUtilityLog::Shutdown();
		return false;
	}

	MEngineGlobalSystems::Start();

	MLOG_INFO("MEngine initialized successfully", LOG_CATEGORY_GENERAL);

	m_Initialized = true;
	return true;
}

void MEngine::Shutdown()
{
	assert(IsInitialized() && "Calling SDLWrapper::Shutdown but it has not yet been initialized");

	MEngineGlobalSystems::Stop();

	m_Initialized = false;
	SDL_Quit();

	MLOG_INFO("MEngine terminated gracefully", LOG_CATEGORY_GENERAL);
	MUtilityLog::Shutdown();
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
	MEngineGlobalSystems::Update();

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

	MEngineSystemManager::Update();
}

void MEngine::Render()
{
	MEngineGraphics::Render();
}