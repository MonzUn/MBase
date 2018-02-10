#pragma once

// TODODB: Find a good way to handle MKEYs such as MKEY_CONTROL and MKEY_1 and the like (Scancode Left control should trigger both "left_control" and "control")
enum MENGINE_KEY // Names are based on US keyboard layout (101)
{
	// Letters
	MKEY_A,
	MKEY_B,
	MKEY_C,
	MKEY_D,
	MKEY_E,
	MKEY_F,
	MKEY_G,
	MKEY_H,
	MKEY_I,
	MKEY_J,
	MKEY_K,
	MKEY_L,
	MKEY_M,
	MKEY_N,
	MKEY_O,
	MKEY_P,
	MKEY_Q,
	MKEY_R,
	MKEY_S,
	MKEY_T,
	MKEY_U,
	MKEY_V,
	MKEY_W,
	MKEY_X,
	MKEY_Y,
	MKEY_Z,

	// Numeric
	//MKEY_1,
	//MKEY_2,
	//MKEY_3,
	//MKEY_4,
	//MKEY_5,
	//MKEY_6,
	//MKEY_7,
	//MKEY_8,
	//MKEY_9,
	//MKEY_0,
	MKEY_NUMROW_1,
	MKEY_NUMROW_2,
	MKEY_NUMROW_3,
	MKEY_NUMROW_4,
	MKEY_NUMROW_5,
	MKEY_NUMROW_6,
	MKEY_NUMROW_7,
	MKEY_NUMROW_8,
	MKEY_NUMROW_9,
	MKEY_NUMROW_0,
	MKEY_NUMPAD_1,
	MKEY_NUMPAD_2,
	MKEY_NUMPAD_3,
	MKEY_NUMPAD_4,
	MKEY_NUMPAD_5,
	MKEY_NUMPAD_6,
	MKEY_NUMPAD_7,
	MKEY_NUMPAD_8,
	MKEY_NUMPAD_9,
	MKEY_NUMPAD_0,

	// Function keys
	MKEY_F1,
	MKEY_F2,
	MKEY_F3,
	MKEY_F4,
	MKEY_F5,
	MKEY_F6,
	MKEY_F7,
	MKEY_F8,
	MKEY_F9,
	MKEY_F10,
	MKEY_F11,
	MKEY_F12,

	// Modifiers
	MKEY_LEFT_SHIFT,
	MKEY_RIGHT_SHIFT,
	//MKEY_SHIFT,
	MKEY_LEFT_ALT,
	MKEY_RIGHT_ALT,
	//MKEY_ALT,
	MKEY_LEFT_CONTROL,
	MKEY_RIGHT_CONTROL,
	//MKEY_CONTROL,

	// Special
	MKEY_TAB,
	MKEY_GRAVE,
	MKEY_CAPS_LOCK,
	MKEY_ANGLED_BRACKET, // Between left shift and Z
	MKEY_LEFT_COMMAND, // TODODB: Find out the scancode for this key (left windows key)
	MKEY_RIGHT_COMMAND, // TODODB: Find out the scancode for this key (right windows key)
	//MKEY_COMMAND, // Windows key
	//MKEY_ENTER,
	MKEY_NUMPAD_ENTER,
	MKEY_MAIN_ENTER, // Enter button above right shift
	MKEY_EQUALS, // Left of backspace
	MKEY_MINUS, // Right of 0
	MKEY_NUMPAD_PLUS,
	MKEY_NUMPAD_MINUS,
	MKEY_NUMPAD_ASTERISK,
	MKEY_NUMPAD_SLASH,
	MKEY_INSERT,
	MKEY_DELETE,
	MKEY_HOME,
	MKEY_END,
	MKEY_PAGE_UP,
	MKEY_PAGE_DOWN,
	MKEY_PRINTSCREEN,
	MKEY_SCROLL_LOCK,
	MKEY_PAUSE_BREAK,
	MKEY_NUM_LOCK,
	MKEY_COMMA, // Right of M
	MKEY_NUMPAD_COMMA,
	MKEY_PERIOD, // Right of comma
	MKEY_SLASH, // Left of right shift
	MKEY_APSTROPHE, // Left of enter
	MKEY_SEMICOLON, // Left of apostrophe

	MKEY_COUNT // Not a key but the amount of entries in the enum
};