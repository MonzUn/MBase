#include "interface/mengine.h"
#include "mengineData.h"
#include "mengineLogInternal.h"
#include "interface/mengineLog.h"
#include <SDL.h>
#include <cassert>
#include <iostream>

#define MENGINE_LOG_CATEGORY_GENERAL "MEngine"

namespace
{
	bool Initialized = false;
	bool QuitRequested = false;
}

bool MEngine::Initialize(const char* appName = "MEngineApp")
{
	assert(!IsInitialized() && "Calling SDLWrapper::Initialize but it has already been initialized");

	MEngineLog::Initialize();
	MEngineLog::RegisterCategory(MENGINE_LOG_CATEGORY_GENERAL);

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		MLOG_ERROR("MEngine initialization failed; SDL_Init Error: " + std::string(SDL_GetError()) + "; program will close", MENGINE_LOG_CATEGORY_GENERAL);
		return false;
	}

	SDLData::GetInstance().Window = SDL_CreateWindow(appName, 100, 100, 640, 480, SDL_WINDOW_SHOWN);
	if (SDLData::GetInstance().Window == nullptr)
	{
		MLOG_ERROR("MEngine initialization failed; SDL_CreateWindow Error: " + std::string(SDL_GetError()) + "; program will close", MENGINE_LOG_CATEGORY_GENERAL);
		return false;
	}

	SDLData::GetInstance().Renderer = SDL_CreateRenderer(SDLData::GetInstance().Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (SDLData::GetInstance().Renderer == nullptr)
	{
		MLOG_ERROR("MEngine initialization failed; SDL_CreateRenderer Error: " + std::string(SDL_GetError()) + "; program will close", MENGINE_LOG_CATEGORY_GENERAL);
		return false;
	}

	MLOG_INFO("MEngine initialized successfully", MENGINE_LOG_CATEGORY_GENERAL);

	Initialized = true;
	return true;
}

void MEngine::Shutdown()
{
	assert(IsInitialized() && "Calling SDLWrapper::Shutdown but it has not yet been initialized");

	Initialized = false;
	SDL_Quit();

	MLOG_INFO("MEngine terminated gracefully", MENGINE_LOG_CATEGORY_GENERAL);
	MEngineLog::Shutdown();
}

bool MEngine::IsInitialized()
{
	return Initialized;
}

bool MEngine::ShouldQuit()
{
	return QuitRequested;
}

void MEngine::Update()
{
	SDL_Event event;
	while (SDL_PollEvent(&event) != 0)
	{
		if (event.type == SDL_QUIT)
			QuitRequested = true;
	};
}

void MEngine::Render()
{
	SDL_RenderClear(SDLData::GetInstance().Renderer);
	SDL_RenderPresent(SDLData::GetInstance().Renderer);
}