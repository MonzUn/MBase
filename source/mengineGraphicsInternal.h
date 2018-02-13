#pragma once
#include "interface/mengineGraphics.h"
#include <MUtilityLocklessQueue.h>
#include <SDL.h>

using namespace MEngineGraphics;

struct SurfaceToTextureJob;

namespace MEngineGraphics
{
	struct MEngineTexture // TODODB: Fix casing on member variables
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
	void Shutdown();
	MEngineTextureID AddTexture(SDL_Texture* texture, SDL_Surface* optionalSurfaceCopy = nullptr, MEngineTextureID reservedTextureID = INVALID_MENGINE_TEXTURE_ID);
	void HandleSurfaceToTextureConversions();
	MEngineTextureID GetNextTextureID();

	SDL_Renderer*	GetRenderer();
	SDL_Window*		GetWindow();

	void Render();
	void RenderEntities();
}

struct SurfaceToTextureJob
{
	SurfaceToTextureJob(SDL_Surface* surface, MEngineTextureID reservedID, bool storeSurfaceInRAM) :
	Surface(surface), ReservedID(reservedID), StoreSurfaceInRAM(storeSurfaceInRAM) {}

	SDL_Surface* Surface		= nullptr;
	MEngineTextureID ReservedID = INVALID_MENGINE_TEXTURE_ID;
	bool StoreSurfaceInRAM		= false;
};