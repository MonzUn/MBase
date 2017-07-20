#pragma once
#include "interface\mengine.h"

struct SDL_Window;
struct SDL_Renderer;

struct SDLData
{
	SDL_Window*			Window 		= nullptr;
	SDL_Renderer*		Renderer 	= nullptr;
};

SDLData g_SDLData;