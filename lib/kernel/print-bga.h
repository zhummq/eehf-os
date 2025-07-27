#ifndef _PRINTF_VGA_H
#define _PRINTF_VGA_H
#include "stdint.h"
#include "bga.h"
void put_char_bga(uint8_t char_asci);
void put_string_bga(char* meaasge);
void sys_cursor(void);
#endif
