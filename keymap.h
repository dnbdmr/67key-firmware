#ifndef _KEYMAP_H_
#define _KEYMAP_H_

uint8_t read_keys(uint8_t keycodes[]);
uint8_t read_modifiers(uint8_t *modifiers);
uint8_t read_mousekeys(uint8_t *mousekeys, uint8_t *fn_key);
uint8_t read_consumer(uint16_t *button);

#endif // _KEYMAP_H_
