#pragma once

namespace MEngineInput
{
	enum MEngineKey
	{
		MKey_TAB,
		MKey_SHIFT,
		MKey_ALT,
		MKey_CONTROL,
		MKey_GRAVE,

		MKey_T,

		MKEY_COUNT // Not a key but the amount of entries in the enum
	};

	void SetFocusRequired(bool required);


	bool KeyDown(MEngineKey key);		// Is the key pressed down? (Down)
	bool KeyUp(MEngineKey key);			// Is the key not pressed down? (Up)
	bool KeyPressed(MEngineKey key);	// Was the key just pressed down? (Up->Down) 
	bool KeyReleased(MEngineKey key);	// Was the key just released? (Down->Up)
}