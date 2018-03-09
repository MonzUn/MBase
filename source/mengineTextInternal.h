#pragma once
#include "interface/mengineTypes.h"
#include <SDL_FontCache.h>

namespace MEngineText
{
	void Initialize();
	void Shutdown();

	FC_Font* GetFont(MEngine::MEngineFontID ID);
}