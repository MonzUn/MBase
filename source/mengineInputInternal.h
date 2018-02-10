#pragma once
#include "interface/mengineInput.h"
#include <SDL.h>

namespace MEngineInput
{
	void Initialize();
	void Update();
	bool HandleEvent(const SDL_Event& sdlEvent);
}