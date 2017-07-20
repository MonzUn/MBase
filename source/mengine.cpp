#include "interface\mengine.h"

#include <iostream>
#include <SDL.h>

#include "mengineAssert.h"
#include "mengineData.h"

bool Initialized = false;
bool QuitRequested = false;

bool MEngine::Initialize()
{
	assert(!IsInitialized() && "Calling MEngine::Initialize but it has already been initialized");

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return false;
	}

	g_SDLData.Window = SDL_CreateWindow("Hello World!", 100, 100, 640, 480, SDL_WINDOW_SHOWN);
	if (g_SDLData.Window == nullptr)
	{
		std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		return false;
	}

	g_SDLData.Renderer = SDL_CreateRenderer(g_SDLData.Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (g_SDLData.Renderer == nullptr)
	{
		std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
		return false;
	}

	Initialized = true;
	return true;
}

void MEngine::Shutdown()
{
	ASSERT_MEngine_INITIALIZED

	Initialized = false;
	SDL_Quit();
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
	ASSERT_MEngine_INITIALIZED

	SDL_Event event;
	while (SDL_PollEvent(&event) != 0)
	{
		if (event.type == SDL_QUIT)
			QuitRequested = true;
	};
}

void MEngine::Render()
{
	ASSERT_MEngine_INITIALIZED

	SDL_RenderClear(g_SDLData.Renderer);
	SDL_RenderPresent(g_SDLData.Renderer);
}