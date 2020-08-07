#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdint.h>

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

	uint8_t tp_sensitivity;
} config_t;

extern config_t config;

void config_init(void);
uint8_t config_check_version(void);
uint8_t config_read_eeprom(void);
uint8_t config_write_eeprom(void);

#endif // _CONFIG_H_
