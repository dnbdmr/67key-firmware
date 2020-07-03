/*
 * Copyright (c) 2016-2017, Alex Taradov <alex@taradov.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *	  this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *	  notice, this list of conditions and the following disclaimer in the
 *	  documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *	  derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*- Includes ----------------------------------------------------------------*/
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdalign.h>
#include <string.h>
#include "samd21.h"
#include "hal_gpio.h"
#include "nvm_data.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include "rgb.h"
#include "spi_master.h"
#include "matrix.h"
#include "keymap.h"
#include "config.h"

/*- Definitions -------------------------------------------------------------*/
HAL_GPIO_PIN(LED1,	A, 22);

RGB_type led[2];

/*- Implementations ---------------------------------------------------------*/

volatile uint32_t msticks = 0;

void SysTick_Handler(void)
{
       msticks++;
}

uint32_t millis(void)
{
	uint32_t m;
	__disable_irq();
	__DMB();
	m = msticks;
	__enable_irq();
	__DMB();
	return m;
}

void delay_us(uint32_t us)
{
	if (!us || (us >= SysTick->LOAD))
		return;
	if(!(SysTick->CTRL & SysTick_CTRL_ENABLE_Msk))
		return;
	us = F_CPU/1000000*us;
	uint32_t time = SysTick->VAL;
	while ((time - SysTick->VAL) < us);
}

void delay_ms(uint32_t ms)
{
	uint32_t tempms = millis();
	while((millis() - tempms) < ms);
}


//-----------------------------------------------------------------------------
void TC3_Handler(void)
{
	if (TC3->COUNT16.INTFLAG.reg & TC_INTFLAG_MC(1))
	{
		HAL_GPIO_LED1_toggle();
		TC3->COUNT16.INTFLAG.reg = TC_INTFLAG_MC(1);
	}
}

//-----------------------------------------------------------------------------
static void timer_ms(uint32_t ms) {
	TC3->COUNT16.CC[0].reg = (F_CPU / 1000ul / 1024) * ms;
	TC3->COUNT16.COUNT.reg = 0;
}

static void timer_init(void)
{
	PM->APBCMASK.reg |= PM_APBCMASK_TC3;

	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(TC3_GCLK_ID) |
		GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(0);

	TC3->COUNT16.CTRLA.reg = TC_CTRLA_MODE_COUNT16 | TC_CTRLA_WAVEGEN_MFRQ |
		TC_CTRLA_PRESCALER_DIV1024 | TC_CTRLA_PRESCSYNC_RESYNC;

	TC3->COUNT16.COUNT.reg = 0;

	TC3->COUNT16.CC[0].reg = (F_CPU / 1000ul / 1024) * 500; // 500ms
	TC3->COUNT16.COUNT.reg = 0;

	TC3->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE;

	TC3->COUNT16.INTENSET.reg = TC_INTENSET_MC(1);
	NVIC_EnableIRQ(TC3_IRQn);
}

//-----------------------------------------------------------------------------
static void sys_init(void)
{

	NVMCTRL->CTRLB.bit.RWS = 1; // Set Flash Wait States to 1 for 3.3V operation @ 48MHz

	SYSCTRL->DFLLCTRL.reg = 0; // See Errata 9905
	while (0 == (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY)); // Wait for DFLL sync complete

	SYSCTRL->DFLLMUL.reg = SYSCTRL_DFLLMUL_MUL(48000); // Set to multiply USB SOF frequency (when USB attached)
	
	uint32_t coarse, fine;
	coarse = NVM_READ_CAL(DFLL48M_COARSE_CAL); // Read factory cals for DFLL48M
	//fine = NVM_READ_CAL(DFLL48M_FINE_CAL);
	fine = 0xA; // D21 Fam Datasheet table 37-64 note 1
	SYSCTRL->DFLLVAL.reg = SYSCTRL_DFLLVAL_COARSE(coarse) | SYSCTRL_DFLLVAL_FINE(fine); // Load factory cals

	SYSCTRL->DFLLCTRL.reg = SYSCTRL_DFLLCTRL_ENABLE | 
							SYSCTRL_DFLLCTRL_USBCRM | // Set DFLL for USB Clock Recovery Mode
							//SYSCTRL_DFLLCTRL_BPLCKC | // Bypass Coarse Lock, ignored with USBCRM
							SYSCTRL_DFLLCTRL_CCDIS |  // Disable Chill Cycle
							SYSCTRL_DFLLCTRL_RUNSTDBY |  // Run during standby for USB wakeup interrupts
							SYSCTRL_DFLLCTRL_MODE;   // Set Closed Loop Mode
							//SYSCTRL_DFLLCTRL_STABLE; // Fine calibration register locks (stable) after fine lock, ignored with USBCRM

	while (!(SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY)); // Wait for DFLL sync complete

	//Setup Generic Clock Generator 0 with DFLL48M as source:
	GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(0) | GCLK_GENCTRL_SRC(GCLK_SOURCE_DFLL48M) |
		GCLK_GENCTRL_RUNSTDBY | GCLK_GENCTRL_GENEN;
	while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);
	
	//Disable OSC8M and generator 2 (enabled by UF2 bootloader)
	GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(2);
	GCLK->GENCTRL.bit.GENEN = 0;
	SYSCTRL->OSC8M.bit.ENABLE = 0;

	SysTick_Config(48000); //systick at 1ms
}

void usb_setup(void)
{
	// Enable USB Clocks
	PM->APBBMASK.reg |= PM_APBBMASK_USB;
	PM->AHBMASK.reg |= PM_AHBMASK_USB;
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_ID(USB_GCLK_ID) |
		GCLK_CLKCTRL_GEN(0);

	// Enable USB pins
	PORT->Group[0].PINCFG[PIN_PA24G_USB_DM].bit.PMUXEN = 1;
	PORT->Group[0].PMUX[PIN_PA24G_USB_DM/2].reg &= ~(0xF << (4 * (PIN_PA24G_USB_DM & 0x01u)));
	PORT->Group[0].PMUX[PIN_PA24G_USB_DM/2].reg |= MUX_PA24G_USB_DM << (4 * (PIN_PA24G_USB_DM & 0x01u));
	PORT->Group[0].PINCFG[PIN_PA25G_USB_DP].bit.PMUXEN = 1;
	PORT->Group[0].PMUX[PIN_PA25G_USB_DP/2].reg &= ~(0xF << (4 * (PIN_PA25G_USB_DP & 0x01u)));
	PORT->Group[0].PMUX[PIN_PA25G_USB_DP/2].reg |= MUX_PA25G_USB_DP << (4 * (PIN_PA25G_USB_DP & 0x01u));
}

//-----------------------------------------------------------------------------
// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us	to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
	(void) remote_wakeup_en;
	rgb_zero(1);
	HAL_GPIO_LED1_in();
	SysTick->CTRL &= ~(SysTick_CTRL_ENABLE_Msk); //disable systick
	SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk << SCB_SCR_SLEEPDEEP_Pos;
	__WFI();
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
	rgb_update(led, 2);
	HAL_GPIO_LED1_out();
	SysTick_Config(48000); //systick at 1ms
}

//-----------------------------------------------------------------------------
// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
	(void) itf;

	// connected
	if ( dtr && rts )
	{
		// print initial message when connected
	}

	//Reset into bootloader when baud is 1200 and dtr unasserted
	if (!dtr) {
		cdc_line_coding_t lc;
		tud_cdc_get_line_coding(&lc);
		if (lc.bit_rate == 1200) {
			unsigned long *a = (unsigned long *)(HMCRAMC0_ADDR + HMCRAMC0_SIZE - 4); // Make a boot key at end of RAM
			*a = 0xf01669ef; // Set boot key to uf2 magic value
			NVIC_SystemReset();
		}
	}
}

//-----------------------------------------------------------------------------
// Invoked when CDC interface received data from host
/*
void tud_cdc_rx_cb(uint8_t itf)
{
	(void) itf;
	//tud_cdc_write_str("Stop That!!\n");
	//tud_cdc_read_flush();

	if ( tud_cdc_connected() )
	{
		// connected and there are data available
		if ( tud_cdc_available() )
		{
			uint8_t buf[64];
			// read and echo back
			uint8_t count = tud_cdc_read(buf, sizeof(buf));

			for(uint32_t i=0; i<count; i++)
			{
				tud_cdc_write_char(buf[i]);
			}
		}
	}
}
*/

/* Retrieves full line from cdc, returns true when found */
uint8_t cdc_task(char line[], uint8_t max)
{
	static uint8_t pos = 0;
	uint8_t success = 0;

	if (tud_cdc_connected() && tud_cdc_available()) {	// connected and there are data available
		uint8_t buf[64];
		uint8_t count = tud_cdc_read(buf, sizeof(buf));

		for (uint8_t i=0; i<count; i++) {
			tud_cdc_write_char(buf[i]);
			if (pos < max-1) {
				if ((line[pos] = buf[i]) == '\n') {
					success = 1;
				}
				pos++;
			}
		}
	}

	tud_cdc_write_flush(); // Freeze without this

	if (success) {
		success = 0;
		line[pos] = '\0';
		pos = 0;
		return 1;
	}
	else
		return 0;
}

void cdc_write_num(int num, uint8_t radix)
{
	char s[25];
	itoa(num, s, radix);
	tud_cdc_write_str(s);
	tud_cdc_write_char('\n');
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

void hid_task(void)
{
  // Poll every 10ms
  const uint32_t interval_ms = 10;
  static uint32_t start_ms = 0;

  if ( millis() - start_ms < interval_ms) return; // not enough time
  start_ms += interval_ms;

  //uint32_t const btn = board_button_read();
  uint32_t const btn = shift_task();

  /*------------- Mouse -------------*/
  if ( tud_hid_ready() )
  {
    if ( btn )
    {
      int8_t const delta = 5;

      // no button, right + down, no scroll pan
      tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, delta, delta, 0, 0);

      // delay a bit before attempt to send keyboard report
	  while( !tud_hid_ready() )
		  tud_task();
    }
  }

  /*------------- Keyboard -------------*/
  if ( tud_hid_ready() )
  {
    if ( btn )
    {
      uint8_t keycode[6] = { 0 };
	  uint8_t modifiers = 0;
	  read_keys(keycode);
	  read_modifiers(&modifiers);

	  if (config.debug) {
		  tud_cdc_write_str("Mods: ");
		  cdc_write_num(modifiers, 2);
		  tud_cdc_write_str("Codes:\n");
		  for(uint8_t i = 0; i < 6; i++) {
			  cdc_write_num(keycode[i], 10);
		  }
		  tud_cdc_write_char('\n');
	  }

      tud_hid_keyboard_report(REPORT_ID_KEYBOARD, modifiers, keycode);
    }
  }
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  // TODO set LED based on CAPLOCK, NUMLOCK etc...
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) bufsize;
}

//-----------------------------------------------------------------------------
int main(void)
{
	sys_init();
	usb_setup();
	tusb_init();
	timer_init();
	rgb_init();
	spi_init(1000000, 0);
	config_init();


	HAL_GPIO_LED1_out();

	led[0].red = 0xFF;
	led[0].blue = 0x0;
	led[0].green = 0x0;
	led[0].bright = 0x02;
	led[1].red = 0xFF;
	led[1].blue = 0x0;
	led[1].green = 0xFF;
	led[1].bright = 0x05;
	rgb_update(led, 2);

	char s[25];
	uint32_t tenthmintick = millis();
	uint8_t ledpos = 0;

	while (1)
	{

		tud_task();
		hid_task();

		if (cdc_task(s, 25)) {
			if (s[0] == 'b') {
				uint32_t ms = atoi((const char *)&s[1]);
				if (ms > 0 && ms < 50000) {
					timer_ms(ms);
				}
			} else if (s[0] == 'd') {
				config.debug = !config.debug;
				cdc_write_num(config.debug, 10);
			}
		}

		if ((millis() - tenthmintick) >= 50) {
			tenthmintick = millis();
			rgb_wheel(&led[1], ledpos);
			rgb_update(led, 2);
			ledpos += 4;
		}
	}

	return 0;
}
