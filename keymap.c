#include <stdint.h>
#include "matrix.h"
#include "config.h"
#include "tusb.h"

#define FN_KEY	prev_keys[1] & (1<<5)

extern uint8_t prev_keys[MATRIX_REG_COUNT];

static uint8_t prev_keys_keys[MATRIX_REG_COUNT] = { 0 };
static uint8_t prev_keys_modifiers[MATRIX_REG_COUNT] = { 0 };
static uint8_t prev_keys_mousekeys[MATRIX_REG_COUNT] = { 0 };
static uint8_t prev_keys_consumer[MATRIX_REG_COUNT] = { 0 };

uint8_t keymap_keys[2][MATRIX_REG_COUNT][8] = 
{
	{	//Layer 0
		{	//Register 0
			0,
			HID_KEY_PAGE_DOWN,
			HID_KEY_PAGE_UP,
			HID_KEY_ARROW_RIGHT,
			HID_KEY_ARROW_UP,
			HID_KEY_DELETE,
			HID_KEY_ARROW_DOWN,
			HID_KEY_ARROW_LEFT },
		{	//Register 1
			0,		// HID_KEY_SHIFT_RIGHT,
			HID_KEY_RETURN,
			HID_KEY_BACKSLASH,
			HID_KEY_BACKSPACE,
			0,
			0,		// HID_KEY_ALT_RIGHT,
			0,		// FUNCTION
			0 },	// HID_KEY_CONTROL_RIGHT 
		{	// Register 2
			HID_KEY_SLASH,
			HID_KEY_APOSTROPHE,
			HID_KEY_SEMICOLON,
			HID_KEY_BRACKET_RIGHT,
			HID_KEY_BRACKET_LEFT,
			HID_KEY_P,
			HID_KEY_EQUAL,
			HID_KEY_MINUS },
		{	// Register 3
			HID_KEY_PERIOD,
			HID_KEY_COMMA,
			HID_KEY_L,
			HID_KEY_K,
			HID_KEY_O,
			HID_KEY_I,
			HID_KEY_0,
			HID_KEY_9 },
		{	// Register 4
			HID_KEY_M,
			HID_KEY_N,
			HID_KEY_J,
			HID_KEY_H,
			HID_KEY_U,
			HID_KEY_Y,
			HID_KEY_8,
			HID_KEY_7 },
		{	// Register 5
			0,
			0,
			0,
			HID_KEY_SPACE,
			HID_KEY_B,
			HID_KEY_G,
			HID_KEY_T,
			HID_KEY_6 },
		{	// Register 6
			HID_KEY_V,
			HID_KEY_C,
			HID_KEY_F,
			HID_KEY_D,
			HID_KEY_R,
			HID_KEY_E,
			HID_KEY_5,
			HID_KEY_4 },
		{	// Register 7
			HID_KEY_X,
			HID_KEY_Z,
			HID_KEY_S,
			HID_KEY_A,
			HID_KEY_W,
			HID_KEY_Q,
			HID_KEY_3,
			HID_KEY_2 },
		{	// Register 8
			0,	//HID_KEY_ALT_LEFT,
			0,	//HID_KEY_GUI_LEFT,
			0,	//HID_KEY_CONTROL_LEFT,
			0,	//HID_KEY_SHIFT_LEFT,
			HID_KEY_ESCAPE,
			HID_KEY_TAB,
			HID_KEY_1,
			HID_KEY_GRAVE }
	},
	{	//Layer 1
		{	// Register 0
			0,
			HID_KEY_END,
			HID_KEY_HOME,
			0,
			0,
			HID_KEY_INSERT,
			0,
			0 },
		{	// Register 1
			0,
			HID_KEY_RETURN,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 2
			0,
			0,
			0,
			0,
			0,
			0,
			HID_KEY_F12,
			HID_KEY_F11 },
		{	// Register 3
			HID_KEY_3,
			HID_KEY_2,
			HID_KEY_6,
			HID_KEY_5,
			HID_KEY_9,
			HID_KEY_8,
			HID_KEY_F10,
			HID_KEY_F9 },
		{	// Register 4
			HID_KEY_1,
			0,
			HID_KEY_4,
			0,
			HID_KEY_7,
			0,
			HID_KEY_F8,
			HID_KEY_F7 },
		{	// Register 5
			0,
			0,
			0,
			HID_KEY_0,
			0,
			0,
			0,
			HID_KEY_F6 },
		{	// Register 6
			0,
			0,
			0,
			0,
			0,
			0,
			HID_KEY_F5,
			HID_KEY_F4 },
		{	// Register 7
			0,
			0,
			0,
			0,
			0,
			0,
			HID_KEY_F3,
			HID_KEY_F2 },
		{	// Register 8
			0,
			0,
			0,
			0,
			HID_KEY_CAPS_LOCK,
			0,
			HID_KEY_F1,
			0 }
	}
};

uint8_t read_keys(uint8_t keycodes[])
{
	uint8_t layer = FN_KEY ? 1 : 0;
	uint8_t keycount = 0;
	uint8_t change = 0;
	for (uint8_t reg = 0; reg < MATRIX_REG_COUNT; reg++) { // Check each register
		for (uint8_t j = 0; j < 8; j++) { // Check each bit
			if ((prev_keys[reg] & (1 << j)) != (prev_keys_keys[reg] & (1 << j))) { // This bit changed, and
				if (keymap_keys[layer][reg][j]) { // this bit is populated in keymap
					change = 1; // report a change
				}
			}
			if ((prev_keys[reg] & (1 << j)) && (keymap_keys[layer][reg][j])) { // If it was pressed and populated,
				keycodes[keycount++] = keymap_keys[layer][reg][j]; // load it
				if (keycount > 5)
					return change;
			}
		}

		prev_keys_keys[reg] = prev_keys[reg];
	}
	return change;
}

uint8_t keymap_modifiers[2][MATRIX_REG_COUNT][8] = 
{
	{	//Layer 0
		{	//Register 0
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	//Register 1
			KEYBOARD_MODIFIER_RIGHTSHIFT,
			0,
			0,
			0,
			0,
			0,
			KEYBOARD_MODIFIER_RIGHTGUI,
			KEYBOARD_MODIFIER_RIGHTCTRL },
		{	// Register 2
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 3
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 4
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 5
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 6
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 7
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 8
			KEYBOARD_MODIFIER_LEFTALT,
			KEYBOARD_MODIFIER_LEFTGUI,
			KEYBOARD_MODIFIER_LEFTCTRL,
			KEYBOARD_MODIFIER_LEFTSHIFT,
			0,
			0,
			0,
			0 }
	},
	{	//Layer 1
		{	// Register 0
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 1
			KEYBOARD_MODIFIER_RIGHTSHIFT,
			0,
			0,
			0,
			0,
			0,
			KEYBOARD_MODIFIER_RIGHTGUI,
			KEYBOARD_MODIFIER_RIGHTCTRL },
		{	// Register 2
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 3
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 4
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 5
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 6
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 7
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 8
			KEYBOARD_MODIFIER_LEFTALT,
			KEYBOARD_MODIFIER_LEFTGUI,
			KEYBOARD_MODIFIER_LEFTCTRL,
			KEYBOARD_MODIFIER_LEFTSHIFT,
			0,
			0,
			0,
			0 }
	}
};

uint8_t read_modifiers(uint8_t *modifiers)
{
	uint8_t layer = FN_KEY ? 1 : 0;;
	uint8_t change = 0;
	for (uint8_t reg = 0; reg < MATRIX_REG_COUNT; reg++) { // Check each register
		for (uint8_t j = 0; j < 8; j++) { // Check each bit
			if ((prev_keys[reg] & (1 << j)) != (prev_keys_modifiers[reg] & (1 << j))) { // This bit changed, and
				if (keymap_modifiers[layer][reg][j]) { // this bit is populated in keymap
					change = 1;
				}
			}
			if (prev_keys[reg] & (1 << j) && (keymap_modifiers[layer][reg][j])) { // If it was pressed and populated
				*modifiers |= keymap_modifiers[layer][reg][j]; // load it
			}
		}

		prev_keys_modifiers[reg] = prev_keys[reg];
	}
	return change;
}

uint8_t keymap_mousekeys[2][MATRIX_REG_COUNT][8] = 
{
	{	//Layer 0
		{	//Register 0
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	//Register 1
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 2
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 3
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 4
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 5
			MOUSE_BUTTON_RIGHT,
			MOUSE_BUTTON_MIDDLE,
			MOUSE_BUTTON_LEFT,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 6
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 7
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 8
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 }
	},
	{	//Layer 1
		{	// Register 0
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 1
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 2
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 3
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 4
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 5
			MOUSE_BUTTON_FORWARD,
			MOUSE_BUTTON_MIDDLE,
			MOUSE_BUTTON_BACKWARD,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 6
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 7
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 8
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 }
	}
};

uint8_t read_mousekeys(uint8_t *mousekeys, uint8_t *fn_key)
{
	*fn_key = FN_KEY ? 1 : 0;
	uint8_t change = 0;
	for (uint8_t reg = 0; reg < MATRIX_REG_COUNT; reg++) { // Check each register
		for (uint8_t j = 0; j < 8; j++) { // Check each bit
			if ((prev_keys[reg] & (1 << j)) != (prev_keys_mousekeys[reg] & (1 << j))) { // This bit changed, and
				if (keymap_mousekeys[*fn_key][reg][j]) { // this bit is populated in keymap
					change = 1;
				}
			}
			if (prev_keys[reg] & (1 << j) && (keymap_mousekeys[*fn_key][reg][j])) { // If it was pressed and populated,
				*mousekeys |= keymap_mousekeys[*fn_key][reg][j]; // load it
			}
		}
		prev_keys_mousekeys[reg] = prev_keys[reg];
	}
	return change;
}

// TODO: should technically be uint16_t
uint8_t keymap_consumer[2][MATRIX_REG_COUNT][8] =
{
	{	//Layer 0
		{	//Register 0
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	//Register 1
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 2
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 3
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 4
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 5
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 6
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 7
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 8
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 }
	},
	{	//Layer 1
		{	// Register 0
			0,
			0,
			0,
			HID_USAGE_CONSUMER_VOLUME_INCREMENT,
			HID_USAGE_CONSUMER_SCAN_NEXT,
			0,
			HID_USAGE_CONSUMER_PLAY_PAUSE,
			HID_USAGE_CONSUMER_VOLUME_DECREMENT},
		{	// Register 1
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 2
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 3
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 4
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 5
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 6
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 7
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{	// Register 8
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 }
	}
};

uint8_t read_consumer(uint16_t *button)
{
	uint8_t fn_key = FN_KEY ? 1 : 0;
	uint8_t change = 0;
	for (uint8_t reg = 0; reg < MATRIX_REG_COUNT; reg++) { // Check each register
		for (uint8_t j = 0; j < 8; j++) { // Check each bit
			if ((prev_keys[reg] & (1 << j)) != (prev_keys_consumer[reg] & (1 << j))) { // This bit changed, and
				if (keymap_consumer[fn_key][reg][j]) { // this bit is populated in keymap
					change = 1;
				}
			}
			if (prev_keys[reg] & (1 << j) && (keymap_consumer[fn_key][reg][j])) { // If it was pressed and populated,
				*button = keymap_consumer[fn_key][reg][j]; // load it
			}
		}
		prev_keys_consumer[reg] = prev_keys[reg];
	}
	return change;
}
