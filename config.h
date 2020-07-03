#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdint.h>

typedef struct config_tag {
	uint8_t debug;
} config_type;

extern struct config_tag config;

void config_init(void);

#endif // _CONFIG_H_
