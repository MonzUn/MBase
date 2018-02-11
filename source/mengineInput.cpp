#include "mengineInputInternal.h"
#include "interface/mengineUtility.h"
#include "scancodes.h"
#include <MUtilityLog.h>
#include <MUtilityPlatformDefinitions.h>
#include <MUtilityWindowsInclude.h>
#include <SDL_ttf.h>
#include <unordered_map>

using namespace MEngineInput;

#define MUTILITY_LOG_CATEGORY_INPUT "MEngineInput"

namespace MEngineInput
{
	bool WindowFocusRequired = true;

	// Key input
	bool PressedKeys[MENGINE_KEY::MKEY_COUNT] = { false };
	bool PreviouslyPressedKeys[MENGINE_KEY::MKEY_COUNT] = { false };
	bool PressedKeysBuffer[MENGINE_KEY::MKEY_COUNT] = { false }; // Used when focus is not required
	std::unordered_map<uint32_t, MENGINE_KEY> ScanCodeToMKeyConversionTable;
	std::unordered_map<SDL_Scancode, MENGINE_KEY> SDLScanCodeToMKeyConversionTable;

	// Text input
	std::string* TextInputString = nullptr;
	uint64_t TextInputCaretIndex = 0;

	// Cursor input
	int32_t CursorPosX		= -1;
	int32_t CursorPosY		= -1;
	int32_t CursorDeltaX	= -1;
	int32_t CursorDeltaY	= -1;
}

#if PLATFORM == PLATFORM_WINDOWS
HHOOK hook = nullptr;
LRESULT HookCallback(int keyCode, WPARAM wParam, LPARAM lParam);
#endif

void PopulateConversionTables();

// ---------- INTERFACE ----------

void MEngineInput::StartTextInput(std::string* textInputString)
{
	if (textInputString == nullptr)
		SDL_StartTextInput();

	TextInputString		= textInputString;
	TextInputCaretIndex = textInputString->length();
}

void MEngineInput::StopTextInput()
{
	if (TextInputString != nullptr)
	{
		SDL_StopTextInput();
		TextInputString			= nullptr;
		TextInputCaretIndex = 0;
	}
	else
		MLOG_WARNING("Attempted to stop text input mode without first starting it", MUTILITY_LOG_CATEGORY_INPUT);
}

void MEngineInput::SetFocusRequired(bool required)
{
#if PLATFORM != PLATFORM_WINDOWS
	static_assert(false, "SetFocusRequired is only supported on the windows platform");
#else
	if (WindowFocusRequired == required)
		return;

	if (!required)
	{
		if (hook = SetWindowsHookEx(WH_KEYBOARD_LL, HookCallback, NULL, 0))
			WindowFocusRequired = false;
		else
			MLOG_ERROR("Failed to initialize non focus key input mode", MUTILITY_LOG_CATEGORY_INPUT);
	}
	else
	{
		UnhookWindowsHookEx(hook);
		memset(&PressedKeysBuffer, false, sizeof(PressedKeysBuffer));
		WindowFocusRequired = true;
	}
#endif
}

bool MEngineInput::KeyDown(MENGINE_KEY key)
{
	return PressedKeys[key];
}

bool MEngineInput::KeyUp(MENGINE_KEY key)
{
	return !PressedKeys[key];
}

bool MEngineInput::KeyPressed(MENGINE_KEY key)
{
	return (!PreviouslyPressedKeys[key] && PressedKeys[key]);
}

bool MEngineInput::KeyReleased(MENGINE_KEY key)
{
	return (PreviouslyPressedKeys[key] && !PressedKeys[key]);
}

int32_t MEngineInput::GetCursorPosX()
{
	return CursorPosX;
}

int32_t MEngineInput::GetCursorPosY()
{
	return CursorPosY;
}

int32_t MEngineInput::GetCursorDeltaX()
{
	return CursorDeltaX;
}

int32_t MEngineInput::GetCursorDeltaY()
{
	return CursorDeltaY;
}

uint64_t MEngineInput::GetTextInputCaretIndex()
{
	return TextInputCaretIndex;
}

// ---------- INTERNAL ----------

void MEngineInput::Initialize()
{
	PopulateConversionTables();
}

void MEngineInput::Update()
{
	memcpy(&PreviouslyPressedKeys, &PressedKeys, sizeof(PressedKeys));
	if (!WindowFocusRequired)
		memcpy(&PressedKeys, &PressedKeysBuffer, sizeof(PressedKeys));

	CursorDeltaX = 0;
	CursorDeltaY = 0;
}

bool MEngineInput::HandleEvent(const SDL_Event& sdlEvent)
{
	bool consumedEvent = false;

	// Handle text input
	// TODODB: Add support for insert
	// TODODB: Add support for marking and handling marked text
	if (TextInputString != nullptr)
	{	
		if (sdlEvent.key.keysym.sym == SDLK_BACKSPACE) // Remove last character
		{
			if (sdlEvent.key.state == SDL_PRESSED && TextInputCaretIndex > 0)
					TextInputString->erase(--TextInputCaretIndex, 1);

			consumedEvent = true;
		}
		else if (sdlEvent.key.keysym.sym == SDLK_c && SDL_GetModState() & KMOD_CTRL) // Clipboard copy
		{
			if (sdlEvent.key.state == SDL_PRESSED)
				SDL_SetClipboardText(TextInputString->c_str());

			consumedEvent = true;
		}
		else if (sdlEvent.key.keysym.sym == SDLK_v && SDL_GetModState() & KMOD_CTRL) // Clipboard paste
		{
			if (sdlEvent.key.state == SDL_PRESSED)
			{
				char* clipboardString = SDL_GetClipboardText();
				TextInputString->insert(TextInputCaretIndex, clipboardString);
				TextInputCaretIndex += strlen(clipboardString);
			}

			consumedEvent = true;
		}
		else if (sdlEvent.key.keysym.sym == SDLK_HOME)
		{
			if (sdlEvent.key.state == SDL_PRESSED)
				TextInputCaretIndex = 0;

			consumedEvent = true;
		}
		else if (sdlEvent.key.keysym.sym == SDLK_END)
		{
			if (sdlEvent.key.state == SDL_PRESSED)
				TextInputCaretIndex = TextInputString->length();

			consumedEvent = true;
		}
		else if(sdlEvent.key.keysym.sym == SDLK_LEFT)
		{
			if (sdlEvent.key.state == SDL_PRESSED && TextInputCaretIndex > 0)
				--TextInputCaretIndex;
		}
		else if (sdlEvent.key.keysym.sym == SDLK_RIGHT)
		{
			if (sdlEvent.key.state == SDL_PRESSED && TextInputCaretIndex < TextInputString->length())
				++TextInputCaretIndex;
		}
		else if (sdlEvent.type == SDL_TEXTINPUT)
		{
			TextInputString->insert(TextInputCaretIndex++, sdlEvent.text.text);
			consumedEvent = true;
		}
	}
	// Handle mouse movement input
	else if (sdlEvent.type == SDL_MOUSEMOTION)
	{
		CursorPosX		= sdlEvent.motion.x;
		CursorPosY		= sdlEvent.motion.y;
		CursorDeltaX	= sdlEvent.motion.xrel;
		CursorDeltaY	= sdlEvent.motion.yrel;
	}
	// Handle keyboard input
	else if (sdlEvent.type == SDL_KEYDOWN || sdlEvent.type == SDL_KEYUP)
	{
		auto scancodeAndMKey = SDLScanCodeToMKeyConversionTable.find(sdlEvent.key.keysym.scancode);
		if (scancodeAndMKey != SDLScanCodeToMKeyConversionTable.end())
			PressedKeys[scancodeAndMKey->second] = (sdlEvent.key.state == SDL_PRESSED);
		else
			MLOG_WARNING("A key was pressed that could not be converted into an MKEY; Scancode = " << sdlEvent.key.keysym.scancode, MUTILITY_LOG_CATEGORY_INPUT);
	}

	return consumedEvent;
}

#if PLATFORM == PLATFORM_WINDOWS
LRESULT HookCallback(int keyCode, WPARAM wParam, LPARAM lParam)
{
	if (!MEngineUtility::WindowHasFocus())
	{
		KBDLLHOOKSTRUCT keyStruct;
		if (keyCode >= HC_ACTION)
		{
			if (wParam == WM_KEYDOWN || wParam == WM_KEYUP)
			{
				keyStruct = *reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
				auto iterator = ScanCodeToMKeyConversionTable.find(keyStruct.scanCode);
				if (iterator != ScanCodeToMKeyConversionTable.end())
					PressedKeysBuffer[iterator->second] = (wParam == WM_KEYDOWN); // Set key pressed to true if wparam is WM_KEYDOWN
			}
		}
	}

	// Call the next hook in the hook chain.
	return CallNextHookEx(hook, keyCode, wParam, lParam);
}
#endif

// ---------- LOCAL ----------

void PopulateConversionTables()  // TODODB: Implement a more efficient way to map scancodes to MKEYs (can make an array with all MKEYs mapped to a scancode if we can calculate an offset for the scancodes to use as index)
{
	// Letters
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_A, MKEY_A));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_B, MKEY_B));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_C, MKEY_C));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_D, MKEY_D));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_E, MKEY_E));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_F, MKEY_F));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_G, MKEY_G));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_H, MKEY_H));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_I, MKEY_I));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_J, MKEY_J));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_K, MKEY_K));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_L, MKEY_L));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_M, MKEY_M));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_N, MKEY_N));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_O, MKEY_O));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_P, MKEY_P));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_Q, MKEY_Q));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_R, MKEY_R));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_T, MKEY_T));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_U, MKEY_U));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_V, MKEY_V));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_W, MKEY_W));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_X, MKEY_X));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_Y, MKEY_Y));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_Z, MKEY_Z));

	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_A, MKEY_A));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_B, MKEY_B));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_C, MKEY_C));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_D, MKEY_D));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_E, MKEY_E));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_F, MKEY_F));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_G, MKEY_G));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_H, MKEY_H));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_I, MKEY_I));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_J, MKEY_J));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_K, MKEY_K));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_L, MKEY_L));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_M, MKEY_M));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_N, MKEY_N));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_O, MKEY_O));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_P, MKEY_P));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_Q, MKEY_Q));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_R, MKEY_R));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_T, MKEY_T));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_U, MKEY_U));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_V, MKEY_V));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_W, MKEY_W));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_X, MKEY_X));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_Y, MKEY_Y));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_Z, MKEY_Z));

	// Numeric
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_1, MKEY_NUMROW_1));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_2, MKEY_NUMROW_2));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_3, MKEY_NUMROW_3));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_4, MKEY_NUMROW_4));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_5, MKEY_NUMROW_5));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_6, MKEY_NUMROW_6));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_7, MKEY_NUMROW_7));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_8, MKEY_NUMROW_8));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_9, MKEY_NUMROW_9));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_0, MKEY_NUMROW_0));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_NUMPAD_0, MKEY_NUMPAD_1));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_NUMPAD_1, MKEY_NUMPAD_2));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_NUMPAD_2, MKEY_NUMPAD_3));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_NUMPAD_3, MKEY_NUMPAD_4));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_NUMPAD_4, MKEY_NUMPAD_5));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_NUMPAD_5, MKEY_NUMPAD_6));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_NUMPAD_6, MKEY_NUMPAD_7));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_NUMPAD_7, MKEY_NUMPAD_8));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_NUMPAD_8, MKEY_NUMPAD_9));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_NUMPAD_9, MKEY_NUMPAD_0));

	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_1, MKEY_NUMROW_1));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_2, MKEY_NUMROW_2));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_3, MKEY_NUMROW_3));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_4, MKEY_NUMROW_4));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_5, MKEY_NUMROW_5));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_6, MKEY_NUMROW_6));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_7, MKEY_NUMROW_7));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_8, MKEY_NUMROW_8));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_9, MKEY_NUMROW_9));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_0, MKEY_NUMROW_0));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_KP_0, MKEY_NUMPAD_1));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_KP_1, MKEY_NUMPAD_2));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_KP_2, MKEY_NUMPAD_3));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_KP_3, MKEY_NUMPAD_4));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_KP_4, MKEY_NUMPAD_5));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_KP_5, MKEY_NUMPAD_6));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_KP_6, MKEY_NUMPAD_7));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_KP_7, MKEY_NUMPAD_8));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_KP_8, MKEY_NUMPAD_9));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_KP_9, MKEY_NUMPAD_0));

	// Function keys
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_F1, MKEY_F1));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_F2, MKEY_F2));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_F3, MKEY_F3));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_F4, MKEY_F4));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_F5, MKEY_F5));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_F6, MKEY_F6));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_F7, MKEY_F7));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_F8, MKEY_F8));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_F9, MKEY_F9));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_F10, MKEY_F10));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_F11, MKEY_F11));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_F12, MKEY_F12));

	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_F1, MKEY_F1));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_F2, MKEY_F2));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_F3, MKEY_F3));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_F4, MKEY_F4));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_F5, MKEY_F5));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_F6, MKEY_F6));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_F7, MKEY_F7));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_F8, MKEY_F8));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_F9, MKEY_F9));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_F10, MKEY_F10));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_F11, MKEY_F11));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_F12, MKEY_F12));

	// Modifiers
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_LEFT_SHIFT, MKEY_LEFT_SHIFT));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_RIGHT_SHIFT, MKEY_RIGHT_SHIFT));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_LEFT_ALT, MKEY_LEFT_ALT));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_RIGHT_ALT, MKEY_RIGHT_ALT));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_LEFT_CONTROL, MKEY_LEFT_CONTROL));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_RIGHT_CONTROL, MKEY_RIGHT_CONTROL));

	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_LSHIFT, MKEY_LEFT_SHIFT));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_RSHIFT, MKEY_RIGHT_SHIFT));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_LALT, MKEY_LEFT_ALT));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_RALT, MKEY_RIGHT_ALT));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_LCTRL, MKEY_LEFT_CONTROL));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_RCTRL, MKEY_RIGHT_CONTROL));

	// Special
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_TAB, MKEY_TAB));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_GRAVE, MKEY_GRAVE));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_CAPSLOCK, MKEY_CAPS_LOCK));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_ANGLED_BRACKET, MKEY_ANGLED_BRACKET));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_NUMPAD_ENTER, MKEY_NUMPAD_ENTER));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_ENTER, MKEY_MAIN_ENTER));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_EQUALS, MKEY_EQUALS));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_MINUS, MKEY_MINUS));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_NUMPAD_PLUS, MKEY_NUMPAD_PLUS));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_NUMPAD_MINUS, MKEY_NUMPAD_MINUS));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_NUMPAD_ASTERISK, MKEY_NUMPAD_ASTERISK));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_NUMPAD_SLASH, MKEY_NUMPAD_SLASH));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_INSERT, MKEY_INSERT));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_DELETE, MKEY_DELETE));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_HOME, MKEY_HOME));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_END, MKEY_END));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_PAGE_UP, MKEY_PAGE_UP));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_PAGE_DOWN, MKEY_PAGE_DOWN));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_PRINTSCREEN, MKEY_PRINTSCREEN));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_SCROLL_LOCK, MKEY_SCROLL_LOCK));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_PAUSE, MKEY_PAUSE_BREAK));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_NUM_LOCK, MKEY_NUM_LOCK));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_COMMA, MKEY_COMMA));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_NUMPAD_PERIOD, MKEY_NUMPAD_COMMA));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_PERIOD, MKEY_PERIOD));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_SLASH, MKEY_SLASH));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_APOSTROPHE, MKEY_APSTROPHE));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_SEMICOLON, MKEY_SEMICOLON));

	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_TAB, MKEY_TAB));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_GRAVE, MKEY_GRAVE));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_CAPSLOCK, MKEY_CAPS_LOCK));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_NONUSBACKSLASH, MKEY_ANGLED_BRACKET));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_KP_ENTER, MKEY_NUMPAD_ENTER));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_RETURN, MKEY_MAIN_ENTER));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_EQUALS, MKEY_EQUALS));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_MINUS, MKEY_MINUS));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_KP_PLUS, MKEY_NUMPAD_PLUS));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_KP_MINUS, MKEY_NUMPAD_MINUS));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_KP_MULTIPLY, MKEY_NUMPAD_ASTERISK));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_KP_DIVIDE, MKEY_NUMPAD_SLASH));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_INSERT, MKEY_INSERT));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_DELETE, MKEY_DELETE));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_HOME, MKEY_HOME));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_END, MKEY_END));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_PAGEUP, MKEY_PAGE_UP));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_PAGEDOWN, MKEY_PAGE_DOWN));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_PRINTSCREEN, MKEY_PRINTSCREEN));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_SCROLLLOCK, MKEY_SCROLL_LOCK));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_PAUSE, MKEY_PAUSE_BREAK));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_NUMLOCKCLEAR, MKEY_NUM_LOCK));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_COMMA, MKEY_COMMA));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_KP_COMMA, MKEY_NUMPAD_COMMA));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_PERIOD, MKEY_PERIOD));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_SLASH, MKEY_SLASH));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_APOSTROPHE, MKEY_APSTROPHE));
	SDLScanCodeToMKeyConversionTable.insert(std::make_pair(SDL_SCANCODE_SEMICOLON, MKEY_SEMICOLON));
}