#ifndef _VGA_H_
#define _VGA_H_
#include "stdint.h"

#define VGA_QEMU_VENDER_ID 0x1234
#define VGA_QEMU_DEVICE_ID 0x1111

void vga_init(void);
void put_piexl(uint32_t x,uint32_t y,uint32_t color);
#endif
