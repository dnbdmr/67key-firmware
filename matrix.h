#ifndef _MATRIX_H_
#define _MATRIX_H_

#include <stdint.h>

#define MATRIX_REG_COUNT	9
#define MATRIX_REG_INVERT	(1<<1)

void print_keys(uint8_t keyarray[], uint8_t num);

uint8_t shift_task(void);

#endif // _MATRIX_H_
