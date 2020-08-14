#include <stdbool.h>
#include "config.h"
#include "i2c_master.h"
#include "tusb.h"

config_t config;

const config_t config_defaults = {
	.version = 'B',

	.debug = 0,

	.debouncems = 5,
	.pollms = 1,

	.scalex = -1,
	.scaley = -1,
	.swapxy = 1,

	.scrollscalex = -1,
	.scrollscaley = 1,
	.scrollswapxy = 1,
	.scrolllim = 2,

	.tp_sensitivity = 78
};

void config_init(void)
{
	i2c_init(400000);

	// Load from eeprom if version is the same.
	if (config_check_version() == config_defaults.version)
		config_read_eeprom();
	else
		memcpy(&config, &config_defaults, sizeof(config_t));
}

uint8_t config_check_version(void)
{
	uint8_t byte;

	// Set address to zero
	i2c_start(EEP_ADDR(0x0) | I2C_TRANSFER_WRITE);
	i2c_write_byte(0x0);
	
	// Repated start to stay in read mode
	i2c_start(EEP_ADDR(0x0) | I2C_TRANSFER_READ);
	i2c_read_byte(&byte, true);
	i2c_stop();
	return byte;
}

uint8_t config_read_eeprom(void)
{
	uint8_t *cfgptr = (uint8_t *)&config;
	uint32_t len = sizeof(config_t);

	// Reset address to zero
	i2c_start(EEP_ADDR(0x0) | I2C_TRANSFER_WRITE);
	i2c_write_byte(0x0);

	// Repeated start to stay in read mode
	i2c_start(EEP_ADDR(0x0) | I2C_TRANSFER_READ);
	while (len-- > 1) {
		i2c_read_byte(cfgptr++, false);
	}
	i2c_read_byte(cfgptr, true);
	i2c_stop();

	return 1; //TODO: add checks, bools?
}

uint8_t config_write_eeprom(void)
{
	uint32_t eepaddr = 0;
	uint8_t *cfgptr = (uint8_t *)&config;
	uint32_t len = sizeof(config_t);

	while (eepaddr < len) {
		i2c_start(EEP_ADDR(eepaddr) | I2C_TRANSFER_WRITE);
		i2c_write_byte((eepaddr & ~0x7) & 0xff);

		for (uint8_t i = 0; i < 16; i++) {
			if (eepaddr < len)
				eepaddr++;
			else
				break;

			i2c_write_byte(*cfgptr++);
		}
		i2c_stop();

		while (i2c_busy(EEP_ADDR(eepaddr)));
	}
	return 1;
}

void config_load_defaults(void)
{
	memcpy(&config, &config_defaults, sizeof(config_t));
}
