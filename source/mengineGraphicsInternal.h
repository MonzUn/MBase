#pragma once
#include "mengineData.h"
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

	MEngineTextureID AddTexture(SDL_Texture* texture);

	void RenderEntities();
}