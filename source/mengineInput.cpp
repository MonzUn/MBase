#include "mengineInputInternal.h"
#include "interface/mengineLog.h"
#include <MUtilityPlatformDefinitions.h>
#include <MUtilityWindowsInclude.h>
#include <unordered_map>

using namespace MEngineInput;

#define MENGINE_LOG_CATEGORY_INPUT "MEngineInput"

bool windowFocusRequired = true;
bool pressedKeys[MEngineKey::MKEY_COUNT] = { false };
bool previouslyPressedKeys[MEngineKey::MKEY_COUNT] = { false };
bool PressedKeysBuffer[MEngineKey::MKEY_COUNT] = { false }; // Used when focus is not required

#if PLATFORM == PLATFORM_WINDOWS
HHOOK hook = nullptr;
std::unordered_map<DWORD, MEngineKey> windowsKeyConversionTable;

LRESULT HookCallback(int keyCode, WPARAM wParam, LPARAM lParam);
#endif

void PopulateConversionTables();

/* INTERFACE */

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
			MLOG_ERROR("Failed to initialize non focus key input mode", MENGINE_LOG_CATEGORY_INPUT);
	}
	else
	{
		UnhookWindowsHookEx(hook);
		memset(&PressedKeysBuffer, false, sizeof(PressedKeysBuffer));
		windowFocusRequired = true;
	}
#endif
}

bool MEngineInput::KeyDown(MEngineKey key)
{
	return pressedKeys[key];
}

bool MEngineInput::KeyUp(MEngineKey key)
{
	return !pressedKeys[key];
}

bool MEngineInput::KeyPressed(MEngineKey key)
{
	return (!previouslyPressedKeys[key] && pressedKeys[key]);
}

bool MEngineInput::KeyReleased(MEngineKey key)
{
	return (previouslyPressedKeys[key] && !pressedKeys[key]);
}

/* INTERNAL */

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
			auto iterator = windowsKeyConversionTable.find(keyStruct.vkCode);
			if (iterator != windowsKeyConversionTable.end())
				PressedKeysBuffer[iterator->second] = (wParam == WM_KEYDOWN); // Set key pressed to true if wparam is WM_KEYDOWN
		}
	}

	// Call the next hook in the hook chain.
	return CallNextHookEx(hook, keyCode, wParam, lParam);
}
#endif

/* LOCAL */

void PopulateConversionTables()
{
#if PLATFORM == PLATFORM_WINDOWS
	windowsKeyConversionTable.insert(std::make_pair(VK_TAB, MKey_TAB));
	windowsKeyConversionTable.insert(std::make_pair(VK_LCONTROL, MKey_CONTROL));
	windowsKeyConversionTable.insert(std::make_pair(VK_RCONTROL, MKey_CONTROL));
	windowsKeyConversionTable.insert(std::make_pair(VK_LMENU, MKey_ALT));
	windowsKeyConversionTable.insert(std::make_pair(VK_RMENU, MKey_ALT));
	windowsKeyConversionTable.insert(std::make_pair(VK_OEM_5, MKey_GRAVE));

	// Letters
	windowsKeyConversionTable.insert(std::make_pair(0x54, MKey_T));
#endif
}