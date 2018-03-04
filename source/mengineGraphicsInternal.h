#pragma once
#include "interface/mengineGraphics.h"
#include <SDL.h>

struct SurfaceToTextureJob;

namespace MEngineGraphics
{
	struct MEngineTexture
	{
		MEngineTexture(SDL_Texture* sdlTexture, SDL_Surface* sdlSurface = nullptr) : Texture(sdlTexture), Surface(sdlSurface)
		{
			SDL_QueryTexture(sdlTexture, &Format, &Access, &Width, &Height);
		}

		MEngineTexture(const MEngineTexture& other) = delete;

		~MEngineTexture()
		{
			SDL_DestroyTexture(Texture);

			if(Surface)
				SDL_FreeSurface(Surface);
		}

		SDL_Texture*	Texture;
		SDL_Surface*	Surface;
		int32_t			Width;
		int32_t			Height;
		uint32_t		Format;
		int32_t			Access;
	};

	bool Initialize(const char* appName, int32_t windowWidth, int32_t windowHeight);
	void Shutdown();
	MEngine::TextureID AddTexture(SDL_Texture* texture, SDL_Surface* optionalSurfaceCopy = nullptr, MEngine::TextureID reservedTextureID = INVALID_MENGINE_TEXTURE_ID);
	void HandleSurfaceToTextureConversions();
	MEngine::TextureID GetNextTextureID();

	SDL_Renderer*	GetRenderer();
	SDL_Window*		GetWindow();

	void Render();
	void RenderRectangles();
	void RenderTextures();
}

struct SurfaceToTextureJob
{
	SurfaceToTextureJob(SDL_Surface* surface, MEngine::TextureID reservedID, bool storeSurfaceInRAM) :
	Surface(surface), ReservedID(reservedID), StoreSurfaceInRAM(storeSurfaceInRAM) {}

	SDL_Surface* Surface			= nullptr;
	MEngine::TextureID ReservedID	= INVALID_MENGINE_TEXTURE_ID;
	bool StoreSurfaceInRAM			= false;
};