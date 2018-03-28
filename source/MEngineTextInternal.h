#pragma once
#include "Interface/MEngineTypes.h"
#include <SDL_FontCache.h>

namespace MEngineText
{
	void Initialize();
	void Shutdown();

	FC_Font* GetFont(MEngine::FontID ID);
}