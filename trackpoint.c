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

/*- Includes ----------------------------------------------------------------*/
#include <stdint.h>
#include "sam.h"
#include "hal_gpio.h"
#include "trackpoint.h"
#include "config.h"
#include "tusb.h"
#include "utils.h"

static void gohi(uint8_t pin);
static void golo(uint8_t pin);

extern void delay_us(uint32_t);
extern void delay_ms(uint32_t);


/*- Definitions -------------------------------------------------------------*/
HAL_GPIO_PIN(TP_CLK, A, 4)
#define TPCLK 4
#define TP_EIC_CONFIG	EIC_CONFIG_SENSE4_FALL
#define TP_EIC_INTFLAG	EIC_INTFLAG_EXTINT4
#define TP_EIC_INTENSET	EIC_INTENSET_EXTINT4

HAL_GPIO_PIN(TP_DATA, A, 3)
#define TPDATA 3

HAL_GPIO_PIN(SPACEBAR, A, 11)

/*- Implementations ---------------------------------------------------------*/

static volatile uint8_t dataAvailable;
static volatile struct tp_DataReport data;


//-----------------------------------------------------------------------------
void tp_init(void)
{
	HAL_GPIO_TP_CLK_in();
	HAL_GPIO_TP_CLK_pullen(1);
	HAL_GPIO_TP_DATA_in();
	HAL_GPIO_TP_DATA_pullen(1);

	HAL_GPIO_SPACEBAR_pmuxen(PORT_PMUX_PMUXE_A);

	// enable EIC APBA clock line
	PM->APBAMASK.reg |= PM_APBAMASK_EIC;

	// enable EIC GCLK
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(EIC_GCLK_ID) |
		GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(0);
	while(GCLK->STATUS.bit.SYNCBUSY);

	// Set up EIC on TP_CLK pin
	EIC->CONFIG[0].reg |= TP_EIC_CONFIG; //TP_CLK
	EIC->WAKEUP.reg |= TP_EIC_INTENSET;

	// Set up EIC on Spacebar
	EIC->CONFIG[1].reg |= EIC_CONFIG_SENSE3_LOW; //Spacebar
	EIC->WAKEUP.reg |= EIC_WAKEUP_WAKEUPEN11;

	EIC->CTRL.reg = EIC_CTRL_ENABLE; 
	while(EIC->STATUS.bit.SYNCBUSY);
	NVIC_EnableIRQ(EIC_IRQn);

	tp_reset();
	tp_setStreamMode();
	tp_setSensitivityFactor(config.tp_sensitivity);
	tp_enableInt();
}

void tp_enableInt(void)
{
	EIC->INTENSET.reg = TP_EIC_INTENSET;
	HAL_GPIO_TP_CLK_pmuxen(PORT_PMUX_PMUXE_A);
	while(EIC->STATUS.bit.SYNCBUSY);
}

INLINE void gohi(uint8_t pin)
{
	PORT->Group[0].DIRCLR.reg = (1<<pin); // set as input
	//PORT->Group[0].OUTSET.reg = (1<<pin); // enable pullup
}

INLINE void golo(uint8_t pin)
{
	PORT->Group[0].OUTCLR.reg = (1<<pin); // set low
	PORT->Group[0].DIRSET.reg = (1<<pin); // set as output
}

/* write a uint8_t to the PS2 device */
void tp_write(uint8_t data)
{
	uint8_t i;
	uint8_t parity = 1;

	gohi(TPDATA);
	gohi(TPCLK);
	delay_us(300);
	golo(TPCLK);
	delay_us(300);
	golo(TPDATA);
	delay_us(10);
	gohi(TPCLK);	// start bit
	/* wait for device to take control of clock */
	while (HAL_GPIO_TP_CLK_read())
	;	// this loop intentionally left blank
	// clear to send data
	for (i=0; i < 8; i++)
	{
		if (data & 0x01)
		{
			gohi(TPDATA);
			} else {
			golo(TPDATA);
		}
		// wait for clock
		while (!HAL_GPIO_TP_CLK_read())
		;
		while (HAL_GPIO_TP_CLK_read())
		;
		parity = parity ^ (data & 0x01);
		data = data >> 1;
	}
	// parity bit
	if (parity)
		gohi(TPDATA);
	else
		golo(TPDATA);
	// clock cycle - like ack.
	while (!HAL_GPIO_TP_CLK_read())
	;
	while (HAL_GPIO_TP_CLK_read())
	;
	// stop bit
	gohi(TPDATA);
	delay_us(50);
	while (HAL_GPIO_TP_CLK_read())
	;
	// mode switch
	while (!HAL_GPIO_TP_CLK_read() || !HAL_GPIO_TP_DATA_read())
	;
	// hold up incoming data
	golo(TPCLK);
}


/*
* read a uint8_t of data from the ps2 device.  Ignores parity.
*/
uint8_t tp_read(void) {
	uint8_t data = 0x00;
	uint8_t i;
	uint8_t bit = 0x01;

	// start clock
	gohi(TPCLK);
	gohi(TPDATA);
	delay_us(50);
	while (HAL_GPIO_TP_CLK_read())
	;
	delay_us(5);	// not sure why.
	while (!HAL_GPIO_TP_CLK_read())
	;	// eat start bit
	for (i=0; i < 8; i++)
	{
		while (HAL_GPIO_TP_CLK_read())
		;
		if (HAL_GPIO_TP_DATA_read())
		{
			data = data | bit;
		}
		while (!HAL_GPIO_TP_CLK_read())
		;
		bit = bit << 1;
	}
	// eat parity bit, ignore it.
	while (HAL_GPIO_TP_CLK_read())
	;
	while (!HAL_GPIO_TP_CLK_read())
	;
	// eat stop bit
	while (HAL_GPIO_TP_CLK_read())
	;
	while (!HAL_GPIO_TP_CLK_read())
	;
	golo(TPCLK);	// hold incoming data

	return data;
}

// undefined in case of error
uint8_t tp_readFromRamLocation(uint8_t location)
{
	tp_write(0xe2);
	tp_read(); // ACK
	tp_write(0x80);
	tp_read(); // ACK
	tp_write(location);
	tp_read(); // ACK
	return tp_read();
}

// errors are ignored
void tp_writeToRamLocation(uint8_t location, uint8_t value) {
	tp_write(0xe2);
	tp_read(); // ACK
	tp_write(0x81);
	tp_read(); // ACK
	tp_write(location);
	tp_read(); // ACK
	tp_write(value);
	tp_read(); // ACK
}

void tp_setStreamMode(void)
{
	tp_write(0xea);
	tp_read();
	tp_write(0xf4); //enable report
	tp_read();

	//put mouse into idle mode, ready to send
	gohi(TPCLK);
	gohi(TPDATA);
}

void tp_getDataBit(void)
{
	static volatile uint8_t bitcount;
	static volatile uint8_t incoming;
	static volatile uint8_t bytecount;
	static volatile uint32_t timeout;

	uint8_t n = 0;
	uint8_t val = 0;

	/* Reset counters if we've waited more than 11ms. We are just starting a stream */
	if ((millis() - timeout) > 11) {
		if (bytecount) {
			bitcount = 0;
			incoming = 0;
			bytecount = 0;
		}
	}
	timeout = millis();

	dataAvailable = 0;

	val = (HAL_GPIO_TP_DATA_read() ? 1 : 0);
	n = bitcount-1;
	if(n <= 7) {
		incoming |= (val << n);
	}
	bitcount++;

	if(bitcount == 11) {
		switch(bytecount) {
			case 0:
				data.state = incoming;
				bytecount++;
				break;

			case 1:
				data.x = incoming;
				bytecount++;
				break;

			case 2:
				data.y = incoming;
				bytecount = 0;
				dataAvailable = 1;
				break;
		}
		bitcount = 0;
		incoming = 0;
	}
}

uint8_t tp_reportAvailable(void)
{
	return dataAvailable;
}

struct tp_DataReport tp_getStreamReport(void)
{
	dataAvailable = 0;
	return data;
}

void tp_reset(void)
{
	/*
	   an inverter RC circuit is being used, pull CLOCK low for
	   5ms to charge RESET line, then leave it there for 2s,
	   then pull CLOCK high for 5ms to discharge the RC circuit
	   to pull RESET line LOW.
	   */
	golo(TPCLK);
	delay_ms(400);
	gohi(TPCLK);
	delay_ms(50);

	gohi(TPCLK);
}

void tp_setSensitivityFactor(uint8_t sensitivityFactor)
{
	tp_writeToRamLocation(0x4a, sensitivityFactor);
}

void EIC_Handler(void)
{
	if (EIC->INTFLAG.reg & TP_EIC_INTFLAG) {
		if (EIC->CONFIG[0].bit.SENSE4 == EIC_CONFIG_SENSE4_LOW_Val) {
			EIC->CONFIG[0].bit.SENSE4 = EIC_CONFIG_SENSE4_FALL_Val; //TP_CLK
			while(EIC->STATUS.bit.SYNCBUSY);
		}
		EIC->INTFLAG.reg |= TP_EIC_INTFLAG;
		tp_getDataBit();
	}
	if (EIC->INTFLAG.reg & EIC_INTFLAG_EXTINT11) {
		EIC->INTENCLR.reg = EIC_INTENCLR_EXTINT11; //disable spacebar interrupt
		EIC->INTFLAG.reg |= EIC_INTFLAG_EXTINT11;
		while(EIC->STATUS.bit.SYNCBUSY);
	}

	if ( tud_suspended() )
		tud_remote_wakeup();
}
