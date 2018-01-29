#include "mengineInputInternal.h"
#include "scancodes.h"
#include <MUtilityLog.h>
#include <MUtilityPlatformDefinitions.h>
#include <MUtilityWindowsInclude.h>
#include <unordered_map>

using namespace MEngineInput;

#define MUTILITY_LOG_CATEGORY_INPUT "MEngineInput"

bool windowFocusRequired = true;
bool pressedKeys[MENGINE_KEY::MKEY_COUNT] = { false };
bool previouslyPressedKeys[MENGINE_KEY::MKEY_COUNT] = { false };
bool PressedKeysBuffer[MENGINE_KEY::MKEY_COUNT] = { false }; // Used when focus is not required

std::unordered_map<uint32_t, MENGINE_KEY> ScanCodeToMKeyConversionTable;

#if PLATFORM == PLATFORM_WINDOWS
HHOOK hook = nullptr;
LRESULT HookCallback(int keyCode, WPARAM wParam, LPARAM lParam);
#endif

void PopulateConversionTables();

// ---------- INTERFACE ----------

void MEngineInput::SetFocusRequired(bool required)
{
#if PLATFORM != PLATFORM_WINDOWS
	static_assert(false, "SetFocusRequired is only supported on the windows platform");
#else
	if (windowFocusRequired == required)
		return;

	if (!required)
	{
		if (hook = SetWindowsHookEx(WH_KEYBOARD_LL, HookCallback, NULL, 0))
			windowFocusRequired = false;
		else
			MLOG_ERROR("Failed to initialize non focus key input mode", MUTILITY_LOG_CATEGORY_INPUT);
	}
	else
	{
		UnhookWindowsHookEx(hook);
		memset(&PressedKeysBuffer, false, sizeof(PressedKeysBuffer));
		windowFocusRequired = true;
	}
#endif
}

bool MEngineInput::KeyDown(MENGINE_KEY key)
{
	return pressedKeys[key];
}

bool MEngineInput::KeyUp(MENGINE_KEY key)
{
	return !pressedKeys[key];
}

bool MEngineInput::KeyPressed(MENGINE_KEY key)
{
	return (!previouslyPressedKeys[key] && pressedKeys[key]);
}

bool MEngineInput::KeyReleased(MENGINE_KEY key)
{
	return (previouslyPressedKeys[key] && !pressedKeys[key]);
}

// ---------- INTERNAL ----------

void MEngineInput::Initialize()
{
	PopulateConversionTables();
}

void MEngineInput::Update()
{
	memcpy(&previouslyPressedKeys, &pressedKeys, sizeof(pressedKeys));
	if (!windowFocusRequired)
		memcpy(&pressedKeys, &PressedKeysBuffer, sizeof(pressedKeys));
}

#if PLATFORM == PLATFORM_WINDOWS
LRESULT HookCallback(int keyCode, WPARAM wParam, LPARAM lParam)
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

	// Modifiers
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_LEFT_SHIFT, MKEY_LEFT_SHIFT));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_RIGHT_SHIFT, MKEY_RIGHT_SHIFT));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_LEFT_ALT, MKEY_LEFT_ALT));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_RIGHT_ALT, MKEY_RIGHT_ALT));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_LEFT_CONTROL, MKEY_LEFT_CONTROL));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_RIGHT_CONTROL, MKEY_RIGHT_CONTROL));

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
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_NUMPAD_PERIOD, MKEY_NUMPAD_PERIOD));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_PERIOD, MKEY_PERIOD));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_SLASH, MKEY_SLASH));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_APOSTROPHE, MKEY_APSTROPHE));
	ScanCodeToMKeyConversionTable.insert(std::make_pair(SCANCODE_SEMICOLON, MKEY_SEMICOLON));
}