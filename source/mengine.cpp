#include "interface/mengine.h"

#include "interface/mengineLog.h"
#include "mengineData.h"
#include "mengineLogInternal.h"

#include <cassert>
#include <iostream>
#include <SDL.h>

#define MENGINE_LOG_CATEGORY_GENERAL "MEngine"

namespace
{
	bool Initialized = false;
	bool QuitRequested = false;
}

bool MEngine::Initialize()
{
	assert(!IsInitialized() && "Calling SDLWrapper::Initialize but it has already been initialized");

	MEngineLog::Initialize();
	MEngineLog::RegisterCategory(MENGINE_LOG_CATEGORY_GENERAL);

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		MLOG_ERROR("MEngine initialization failed; SDL_Init Error: " + std::string(SDL_GetError()) + "; program will close", MENGINE_LOG_CATEGORY_GENERAL);
		return false;
	}

	g_SDLData.Window = SDL_CreateWindow("BamboBlocks", 100, 100, 640, 480, SDL_WINDOW_SHOWN);
	if (g_SDLData.Window == nullptr)
	{
		MLOG_ERROR("MEngine initialization failed; SDL_CreateWindow Error: " + std::string(SDL_GetError()) + "; program will close", MENGINE_LOG_CATEGORY_GENERAL);
		return false;
	}

	g_SDLData.Renderer = SDL_CreateRenderer(g_SDLData.Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (g_SDLData.Renderer == nullptr)
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
	SDL_RenderClear(g_SDLData.Renderer);
	SDL_RenderPresent(g_SDLData.Renderer);
}