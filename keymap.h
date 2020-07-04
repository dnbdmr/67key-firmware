#ifndef _KEYMAP_H_
#define _KEYMAP_H_

void read_keys(uint8_t keycodes[]);
void read_modifiers(uint8_t *modifiers);
void read_mousekeys(uint8_t *mousekeys, uint8_t *fn_key);
void read_consumer(uint16_t *button);

#endif // _KEYMAP_H_
