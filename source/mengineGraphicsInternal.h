#pragma once
#include "interface/mengineGraphics.h"
#include "interface/mengineColor.h"
#include <SDL.h>

struct SurfaceToTextureJob;

namespace MEngineGraphics
{
	enum JobTypeMask : uint32_t // TODODB: Incorporate text rendering
	{
		INVALID = 0,

		RECTANGLE = 1 << 0,
		TEXTURE = 1 << 1,
	};

	inline JobTypeMask& operator |=(JobTypeMask& a, JobTypeMask b)
	{
		return a = static_cast<JobTypeMask>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
	}

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

	struct RenderJob
	{
		JobTypeMask JobMask				= JobTypeMask::INVALID;
		SDL_Rect DestinationRect		= {0,0,0,0};
		MEngine::TextureID TextureID	= INVALID_MENGINE_TEXTURE_ID;
		MEngine::ColorData FillColor	= MEngine::PredefinedColors::Colors[MEngine::PredefinedColors::TRANSPARENT];
		MEngine::ColorData BorderColor	= MEngine::PredefinedColors::Colors[MEngine::PredefinedColors::TRANSPARENT];
	};

	bool Initialize(const char* appName, int32_t windowWidth, int32_t windowHeight);
	void Shutdown();
	MEngine::TextureID AddTexture(SDL_Texture* texture, SDL_Surface* optionalSurfaceCopy = nullptr, MEngine::TextureID reservedTextureID = INVALID_MENGINE_TEXTURE_ID);
	void HandleSurfaceToTextureConversions();
	MEngine::TextureID GetNextTextureID();

	SDL_Renderer*	GetRenderer();
	SDL_Window*		GetWindow();

	void Render();
}

struct SurfaceToTextureJob
{
	SurfaceToTextureJob(SDL_Surface* surface, MEngine::TextureID reservedID, bool storeSurfaceInRAM) :
	Surface(surface), ReservedID(reservedID), StoreSurfaceInRAM(storeSurfaceInRAM) {}

	SDL_Surface* Surface			= nullptr;
	MEngine::TextureID ReservedID	= INVALID_MENGINE_TEXTURE_ID;
	bool StoreSurfaceInRAM			= false;
};