#include "config.h"

config_type config;

void config_init(void)
{
	config.debug = 1;
	config.invertx = 1;
	config.inverty = 1;
	config.swapxy = 1;
	config.tp_sensitivity = 0x80;
}

