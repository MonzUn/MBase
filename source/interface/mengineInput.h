#pragma once
#include "mengineInputKeys.h"

namespace MEngineInput
{
	void SetFocusRequired(bool required);

	bool KeyDown(MENGINE_KEY key);		// Is the key pressed down? (Down)
	bool KeyUp(MENGINE_KEY key);		// Is the key not pressed down? (Up)
	bool KeyPressed(MENGINE_KEY key);	// Was the key just pressed down? (Up->Down) 
	bool KeyReleased(MENGINE_KEY key);	// Was the key just released? (Down->Up)
}