#include <stdbool.h>
#include "config.h"
#include "i2c_master.h"
#include "tusb.h"

config_t config; // Main config in RAM

const config_t config_defaults = { // Defaults in Flash
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
	if (config_check_eep_version() == config_defaults.version)
		config_read_eeprom();
	else
		memcpy(&config, &config_defaults, sizeof(config_t));
}

uint8_t config_check_eep_version(void)
{
	uint8_t byte;

	// Set address to zero
	i2c_start(EEP_ADDR(0x0) | I2C_TRANSFER_WRITE);
	i2c_write_byte(0x0);
	
	// Repated start to stay in read mode
	if (!i2c_start(EEP_ADDR(0x0) | I2C_TRANSFER_READ))
		return 0; // Start failed, return invalid version
	if (!i2c_read_byte(&byte, true))
		return 0; // Read failed, return invalid version
	i2c_stop();
	return byte;
}

uint8_t config_check_ram_version(void)
{
	return config.version;
}

uint8_t config_check_default_version(void)
{
	return config_defaults.version;
}

// Read config from eeprom, return true on success
bool config_read_eeprom(void)
{
	uint8_t *cfgptr = (uint8_t *)&config;
	uint32_t len = sizeof(config_t);

	// Reset address to zero
	if (!i2c_start(EEP_ADDR(0x0) | I2C_TRANSFER_WRITE))
		return false;
	if (!i2c_write_byte(0x0))
		return false;

	// Repeated start to stay in read mode
	i2c_start(EEP_ADDR(0x0) | I2C_TRANSFER_READ);
	while (len-- > 1) {
		if(!i2c_read_byte(cfgptr++, false)) { // Read bytes with ack
			config_load_defaults(); // Read failed, load defaults
			return false;
		}
	}
	if (!i2c_read_byte(cfgptr, true)) {	// Read last byte, and nak
		config_load_defaults();	// Read failed, load defaults
		return false;
	}
	i2c_stop();

	return true; // Got to stop, success!
}

// Write config to eeprom, return true on success
bool config_write_eeprom(void)
{
	uint32_t eepaddr = 0;
	uint8_t *cfgptr = (uint8_t *)&config;
	uint32_t len = sizeof(config_t);

	// Repeatedly write 16 byte eeprom pages
	while (eepaddr < len) {
		if (!i2c_start(EEP_ADDR(eepaddr) | I2C_TRANSFER_WRITE)) // Write device addr and first address byte
			return false;
		if (!i2c_write_byte((eepaddr & ~0x7) & 0xff)) // Write second address byte
			return false;

		for (uint8_t i = 0; i < 16; i++) {
			if (eepaddr < len)
				eepaddr++;
			else // Reached end of config
				break;

			if(!i2c_write_byte(*cfgptr++))
				return false; // Write failed
		}
		i2c_stop();

		while (i2c_busy(EEP_ADDR(eepaddr))); // Wait for page write
	}

	return true;
}

void config_load_defaults(void)
{
	memcpy(&config, &config_defaults, sizeof(config_t));
}
