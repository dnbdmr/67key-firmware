#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdint.h>
#include <stdbool.h>

#define EEP_ADDR(addr)	0b10100000 | (((addr>>8) & 0b111)<<1)

typedef struct PACK {
	uint8_t version;

	uint8_t debug;

	uint8_t debouncems;
	uint8_t pollms;

	int8_t scalex;
	int8_t scaley;
	uint8_t swapxy;

	int8_t scrollscalex;
	int8_t scrollscaley;
	uint8_t scrollswapxy;
	uint8_t scrolllim;

	uint8_t tp_sensitivity;
} config_t;

extern config_t config; // Main config in RAM

void config_init(void);
uint8_t config_check_eep_version(void);
uint8_t config_check_ram_version(void);
uint8_t config_check_default_version(void);
bool config_read_eeprom(void);
bool config_write_eeprom(void);
void config_load_defaults(void);

#endif // _CONFIG_H_
