#include "matrix.h"
#include <string.h>
#include "spi_master.h"
#include "tusb.h"

extern uint32_t millis(void);

uint8_t prev_keys[MATRIX_REG_COUNT];

void print_keys(uint8_t keyarray[], uint8_t num) 
{
	if (!tud_cdc_connected() && tud_cdc_available())
		return;
	tud_cdc_write_str("0b");
	for (uint8_t a = 0; a < num; a++) {
		for (uint8_t i = 0; i < 8; i++) {
			tud_cdc_write_char((keyarray[a] & (1 << i)) ? '1' : '0');
		}
		tud_cdc_write_char(' ');
	}
	tud_cdc_write_char('\n');
}

uint8_t shift_task(void) {
	static uint32_t time;
	uint8_t keys[2];

	/*
	if ((millis() - time) < 10)
		return 0;
	time = millis();
	*/

	spi_ss(1);
	for (int i = 0; i < MATRIX_REG_COUNT; i++) {
		keys[i] = spi_write_byte(0);
		if (MATRIX_REG_INVERT & (1<<i))
			keys[i] = ~keys[i];
	}
	spi_ss(0);

	if (memcmp(prev_keys, keys, sizeof(prev_keys))) {
		memcpy(prev_keys, keys, sizeof(prev_keys));
		//print_keys(keys, 2);
		return 1;
	}
	return 0;
}
