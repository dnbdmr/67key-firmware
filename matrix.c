#include "matrix.h"
#include <string.h>
#include "spi_master.h"
#include "tusb.h"
#include "config.h"

extern uint32_t millis(void);

uint8_t prev_keys[MATRIX_REG_COUNT];

void print_keys(uint8_t keyarray[], uint8_t num) 
{
	if (!tud_cdc_connected() && tud_cdc_write_available() > 20)
		return;
	tud_cdc_write_char('\n');
	tud_cdc_write_str("0b");
	for (uint8_t a = 0; a < num; a++) {
		while (tud_cdc_write_available() < 20) {
			tud_cdc_write_flush();
			tud_task();
		}
		for (uint8_t i = 0; i < 8; i++) {
			tud_cdc_write_char((keyarray[a] & (1 << i)) ? '1' : '0');
		}
		tud_cdc_write_char(' ');
	}
}

uint8_t shift_task(void) {
	static uint32_t calltimer;
	static uint32_t bouncetimer;
	static uint8_t stable = 0;

	if ((millis() - calltimer) < config.pollms)
		return 0;
	calltimer = millis();

	uint8_t new_keys[MATRIX_REG_COUNT];

	spi_ss(1);
	for (int i = 0; i < MATRIX_REG_COUNT; i++) {
		new_keys[i] = spi_write_byte(0);
		if (MATRIX_REG_INVERT & (1<<i))
			new_keys[i] = ~new_keys[i];
	}
	spi_ss(0);

	// If keys changed, copy new to old and reset timer
	if (memcmp(prev_keys, new_keys, sizeof(prev_keys))) {
		memcpy(prev_keys, new_keys, sizeof(prev_keys));
		if (config.debug)
			print_keys(new_keys, MATRIX_REG_COUNT);
		bouncetimer = millis();
		stable = 0;
	} 

	if ((millis() - bouncetimer) >= config.debouncems) { // and debouncems has passed
		if (stable) { // we've already sent update
			return 0;
		}
		else { // we haven't sent update yet
			stable = 1;
			return 1;
		}
	}
	else // not enough time
		return 0;

	return 0;
}
