#ifndef __CTYPE_H_
#define __CTYPE_H_
#include "stdint.h"
int toupper(int c);
char *strncpy(char *dest, const char *src, uint32_t n);
char *strstr(const char *haystack, const char *needle);
int isspace(char c);
int isdigit(char c);
#endif
