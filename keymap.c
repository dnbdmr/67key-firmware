#include <stdint.h>
#include "matrix.h"
#include "tusb.h"

#define FN_KEY	prev_keys[0] & (1<<7)

extern uint8_t prev_keys[MATRIX_REG_COUNT];

uint8_t keymap_keys[2][MATRIX_REG_COUNT][8] = 
{
	{	//Layer 0
		{
			HID_KEY_D,
			HID_KEY_C,
			HID_KEY_B,
			HID_KEY_A,
			0,
			0,
			0,
			0 },
		{
			HID_KEY_Z,
			HID_KEY_Y,
			HID_KEY_X,
			HID_KEY_W,
			0,
			0,
			0,
			0 }
	},
	{	//Layer 1
		{
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0 },
		{
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

void read_keys(uint8_t keycodes[])
{
	uint8_t layer = FN_KEY;
	uint8_t keycount = 0;
	for (uint8_t reg = 0; reg < MATRIX_REG_COUNT; reg++) {
		for (uint8_t j = 0; j < 8; j ++) {
			if ((prev_keys[reg] & (1 << j)) && prev_keys[reg]) {
				keycodes[keycount++] = keymap_keys[layer][reg][j];
				if (keycount > 5)
					return;
			}
		}
	}
	return;
}

