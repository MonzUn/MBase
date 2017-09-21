#pragma once
#include "interface\mengine.h"

struct SDL_Window;
struct SDL_Renderer;

class SDLData
{
public:
	static SDLData& GetInstance() { static SDLData instance; return instance; };

	SDL_Window*			Window		= nullptr;
	SDL_Renderer*		Renderer	= nullptr;
private:
	SDLData() {};
};