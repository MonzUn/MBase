#pragma once
#include "interface/mengineGraphics.h"
#include <SDL.h>
#include <vector>

using namespace MEngineGraphics;

namespace MEngineGraphics
{
	struct MEngineTexture
	{
		MEngineTexture(SDL_Texture* sdlTexture) : texture(sdlTexture)
		{
			SDL_QueryTexture(sdlTexture, &format, &access, &width, &height);
		}
	
		SDL_Texture*	texture;
		int32_t			width;
		int32_t			height;
		uint32_t		format;
		int32_t			access;
	};

	bool Initialize(const char* appName, int32_t windowWidth, int32_t windowHeight);
	MEngineTextureID AddTexture(SDL_Texture* texture);

	void Render();
	void RenderEntities();

	static SDL_Window*			Window = nullptr;
	static SDL_Renderer*		Renderer = nullptr;
}