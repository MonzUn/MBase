#pragma once
#include "interface/mengineGraphics.h"
#include "interface/mengineColor.h"
#include "interface/mengineTypes.h"
#include <SDL.h>

struct SurfaceToTextureJob;

namespace MEngineGraphics
{
	enum JobTypeMask : uint32_t // TODODB: Incorporate text rendering
	{
		INVALID = 0,

		RECTANGLE = 1 << 0,
		TEXTURE = 1 << 1,
		TEXT = 1 << 2,
	};

	enum class TextRenderMode
	{
		PLAIN,
		BOX,

		INVALID,
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
		RenderJob() {}
		~RenderJob()
		{
			delete[] Text; Text = nullptr;
		}

		RenderJob(const RenderJob& other)
		{
			memcpy(this, &other, sizeof(RenderJob));

			if ((JobMask & TEXT) != 0)
				CopyText(other.Text);
		}

		// Genric
		JobTypeMask JobMask				= JobTypeMask::INVALID;
		SDL_Rect DestinationRect		= {0,0,0,0};
		uint32_t Depth					= 0;

		// Texture
		MEngine::TextureID TextureID	= INVALID_MENGINE_TEXTURE_ID;

		// Rectangle
		MEngine::ColorData FillColor	= MEngine::PredefinedColors::Colors[MEngine::PredefinedColors::TRANSPARENT];
		MEngine::ColorData BorderColor	= MEngine::PredefinedColors::Colors[MEngine::PredefinedColors::TRANSPARENT];

		// Text
		TextRenderMode	TextRenderMode	= TextRenderMode::INVALID;
		MEngine::MEngineFontID	FontID	= INVALID_MENGINE_FONT_ID;
		char*			Text			= nullptr;
		uint64_t		CaretIndex		= -1;
		int32_t			TextWidth		= -1;
		int32_t			TextHeight		= -1;

		void CopyText(const char* str)
		{
			if (Text != nullptr)
				delete[] Text;

			Text = new char[strlen(str) + 1];
			strcpy(Text, str);
		}
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