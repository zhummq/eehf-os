#ifndef __DEVICE_KEYBOARD_H
#define __DEVICE_KEYBOARD_H
void keyboard_init(void); 
extern struct ioqueue kbd_buf;
extern struct ioqueue kbd_keycode_buf;
char sys_get_keycode(void);
#endif
