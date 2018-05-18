#include "MEngineBackgroundInput.h"
#include "MEngineGraphicsInternal.h" // For getting the SDL_Window
#include "Scancodes.h"
#include <MUtilityLog.h>
#include <MUtilityWindowsInclude.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include <unordered_map>

#define LOG_CATEGORY_BACKGROUND_INPUT "MEngineBackgroundInput"

namespace BackgroundInput
{
#if PLATFORM == PLATFORM_WINDOWS
	LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	WNDPROC m_OriginalWndProc;
#endif
	void PopulateConversionTables();

	std::unordered_map<uint32_t, SDL_Scancode>* m_ScanCodeToSDLScanCodeConversionTable;
}

bool BackgroundInput::Initialize()
{
	bool result = false;

	m_ScanCodeToSDLScanCodeConversionTable = new std::unordered_map<uint32_t, SDL_Scancode>();
	PopulateConversionTables();

#if PLATFORM == PLATFORM_WINDOWS
	// Get window handle from SDL
	SDL_SysWMinfo systemInfo;
	SDL_VERSION(&systemInfo.version);
	if(!SDL_GetWindowWMInfo(MEngineGraphics::GetWindow(), &systemInfo))
		MLOG_ERROR("Failed to get window handle", LOG_CATEGORY_BACKGROUND_INPUT);

	// Register a keyboard device
	RAWINPUTDEVICE Rid;
	Rid.usUsagePage = 0x01;
	Rid.usUsage = 0x06;
	Rid.dwFlags = RIDEV_INPUTSINK; // Take input regardless of window focus
	Rid.hwndTarget = systemInfo.info.win.window;
	if (RegisterRawInputDevices(&Rid, 1, sizeof(Rid)) != FALSE)
	{
		// Override the original(SDL) event loop with our own and store a handle to the original one so events can be forwarded and handled as originally intended
		m_OriginalWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(systemInfo.info.win.window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&WndProc)));
		result = true;
	}
	else
		MLOG_ERROR("Failed to register keyboard as input device", LOG_CATEGORY_BACKGROUND_INPUT);
#else
	assert(false && "Background input is not supported on this platform");
#endif
	return result;
}

void BackgroundInput::Shutdown()
{
	delete m_ScanCodeToSDLScanCodeConversionTable;

#if PLATFORM == PLATFORM_WINDOWS
	// Get window handle from SDL
	SDL_SysWMinfo systemInfo;
	SDL_VERSION(&systemInfo.version);
	if (!SDL_GetWindowWMInfo(MEngineGraphics::GetWindow(), &systemInfo))
		MLOG_ERROR("Failed to get window handle", LOG_CATEGORY_BACKGROUND_INPUT);

	// Unregister the raw input keyboard device
	RAWINPUTDEVICE Rid;
	Rid.usUsagePage = 0x01;
	Rid.usUsage = 0x06;
	Rid.dwFlags = RIDEV_REMOVE;
	Rid.hwndTarget = systemInfo.info.win.window;

	if (RegisterRawInputDevices(&Rid, 1, sizeof(Rid)) == FALSE)
		MLOG_ERROR("Failed to unregister keyboard as input device", LOG_CATEGORY_BACKGROUND_INPUT);
#else
	assert(false && "Background input is not supported on this platform");
#endif
}

#if PLATFORM == PLATFORM_WINDOWS
LRESULT BackgroundInput::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INPUT:
		{
			if (wParam == RIM_INPUTSINK) // Only handle background input
			{
				UINT size;
				GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER));
				LPBYTE bytePtr = new BYTE[size];
				if (bytePtr != nullptr)
				{
					if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, bytePtr, &size, sizeof(RAWINPUTHEADER)) == size)
					{
						RAWINPUT* rawInput = reinterpret_cast<RAWINPUT*>(bytePtr);
						const RAWKEYBOARD& rawKeboard = rawInput->data.keyboard;
						if (rawInput->header.dwType == RIM_TYPEKEYBOARD)
						{
							uint32_t scanCode = rawKeboard.MakeCode;
							if((rawKeboard.Flags & RI_KEY_E0) != 0)
								scanCode |= 0xE000; // Prefix the scancode with the E0 extension
							else if((rawKeboard.Flags & RI_KEY_E1) != 0)
								MLOG_WARNING("Reiceived input using E1 extension; these scancodes are not yet supported", LOG_CATEGORY_BACKGROUND_INPUT);

							auto iterator = m_ScanCodeToSDLScanCodeConversionTable->find(scanCode);
							if (iterator != m_ScanCodeToSDLScanCodeConversionTable->end())
							{
								// Create an SDL event corresponding to the WM_INPUT event and send it to SDL
								SDL_Event sdlevent = {};
								sdlevent.type = (((rawKeboard.Flags & RI_KEY_BREAK) != 0) ? SDL_KEYUP : SDL_KEYDOWN);
								sdlevent.key.state = (((rawKeboard.Flags & RI_KEY_BREAK) != 0) ? SDL_RELEASED : SDL_PRESSED);
								sdlevent.key.keysym.scancode = SDL_Scancode(iterator->second);
								sdlevent.key.keysym.sym = SDL_SCANCODE_TO_KEYCODE(sdlevent.key.keysym.scancode);
								sdlevent.key.timestamp = SDL_GetTicks();
								sdlevent.key.repeat = 0;

								SDL_PushEvent(&sdlevent);
							}
							else
								MLOG_WARNING("A key was pressed that could not be converted into an MKEY; Scancode = " << scanCode, LOG_CATEGORY_BACKGROUND_INPUT);
						}
					}
					else
						MLOG_ERROR("GetRawInputData did not return the expected size", LOG_CATEGORY_BACKGROUND_INPUT);
				}
				delete[] bytePtr;
			}
		} break;

		default:
			break;
	}

	return CallWindowProc(m_OriginalWndProc, hwnd, msg, wParam, lParam); // Forward the event to the original(SDL) event handling loop
}
#endif

void BackgroundInput::PopulateConversionTables() // TODODB: Implement a more efficient way to map scancodes to SDL_Scancodes
{
	// Letters
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_A, SDL_SCANCODE_A));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_B, SDL_SCANCODE_B));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_C, SDL_SCANCODE_C));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_D, SDL_SCANCODE_D));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_E, SDL_SCANCODE_E));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_F, SDL_SCANCODE_F));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_G, SDL_SCANCODE_G));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_H, SDL_SCANCODE_H));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_I, SDL_SCANCODE_I));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_J, SDL_SCANCODE_J));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_K, SDL_SCANCODE_K));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_L, SDL_SCANCODE_L));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_M, SDL_SCANCODE_M));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_N, SDL_SCANCODE_N));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_O, SDL_SCANCODE_O));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_P, SDL_SCANCODE_P));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_Q, SDL_SCANCODE_Q));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_R, SDL_SCANCODE_R));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_S, SDL_SCANCODE_S));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_T, SDL_SCANCODE_T));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_U, SDL_SCANCODE_U));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_V, SDL_SCANCODE_V));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_W, SDL_SCANCODE_W));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_X, SDL_SCANCODE_X));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_Y, SDL_SCANCODE_Y));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_Z, SDL_SCANCODE_Z));

	// Numeric
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_1, SDL_SCANCODE_1));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_2, SDL_SCANCODE_2));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_3, SDL_SCANCODE_3));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_4, SDL_SCANCODE_4));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_5, SDL_SCANCODE_5));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_6, SDL_SCANCODE_6));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_7, SDL_SCANCODE_7));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_8, SDL_SCANCODE_8));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_9, SDL_SCANCODE_9));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_0, SDL_SCANCODE_0));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_NUMPAD_0, SDL_SCANCODE_KP_1));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_NUMPAD_1, SDL_SCANCODE_KP_2));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_NUMPAD_2, SDL_SCANCODE_KP_3));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_NUMPAD_3, SDL_SCANCODE_KP_4));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_NUMPAD_4, SDL_SCANCODE_KP_5));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_NUMPAD_5, SDL_SCANCODE_KP_6));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_NUMPAD_6, SDL_SCANCODE_KP_7));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_NUMPAD_7, SDL_SCANCODE_KP_8));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_NUMPAD_8, SDL_SCANCODE_KP_9));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_NUMPAD_9, SDL_SCANCODE_KP_0));

	// Function keys
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_F1, SDL_SCANCODE_F1));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_F2, SDL_SCANCODE_F2));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_F3, SDL_SCANCODE_F3));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_F4, SDL_SCANCODE_F4));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_F5, SDL_SCANCODE_F5));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_F6, SDL_SCANCODE_F6));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_F7, SDL_SCANCODE_F7));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_F8, SDL_SCANCODE_F8));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_F9, SDL_SCANCODE_F9));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_F10, SDL_SCANCODE_F10));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_F11, SDL_SCANCODE_F11));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_F12, SDL_SCANCODE_F12));

	// Modifiers
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_LEFT_SHIFT, SDL_SCANCODE_LSHIFT));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_RIGHT_SHIFT, SDL_SCANCODE_RSHIFT));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_LEFT_ALT, SDL_SCANCODE_LALT));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_RIGHT_ALT, SDL_SCANCODE_RALT));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_LEFT_CONTROL, SDL_SCANCODE_LCTRL));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_RIGHT_CONTROL, SDL_SCANCODE_RCTRL));

	// Special
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_ARROW_UP, SDL_SCANCODE_UP));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_ARROW_DOWN, SDL_SCANCODE_DOWN));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_ARROW_LEFT, SDL_SCANCODE_LEFT));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_ARROW_RIGHT, SDL_SCANCODE_RIGHT));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_SPACE, SDL_SCANCODE_SPACE));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_APPLICATION, SDL_SCANCODE_APPLICATION));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_BACKSPACE, SDL_SCANCODE_BACKSPACE));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_TAB, SDL_SCANCODE_TAB));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_GRAVE, SDL_SCANCODE_GRAVE));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_CAPSLOCK, SDL_SCANCODE_CAPSLOCK));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_LEFT_BRACKET, SDL_SCANCODE_LEFTBRACKET));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_RIGHT_BRACKET, SDL_SCANCODE_RIGHTBRACKET));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_BACKSLASH, SDL_SCANCODE_BACKSLASH));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_ANGLED_BRACKET, SDL_SCANCODE_NONUSBACKSLASH));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_LEFT_META, SDL_SCANCODE_LGUI));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_RIGHT_META, SDL_SCANCODE_RGUI));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(57435, static_cast<SDL_Scancode>(57435))); // There exists no matching SDL_SCANCODE for these scancodes (triggered when closing the windows start menu using the windows key)
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(57436, static_cast<SDL_Scancode>(57436)));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_MEDIA_NEXT, SDL_SCANCODE_AUDIONEXT));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_MEDIA_PREVIOUS, SDL_SCANCODE_AUDIOPREV));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_MEDIA_STOP, SDL_SCANCODE_AUDIOSTOP));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_MEDIA_PLAY, SDL_SCANCODE_AUDIOPLAY));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_VOLUME_MUTE, SDL_SCANCODE_AUDIOMUTE));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_NUMPAD_ENTER, SDL_SCANCODE_KP_ENTER));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_ENTER, SDL_SCANCODE_RETURN));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_EQUALS, SDL_SCANCODE_EQUALS));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_MINUS, SDL_SCANCODE_MINUS));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_NUMPAD_PLUS, SDL_SCANCODE_KP_PLUS));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_NUMPAD_MINUS, SDL_SCANCODE_KP_MINUS));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_NUMPAD_ASTERISK, SDL_SCANCODE_KP_MULTIPLY));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_NUMPAD_SLASH, SDL_SCANCODE_KP_DIVIDE));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_INSERT, SDL_SCANCODE_INSERT));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_DELETE, SDL_SCANCODE_DELETE));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_HOME, SDL_SCANCODE_HOME));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_END, SDL_SCANCODE_END));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_PAGE_UP, SDL_SCANCODE_PAGEUP));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_PAGE_DOWN, SDL_SCANCODE_PAGEDOWN));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_PRINTSCREEN, SDL_SCANCODE_PRINTSCREEN));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_SCROLL_LOCK, SDL_SCANCODE_SCROLLLOCK));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_PAUSE, SDL_SCANCODE_PAUSE));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_NUM_LOCK, SDL_SCANCODE_SCROLLLOCK));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_COMMA, SDL_SCANCODE_COMMA));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_NUMPAD_COMMA, SDL_SCANCODE_KP_PERIOD));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_PERIOD, SDL_SCANCODE_PERIOD));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_SLASH, SDL_SCANCODE_SLASH));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_APOSTROPHE, SDL_SCANCODE_APOSTROPHE));
	m_ScanCodeToSDLScanCodeConversionTable->insert(std::make_pair(SCANCODE_SEMICOLON, SDL_SCANCODE_SEMICOLON));
}