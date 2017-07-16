#include "mengine.h"

#include <cassert>
#include <iostream>
#include <SDL.h>

bool MEngine::Initialize()
{
	assert(!m_Initialized && "Calling SDLWrapper::Initialize but it has already been initialized");

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return false;
	}

	m_Window = SDL_CreateWindow("Hello World!", 100, 100, 640, 480, SDL_WINDOW_SHOWN);
	if (m_Window == nullptr)
	{
		std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		return false;
	}

	m_Renderer = SDL_CreateRenderer(m_Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (m_Renderer == nullptr)
	{
		std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
		return false;
	}

	m_Initialized = true;
	return true;
}

void MEngine::Shutdown()
{
	assert(m_Initialized && "Calling SDLWrapper::Shutdown but it has not yet been initialized");

	m_Initialized = false;
	SDL_Quit();
}

bool MEngine::ShouldQuit() const
{
	return m_QuitRequested;
}

void MEngine::Update()
{
	SDL_Event event;
	while (SDL_PollEvent(&event) != 0)
	{
		if (event.type == SDL_QUIT)
			m_QuitRequested = true;
	};
}

void MEngine::Render()
{
	SDL_RenderClear(m_Renderer);
	SDL_RenderPresent(m_Renderer);
}