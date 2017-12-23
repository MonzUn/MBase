#pragma once
#include "interface/mengineGraphics.h"
#include <SDL.h>

using namespace MEngineGraphics;

namespace MEngineGraphics
{
	struct MEngineTexture
	{
		MEngineTexture(SDL_Texture* sdlTexture, SDL_Surface* sdlSurface = nullptr) : texture(sdlTexture), surface(sdlSurface)
		{
			SDL_QueryTexture(sdlTexture, &format, &access, &width, &height);
		}

		~MEngineTexture()
		{
			SDL_DestroyTexture(texture);

			if(surface)
				SDL_FreeSurface(surface);
		}

		SDL_Texture*	texture;
		SDL_Surface*	surface;
		int32_t			width;
		int32_t			height;
		uint32_t		format;
		int32_t			access;
	};

	bool Initialize(const char* appName, int32_t windowWidth, int32_t windowHeight);
	MEngineTextureID AddTexture(SDL_Texture* texture, SDL_Surface* optionalSurfaceCopy = nullptr);

	void Render();
	void RenderEntities();

	static SDL_Window*			Window = nullptr;
	static SDL_Renderer*		Renderer = nullptr;
}