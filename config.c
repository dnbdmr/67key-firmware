#include "config.h"

config_type config = {
	.debug = 0,

	.debouncems = 5,
	.pollms = 1,

	.scalex = -1,
	.scaley = -1,
	.swapxy = 1,

	.scrollscalex = -2,
	.scrollscaley = 2,
	.scrollswapxy = 1,

	.tp_sensitivity = 78
};

void config_init(void)
{
}
