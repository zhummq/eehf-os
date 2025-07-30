#ifndef _STRINGS_H_
#define _STRINGS_H_
#include "stdint.h"
int strcasecmp(const char *s1, const char *s2);
int strncasecmp(const char *s1, const char *s2, uint32_t n);
int strncmp(const char *s1, const char *s2, uint32_t n);
#endif
