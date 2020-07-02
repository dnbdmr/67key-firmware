#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "samd21.h"
#include "hal_gpio.h"
#include "rgb.h"

HAL_GPIO_PIN(MOSI,            A, 0);	// SC1.0
HAL_GPIO_PIN(SCLK,            A, 1);	// SC1.1
#define SPI_SERCOM            SERCOM1
#define SPI_SERCOM_PMUX       PORT_PMUX_PMUXE_D_Val
#define SPI_SERCOM_GCLK_ID    SERCOM1_GCLK_ID_CORE
#define SPI_SERCOM_CLK_GEN    0
#define SPI_SERCOM_APBCMASK   PM_APBCMASK_SERCOM1
#define FREQ	1000000

void rgb_init(void)
{
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

  rgb_zero(RGB_NUM);
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
		led->blue = 85 - pos;
		led->green = pos;
		led->red = 0;
		return;
	}
	if (pos < 170) {
		pos -= 85;
		led->blue = 0;
		led->green = 85 - pos;
		led->red = pos * 2;
		return;
	}
	pos -= 170;
	led->blue = pos;
	led->green = 0;
	led->red = 170 - pos * 2;
	return;
}

