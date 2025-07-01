#ifndef __DEVICE_TIME_H
#define __DEVICE_TIME_H
#include "stdint.h"
extern uint32_t ticks;
void timer_init(void);
void mtime_sleep(uint32_t m_seconds);
#endif

