#include "mengineInputInternal.h"
#include "interface/mengineUtility.h"
#include "scancodes.h"
#include <MUtilityLog.h>
#include <MUtilityPlatformDefinitions.h>
#include <MUtilityWindowsInclude.h>
#include <SDL_ttf.h>
#include <unordered_map>

using namespace MEngineInput;

#define LOG_CATEGORY_INPUT "MEngineInput"

namespace MEngineInput
{
	bool m_WindowFocusRequired = true;

	// Key input
	bool m_PressedKeys[MENGINE_KEY::MKEY_COUNT] = { false };
	bool m_PreviouslyPressedKeys[MENGINE_KEY::MKEY_COUNT] = { false };
	bool m_PressedKeysBuffer[MENGINE_KEY::MKEY_COUNT] = { false }; // Used when focus is not required
	std::unordered_map<uint32_t, MENGINE_KEY>* m_ScanCodeToMKeyConversionTable;
	std::unordered_map<SDL_Scancode, MENGINE_KEY>* m_SDLScanCodeToMKeyConversionTable;

	// Text input
	std::string* m_TextInputStringReference = nullptr; // TODODB: Refactor text input handling
	uint64_t m_TextInputCaretIndex = 0;

	// Cursor input
	int32_t m_CursorPosX	= -1;
	int32_t m_CursorPosY	= -1;
	int32_t m_CursorDeltaX	= -1;
	int32_t m_CursorDeltaY	= -1;
}

using namespace MEngine;
using namespace MEngineInput;

#if PLATFORM == PLATFORM_WINDOWS
HHOOK hook = nullptr;
LRESULT HookCallback(int keyCode, WPARAM wParam, LPARAM lParam);
#endif

void PopulateConversionTables();

// ---------- INTERFACE ----------

void MEngine::StartTextInput(std::string* textInputString)
{
	if (textInputString == nullptr)
		SDL_StartTextInput();

	m_TextInputStringReference = textInputString;
	m_TextInputCaretIndex = textInputString->length();
}

void MEngine::StopTextInput()
{
	if (m_TextInputStringReference != nullptr)
	{
		SDL_StopTextInput();
		m_TextInputStringReference = nullptr;
		m_TextInputCaretIndex = 0;
	}
	else
		MLOG_WARNING("Attempted to stop text input mode without first starting it", LOG_CATEGORY_INPUT);
}

void MEngine::SetFocusRequired(bool required)
{
#if PLATFORM != PLATFORM_WINDOWS
	static_assert(false, "SetFocusRequired is only supported on the windows platform");
#else
	if (m_WindowFocusRequired == required)
		return;

	if (!required)
	{
		if (hook = SetWindowsHookEx(WH_KEYBOARD_LL, HookCallback, NULL, 0))
			m_WindowFocusRequired = false;
		else
			MLOG_ERROR("Failed to initialize non focus key input mode", LOG_CATEGORY_INPUT);
	}
	else
	{
		UnhookWindowsHookEx(hook);
		memset(&m_PressedKeysBuffer, false, sizeof(m_PressedKeysBuffer));
		m_WindowFocusRequired = true;
	}
#endif
}

bool MEngine::KeyDown(MENGINE_KEY key)
{
	return m_PressedKeys[key];
}

bool MEngine::KeyUp(MENGINE_KEY key)
{
	return !m_PressedKeys[key];
}

bool MEngine::KeyPressed(MENGINE_KEY key)
{
	return (!m_PreviouslyPressedKeys[key] && m_PressedKeys[key]);
}

bool MEngine::KeyReleased(MENGINE_KEY key)
{
	return (m_PreviouslyPressedKeys[key] && !m_PressedKeys[key]);
}

int32_t MEngine::GetCursorPosX()
{
	return m_CursorPosX;
}

int32_t MEngine::GetCursorPosY()
{
	return m_CursorPosY;
}

int32_t MEngine::GetCursorDeltaX()
{
	return m_CursorDeltaX;
}

int32_t MEngine::GetCursorDeltaY()
{
	return m_CursorDeltaY;
}

uint64_t MEngine::GetTextInputCaretIndex()
{
	return m_TextInputCaretIndex;
}

bool MEngine::IsTextInputActive()
{
	return m_TextInputStringReference != nullptr;
}

bool MEngine::IsInputString(const std::string* toCompare)
{
	return m_TextInputStringReference == toCompare;
}

// ---------- INTERNAL ----------

void MEngineInput::Initialize()
{
	m_ScanCodeToMKeyConversionTable = new std::unordered_map<uint32_t, MENGINE_KEY>();
	m_SDLScanCodeToMKeyConversionTable = new std::unordered_map<SDL_Scancode, MENGINE_KEY>();

	PopulateConversionTables();
}

void MEngineInput::Shutdown()
{
	delete m_ScanCodeToMKeyConversionTable;
	delete m_SDLScanCodeToMKeyConversionTable;
}

void MEngineInput::Update()
{
	memcpy(&m_PreviouslyPressedKeys, &m_PressedKeys, sizeof(m_PressedKeys));
	if (!m_WindowFocusRequired)
		memcpy(&m_PressedKeys, &m_PressedKeysBuffer, sizeof(m_PressedKeys));

	m_CursorDeltaX = 0;
	m_CursorDeltaY = 0;
}

bool MEngineInput::HandleEvent(const SDL_Event& sdlEvent)
{
	bool consumedEvent = false;

	// Handle text input
	// TODODB: Add support for insert
	// TODODB: Add support for marking and handling marked text
	// TODODB: Move this code somewhere else and use MKEY interface instead
	if (m_TextInputStringReference != nullptr)
	{	
		if (sdlEvent.key.keysym.sym == SDLK_BACKSPACE) // Remove last character
		{
			if (sdlEvent.key.state == SDL_PRESSED && m_TextInputCaretIndex > 0)
				m_TextInputStringReference->erase(--m_TextInputCaretIndex, 1);

			consumedEvent = true;
		}
		else if (sdlEvent.key.keysym.sym == SDLK_c && SDL_GetModState() & KMOD_CTRL) // Clipboard copy
		{
			if (sdlEvent.key.state == SDL_PRESSED)
				SDL_SetClipboardText(m_TextInputStringReference->c_str());

			consumedEvent = true;
		}
		else if (sdlEvent.key.keysym.sym == SDLK_v && SDL_GetModState() & KMOD_CTRL) // Clipboard paste
		{
			if (sdlEvent.key.state == SDL_PRESSED)
			{
				char* clipboardString = SDL_GetClipboardText();
				m_TextInputStringReference->insert(m_TextInputCaretIndex, clipboardString);
				m_TextInputCaretIndex += strlen(clipboardString);
			}

			consumedEvent = true;
		}
		else if (sdlEvent.key.keysym.sym == SDLK_HOME)
		{
			if (sdlEvent.key.state == SDL_PRESSED)
				m_TextInputCaretIndex = 0;

			consumedEvent = true;
		}
		else if (sdlEvent.key.keysym.sym == SDLK_END)
		{
			if (sdlEvent.key.state == SDL_PRESSED)
				m_TextInputCaretIndex = m_TextInputStringReference->length();

			consumedEvent = true;
		}
		else if(sdlEvent.key.keysym.sym == SDLK_LEFT)
		{
			if (sdlEvent.key.state == SDL_PRESSED && m_TextInputCaretIndex > 0)
				--m_TextInputCaretIndex;
		}
		else if (sdlEvent.key.keysym.sym == SDLK_RIGHT)
		{
			if (sdlEvent.key.state == SDL_PRESSED && m_TextInputCaretIndex < m_TextInputStringReference->length())
				++m_TextInputCaretIndex;
		}
		else if (sdlEvent.type == SDL_TEXTINPUT)
		{
			m_TextInputStringReference->insert(m_TextInputCaretIndex++, sdlEvent.text.text);
			consumedEvent = true;
		}
	}
	// Handle mouse movement input
	else if (sdlEvent.type == SDL_MOUSEMOTION)
	{
		m_CursorPosX	= sdlEvent.motion.x;
		m_CursorPosY	= sdlEvent.motion.y;
		m_CursorDeltaX	= sdlEvent.motion.xrel;
		m_CursorDeltaY	= sdlEvent.motion.yrel;
	}
	// Handle mouse button input
	else if (sdlEvent.type == SDL_MOUSEBUTTONDOWN || sdlEvent.type == SDL_MOUSEBUTTONUP)
	{
		m_PressedKeys[MKEY_MOUSE_LEFT + sdlEvent.button.button - 1 ] = (sdlEvent.button.state == SDL_PRESSED);
	}
	// Handle keyboard input
	else if (sdlEvent.type == SDL_KEYDOWN || sdlEvent.type == SDL_KEYUP)
	{
		auto scancodeAndMKey = m_SDLScanCodeToMKeyConversionTable->find(sdlEvent.key.keysym.scancode);
		if (scancodeAndMKey != m_SDLScanCodeToMKeyConversionTable->end())
			m_PressedKeys[scancodeAndMKey->second] = (sdlEvent.key.state == SDL_PRESSED);
		else
			MLOG_WARNING("A key was pressed that could not be converted into an MKEY; Scancode = " << sdlEvent.key.keysym.scancode, LOG_CATEGORY_INPUT);
	}

	return consumedEvent;
}

#if PLATFORM == PLATFORM_WINDOWS
LRESULT HookCallback(int keyCode, WPARAM wParam, LPARAM lParam)
{
	if (!MEngine::WindowHasFocus())
	{
		KBDLLHOOKSTRUCT keyStruct;
		if (keyCode >= HC_ACTION)
		{
			if (wParam == WM_KEYDOWN || wParam == WM_KEYUP)
			{
				keyStruct = *reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
				auto iterator = m_ScanCodeToMKeyConversionTable->find(keyStruct.scanCode);
				if (iterator != m_ScanCodeToMKeyConversionTable->end())
					m_PressedKeysBuffer[iterator->second] = (wParam == WM_KEYDOWN); // Set key pressed to true if wparam is WM_KEYDOWN
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
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_A, MKEY_A));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_B, MKEY_B));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_C, MKEY_C));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_D, MKEY_D));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_E, MKEY_E));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_F, MKEY_F));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_G, MKEY_G));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_H, MKEY_H));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_I, MKEY_I));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_J, MKEY_J));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_K, MKEY_K));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_L, MKEY_L));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_M, MKEY_M));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_N, MKEY_N));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_O, MKEY_O));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_P, MKEY_P));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_Q, MKEY_Q));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_R, MKEY_R));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_T, MKEY_T));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_U, MKEY_U));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_V, MKEY_V));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_W, MKEY_W));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_X, MKEY_X));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_Y, MKEY_Y));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_Z, MKEY_Z));

	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_A, MKEY_A));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_B, MKEY_B));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_C, MKEY_C));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_D, MKEY_D));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_E, MKEY_E));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_F, MKEY_F));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_G, MKEY_G));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_H, MKEY_H));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_I, MKEY_I));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_J, MKEY_J));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_K, MKEY_K));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_L, MKEY_L));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_M, MKEY_M));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_N, MKEY_N));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_O, MKEY_O));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_P, MKEY_P));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_Q, MKEY_Q));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_R, MKEY_R));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_T, MKEY_T));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_U, MKEY_U));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_V, MKEY_V));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_W, MKEY_W));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_X, MKEY_X));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_Y, MKEY_Y));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_Z, MKEY_Z));

	// Numeric
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_1, MKEY_NUMROW_1));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_2, MKEY_NUMROW_2));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_3, MKEY_NUMROW_3));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_4, MKEY_NUMROW_4));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_5, MKEY_NUMROW_5));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_6, MKEY_NUMROW_6));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_7, MKEY_NUMROW_7));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_8, MKEY_NUMROW_8));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_9, MKEY_NUMROW_9));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_0, MKEY_NUMROW_0));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_NUMPAD_0, MKEY_NUMPAD_1));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_NUMPAD_1, MKEY_NUMPAD_2));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_NUMPAD_2, MKEY_NUMPAD_3));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_NUMPAD_3, MKEY_NUMPAD_4));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_NUMPAD_4, MKEY_NUMPAD_5));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_NUMPAD_5, MKEY_NUMPAD_6));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_NUMPAD_6, MKEY_NUMPAD_7));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_NUMPAD_7, MKEY_NUMPAD_8));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_NUMPAD_8, MKEY_NUMPAD_9));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_NUMPAD_9, MKEY_NUMPAD_0));

	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_1, MKEY_NUMROW_1));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_2, MKEY_NUMROW_2));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_3, MKEY_NUMROW_3));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_4, MKEY_NUMROW_4));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_5, MKEY_NUMROW_5));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_6, MKEY_NUMROW_6));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_7, MKEY_NUMROW_7));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_8, MKEY_NUMROW_8));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_9, MKEY_NUMROW_9));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_0, MKEY_NUMROW_0));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_KP_0, MKEY_NUMPAD_1));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_KP_1, MKEY_NUMPAD_2));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_KP_2, MKEY_NUMPAD_3));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_KP_3, MKEY_NUMPAD_4));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_KP_4, MKEY_NUMPAD_5));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_KP_5, MKEY_NUMPAD_6));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_KP_6, MKEY_NUMPAD_7));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_KP_7, MKEY_NUMPAD_8));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_KP_8, MKEY_NUMPAD_9));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_KP_9, MKEY_NUMPAD_0));

	// Function keys
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_F1, MKEY_F1));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_F2, MKEY_F2));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_F3, MKEY_F3));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_F4, MKEY_F4));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_F5, MKEY_F5));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_F6, MKEY_F6));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_F7, MKEY_F7));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_F8, MKEY_F8));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_F9, MKEY_F9));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_F10, MKEY_F10));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_F11, MKEY_F11));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_F12, MKEY_F12));

	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_F1, MKEY_F1));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_F2, MKEY_F2));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_F3, MKEY_F3));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_F4, MKEY_F4));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_F5, MKEY_F5));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_F6, MKEY_F6));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_F7, MKEY_F7));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_F8, MKEY_F8));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_F9, MKEY_F9));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_F10, MKEY_F10));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_F11, MKEY_F11));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_F12, MKEY_F12));

	// Modifiers
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_LEFT_SHIFT, MKEY_LEFT_SHIFT));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_RIGHT_SHIFT, MKEY_RIGHT_SHIFT));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_LEFT_ALT, MKEY_LEFT_ALT));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_RIGHT_ALT, MKEY_RIGHT_ALT));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_LEFT_CONTROL, MKEY_LEFT_CONTROL));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_RIGHT_CONTROL, MKEY_RIGHT_CONTROL));

	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_LSHIFT, MKEY_LEFT_SHIFT));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_RSHIFT, MKEY_RIGHT_SHIFT));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_LALT, MKEY_LEFT_ALT));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_RALT, MKEY_RIGHT_ALT));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_LCTRL, MKEY_LEFT_CONTROL));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_RCTRL, MKEY_RIGHT_CONTROL));

	// Special
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_TAB, MKEY_TAB));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_GRAVE, MKEY_GRAVE));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_CAPSLOCK, MKEY_CAPS_LOCK));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_ANGLED_BRACKET, MKEY_ANGLED_BRACKET));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_NUMPAD_ENTER, MKEY_NUMPAD_ENTER));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_ENTER, MKEY_MAIN_ENTER));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_EQUALS, MKEY_EQUALS));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_MINUS, MKEY_MINUS));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_NUMPAD_PLUS, MKEY_NUMPAD_PLUS));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_NUMPAD_MINUS, MKEY_NUMPAD_MINUS));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_NUMPAD_ASTERISK, MKEY_NUMPAD_ASTERISK));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_NUMPAD_SLASH, MKEY_NUMPAD_SLASH));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_INSERT, MKEY_INSERT));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_DELETE, MKEY_DELETE));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_HOME, MKEY_HOME));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_END, MKEY_END));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_PAGE_UP, MKEY_PAGE_UP));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_PAGE_DOWN, MKEY_PAGE_DOWN));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_PRINTSCREEN, MKEY_PRINTSCREEN));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_SCROLL_LOCK, MKEY_SCROLL_LOCK));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_PAUSE, MKEY_PAUSE_BREAK));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_NUM_LOCK, MKEY_NUM_LOCK));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_COMMA, MKEY_COMMA));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_NUMPAD_PERIOD, MKEY_NUMPAD_COMMA));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_PERIOD, MKEY_PERIOD));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_SLASH, MKEY_SLASH));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_APOSTROPHE, MKEY_APSTROPHE));
	m_ScanCodeToMKeyConversionTable->insert(std::make_pair(SCANCODE_SEMICOLON, MKEY_SEMICOLON));

	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_TAB, MKEY_TAB));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_GRAVE, MKEY_GRAVE));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_CAPSLOCK, MKEY_CAPS_LOCK));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_NONUSBACKSLASH, MKEY_ANGLED_BRACKET));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_KP_ENTER, MKEY_NUMPAD_ENTER));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_RETURN, MKEY_MAIN_ENTER));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_EQUALS, MKEY_EQUALS));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_MINUS, MKEY_MINUS));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_KP_PLUS, MKEY_NUMPAD_PLUS));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_KP_MINUS, MKEY_NUMPAD_MINUS));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_KP_MULTIPLY, MKEY_NUMPAD_ASTERISK));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_KP_DIVIDE, MKEY_NUMPAD_SLASH));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_INSERT, MKEY_INSERT));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_DELETE, MKEY_DELETE));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_HOME, MKEY_HOME));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_END, MKEY_END));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_PAGEUP, MKEY_PAGE_UP));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_PAGEDOWN, MKEY_PAGE_DOWN));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_PRINTSCREEN, MKEY_PRINTSCREEN));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_SCROLLLOCK, MKEY_SCROLL_LOCK));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_PAUSE, MKEY_PAUSE_BREAK));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_NUMLOCKCLEAR, MKEY_NUM_LOCK));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_COMMA, MKEY_COMMA));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_KP_COMMA, MKEY_NUMPAD_COMMA));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_PERIOD, MKEY_PERIOD));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_SLASH, MKEY_SLASH));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_APOSTROPHE, MKEY_APSTROPHE));
	m_SDLScanCodeToMKeyConversionTable->insert(std::make_pair(SDL_SCANCODE_SEMICOLON, MKEY_SEMICOLON));
}