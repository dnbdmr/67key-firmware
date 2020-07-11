#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "samd21.h"
#include "hal_gpio.h"
#include "led.h"
#include "tusb.h"

HAL_GPIO_PIN(MOSI,		A, 0);	// SC1.0
HAL_GPIO_PIN(SCLK,		A, 1);	// SC1.1
#define SPI_SERCOM            SERCOM1
#define SPI_SERCOM_PMUX       PORT_PMUX_PMUXE_D_Val
#define SPI_SERCOM_GCLK_ID    SERCOM1_GCLK_ID_CORE
#define SPI_SERCOM_CLK_GEN    0
#define SPI_SERCOM_APBCMASK   PM_APBCMASK_SERCOM1
#define FREQ	2000000

RGB_type rgbarray[2];

HAL_GPIO_PIN(CAPSLOCK,	A, 23);
#define	CAPS_BLINK_RATE		100
HAL_GPIO_PIN(LED1,	A, 22);

static uint8_t state;

uint8_t gamma8[] = {
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
	1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
	2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
	5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
	10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
	17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
	25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
	37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
	51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
	69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
	90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
	115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
	144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
	177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
	215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };


void rgb_init(void)
{
	//set up RGB SPI
  int baud = F_CPU / (2 * FREQ) - 1;
  if (baud < 0)
    baud = 0;
  if (baud > 255)
    baud = 255;

  HAL_GPIO_MOSI_out();
  HAL_GPIO_MOSI_pmuxen(SPI_SERCOM_PMUX);
  HAL_GPIO_SCLK_out();
  HAL_GPIO_SCLK_pmuxen(SPI_SERCOM_PMUX);

  PM->APBCMASK.reg |= SPI_SERCOM_APBCMASK;
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(SPI_SERCOM_GCLK_ID) |
      GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(SPI_SERCOM_CLK_GEN);
  SPI_SERCOM->SPI.CTRLA.reg = SERCOM_SPI_CTRLA_SWRST;
  while (SPI_SERCOM->SPI.CTRLA.reg & SERCOM_SPI_CTRLA_SWRST);
  SPI_SERCOM->SPI.BAUD.reg = baud;
  SPI_SERCOM->SPI.CTRLA.reg = SERCOM_SPI_CTRLA_ENABLE |
      SERCOM_SPI_CTRLA_DOPO(0) |
      SERCOM_SPI_CTRLA_MODE_SPI_MASTER;

  rgbarray[0].red = 0x0;
  rgbarray[0].blue = 0x0;
  rgbarray[0].green = 0x0;
  rgbarray[0].bright = 0x05;
  rgbarray[1].red = 0xFF;
  rgbarray[1].blue = 0x0;
  rgbarray[1].green = 0x00;
  rgbarray[1].bright = 0x01;
  rgb_update(rgbarray, RGB_NUM);

  // set up LED PWM
  PM->APBCMASK.reg |= PM_APBCMASK_TC4;
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(TC4_GCLK_ID) |
	  GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(0);
  TC4->COUNT8.CTRLA.reg = TC_CTRLA_MODE_COUNT8 | TC_CTRLA_WAVEGEN_NPWM |
	  TC_CTRLA_PRESCALER_DIV1024 | TC_CTRLA_PRESCSYNC_RESYNC;
  TC4->COUNT8.CTRLC.reg = TC_CTRLC_INVEN0 | TC_CTRLC_INVEN1;
  TC4->COUNT8.PER.reg = 255;	// for 400Hz
  TC4->COUNT8.CC[0].reg = 10;	// board led
  TC4->COUNT8.CC[1].reg = 255;	// Capslock
  TC4->COUNT8.COUNT.reg = 0;
  TC4->COUNT8.CTRLA.reg |= TC_CTRLA_ENABLE;

  HAL_GPIO_LED1_out();
  HAL_GPIO_LED1_set();
  HAL_GPIO_LED1_pmux_set(PORT_PMUX_PMUXE_E);
  HAL_GPIO_CAPSLOCK_out();
  HAL_GPIO_CAPSLOCK_set();
  HAL_GPIO_CAPSLOCK_pmux_set(PORT_PMUX_PMUXE_E);

}

void rgb_sendbyte(uint8_t byte)
{
  SPI_SERCOM->SPI.DATA.reg = byte;
  while (!SPI_SERCOM->SPI.INTFLAG.bit.TXC);
}

void rgb_update(RGB_type *leds, uint8_t num)
{
    for(uint8_t i=0; i<4; i++){
        rgb_sendbyte(0x00);
    }

    for(uint8_t i=0; i<num; i++) {
        rgb_sendbyte(0xE0 | (leds[i].bright & 0x1F));
        rgb_sendbyte(leds[i].blue);
        rgb_sendbyte(leds[i].green);
        rgb_sendbyte(leds[i].red);
    }

    for(uint8_t i=0; i<4; i++){
        rgb_sendbyte(0xFF);
    }
}

void rgb_zero(uint8_t num)
{
    for(uint8_t i=0; i<4; i++){
        rgb_sendbyte(0x00);
    }

    for(uint8_t i=0; i<num; i++) {
        rgb_sendbyte(0xE0);
        rgb_sendbyte(0x00);
        rgb_sendbyte(0x00);
        rgb_sendbyte(0x00);
    }

    for(uint8_t i=0; i<4; i++){
        rgb_sendbyte(0xFF);
    }
}

/* Only for single led */
void rgb_wheel(RGB_type *led, uint8_t pos)
{
	if (pos < 85) {
		led->blue = gamma8[85 - pos];
		led->green = gamma8[pos];
		led->red = 0;
		return;
	}
	if (pos < 170) {
		pos -= 85;
		led->blue = 0;
		led->green = gamma8[85 - pos];
		led->red = gamma8[pos * 2];
		return;
	}
	pos -= 170;
	led->blue = gamma8[pos];
	led->green = 0;
	led->red = gamma8[170 - pos * 2];
	return;
}

void led_task(void)
{
	static uint32_t caps_time = 0;
	static uint32_t led1_time = 0;
	static uint32_t rgb0_time = 0;
	static uint32_t rgb1_time = 0;
	static uint8_t rgb0_pos = 0;
	static uint8_t rgb0_dir = 0;
	static uint8_t rgb1_pos = 0;

	uint32_t cur_time = millis();
	if (cur_time - caps_time > CAPS_BLINK_RATE) {
		caps_time = cur_time;

		if (state & KEYBOARD_LED_CAPSLOCK) {
			HAL_GPIO_CAPSLOCK_pmux_toggle();
		}
		else
			HAL_GPIO_CAPSLOCK_pmuxdis();
	}

	if (cur_time - led1_time > 500) {
		led1_time = cur_time;

		HAL_GPIO_LED1_pmux_toggle();
	}

	if ((cur_time - rgb1_time) >= 50) {
		rgb1_time = cur_time;
		rgb_wheel(&rgbarray[1], rgb1_pos);
		rgb_update(rgbarray, RGB_NUM);
		rgb1_pos += 4;
	}

	if ((cur_time - rgb0_time) >= 20) {
		rgb0_time = cur_time;
		if (rgb0_pos >= 172)
			rgb0_dir = 0;
		if (rgb0_pos <= 72)
			rgb0_dir = 1;
		if (rgb0_dir)
			rgb0_pos+=4;
		else
			rgb0_pos-=4;
		rgbarray[0].blue = gamma8[rgb0_pos];
		rgb_update(rgbarray, RGB_NUM);
	}
}

void led_update(uint8_t buffer)
{
	state = buffer;
}

void led_brightness(uint8_t bright) {
	TC4->COUNT8.CC[0].reg = bright; // board led
}

void led_off(void)
{
	rgb_zero(RGB_NUM);
	HAL_GPIO_CAPSLOCK_pmuxdis();
	HAL_GPIO_LED1_pmuxdis();
}

void led_on(void)
{
	rgb_update(rgbarray, RGB_NUM);
}
