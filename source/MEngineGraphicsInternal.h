#pragma once
#include "Interface/MEngineGraphics.h"
#include "Interface/MEngineColor.h"
#include "Interface/MEngineTypes.h"
#include <MUtilityBitset.h>
#include <SDL.h>
#include <SDL_FontCache.h>

struct SurfaceToTextureJob;

namespace MEngineGraphics
{
	enum class JobTypeMask : MUtility::BitSet
	{
		INVALID	= 0,

		RECTANGLE	= 1 << 0,
		TEXTURE		= 1 << 1,
		TEXT		= 1 << 2,
		CARET		= 1 << 3,
	};
	CREATE_BITFLAG_OPERATOR_SIGNATURES(JobTypeMask);
	
	enum class TextRenderMode
	{
		PLAIN,
		BOX,

		INVALID,
	};

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
		~RenderJob(){ delete[] Text; Text = nullptr; }
		RenderJob(const RenderJob& other) = delete;

		// Genric
		JobTypeMask JobMask				= JobTypeMask::INVALID;
		SDL_Rect DestinationRect		= {0,0,0,0};
		uint32_t Depth					= 0;

		// Texture
		MEngine::TextureID TextureID;

		// Rectangle
		MEngine::ColorData FillColor	= MEngine::PredefinedColors::Colors[MEngine::PredefinedColors::TRANSPARENT_];
		MEngine::ColorData BorderColor	= MEngine::PredefinedColors::Colors[MEngine::PredefinedColors::TRANSPARENT_];

		// Text
		TextRenderMode	TextRenderMode			= TextRenderMode::INVALID;
		MEngine::FontID	FontID;
		char*			Text					= nullptr;
		FC_AlignEnum	HorizontalTextAlignment = FC_ALIGN_LEFT;
		SDL_Rect		TextRect				= {0,0,0,0};
		int32_t			CaretOffsetX			= -1;

		void CopyText(const char* str)
		{
			if (Text != nullptr)
				delete[] Text;

			Text = new char[strlen(str) + 1];
			strcpy(Text, str);
		}
	};

	bool Initialize(const char* appName, int32_t windowPosX, int32_t windowPosY, int32_t windowWidth, int32_t windowHeight);
	void Shutdown();

	MEngine::TextureID AddTexture(SDL_Texture* texture, SDL_Surface* optionalSurfaceCopy = nullptr, MEngine::TextureID reservedTextureID = MEngine::TextureID::Invalid());
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
	MEngine::TextureID ReservedID;
	bool StoreSurfaceInRAM			= false;
};