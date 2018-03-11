#pragma once
#include "Interface/MEngineInput.h"
#include <SDL.h>

namespace MEngineInput
{
	void Initialize();
	void Shutdown();
	void Update();
	bool HandleEvent(const SDL_Event& sdlEvent);
}