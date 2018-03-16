#pragma once
#include "MEngineInputKeys.h"
#include <string>

namespace MEngine
{
	void StartTextInput(std::string* textInputString);
	void StopTextInput();

	void SetFocusRequired(bool required);

	bool KeyDown(MENGINE_KEY key);		// Is the key pressed down? (Down)
	bool KeyUp(MENGINE_KEY key);		// Is the key not pressed down? (Up)
	bool KeyPressed(MENGINE_KEY key);	// Was the key just pressed down? (Up->Down) 
	bool KeyReleased(MENGINE_KEY key);	// Was the key just released? (Down->Up)
	bool ScrolledUp();
	bool ScrolledDown();

	int32_t GetCursorPosX();
	int32_t GetCursorPosY();
	int32_t GetCursorDeltaX();
	int32_t GetCursorDeltaY();
	int32_t GetScrollValue();

	uint64_t GetTextInputCaretIndex();

	bool IsTextInputActive();
	bool IsInputString(const std::string* toCompare); // Pointer comparsion
}