/*
 * Adapted from https://github.com/rampadc/arduino-trackpoint-extended
 * Modified work Copyright (c) 2020,  DNBDMR
 * Original work Copyright (c) 2014 rampadc
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _TRACKPOINT_H_
#define _TRACKPOINT_H_

/*- Prototypes --------------------------------------------------------------*/

void tp_init(void);
void tp_write(uint8_t data);
uint8_t tp_read(void);
void tp_setStreamMode();
void tp_getDataBit(void);
uint8_t tp_reportAvailable(void);
void tp_enableInt(void);
void tp_reset(void);
uint8_t tp_readFromRamLocation(uint8_t location);
void tp_setSensitivityFactor(uint8_t sensitivityFactor);
void tp_writeToRamLocation(uint8_t location, uint8_t value);

struct tp_DataReport {
	uint8_t state;
	int8_t x;
	int8_t y;
};

struct tp_DataReport tp_getStreamReport(void);

#endif // _TRACKPOINT_H_

