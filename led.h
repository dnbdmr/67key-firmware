/* APA102/DotStar bitbang library for onboard dotstar on ItsyBitsy M0 express */
#ifndef _LED_H_
#define _LED_H_

#include <stdint.h>

#define RGB_NUM	2

typedef struct {
    uint8_t bright;
    uint8_t blue;
    uint8_t green;
    uint8_t red;
} RGB_type;

void rgb_sendbyte(uint8_t byte);
void rgb_update(RGB_type *leds, uint8_t num);
void rgb_init(void);
void rgb_zero(uint8_t num);
void rgb_wheel(RGB_type *led, uint8_t pos);

void led_task(void);
void led_update(uint8_t buffer);
void led_off(void);
void led_on(void);
void led_brightness(uint8_t bright);

extern uint32_t millis(void);

#endif // _LED_H_ 
