#include "matrix.h"
#include <string.h>
#include "spi_master.h"
#include "tusb.h"

extern uint32_t millis(void);

static uint8_t prev_keys[MATRIX_REG_NUM];

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

void shift_task(void) {
	static uint32_t time;
	uint8_t keys[2];

	if ((millis() - time) < 10)
		return;
	time = millis();

	spi_ss(1);
	keys[0] = spi_write_byte(0);
	keys[1] = spi_write_byte(0);
	spi_ss(0);

	if (keys[0] != prev_keys[0] || keys[1] != prev_keys[1]) {
		prev_keys[0] = keys[0];
		prev_keys[1] = keys[1];
		print_keys(keys, 2);
	}
}
