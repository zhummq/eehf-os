#ifndef _VGA_H_
#define _VGA_H_
#include "stdint.h"

#define VGA_QEMU_VENDER_ID 0x1234
#define VGA_QEMU_DEVICE_ID 0x1111
#define WIN_HEIGHT 768
#define WIN_WIDTH 1024
void bga_init(void);
void put_piexl(uint32_t x, uint32_t y, uint32_t color);
void roll_screen(void);
void cls_vga(void);
#endif
