#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdint.h>

typedef struct config_tag {
	uint8_t debug;

	int8_t scalex;
	int8_t scaley;
	uint8_t swapxy;

	int8_t scrollscalex;
	int8_t scrollscaley;
	uint8_t scrollswapxy;

	uint8_t tp_sensitivity;
} config_type;

extern config_type config;

void config_init(void);

#endif // _CONFIG_H_
