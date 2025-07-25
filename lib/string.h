#ifndef __LIB_STRING_H
#define __LIB_STRING_H
#include "stdint.h"
void memset(void* dst_, uint8_t value, uint32_t size);
void memcpy(void* dst_, const void* src_, uint32_t size);
int memcmp(const void* a_, const void* b_, uint32_t size);
char* strcpy(char* dst_, const char* src_);
uint32_t strlen(const char* str);
int8_t strcmp (const char *a, const char *b); 
char* strchr(const char* string, const uint8_t ch);
char* strrchr(const char* string, const uint8_t ch);
char* strcat(char* dst_, const char* src_);
uint32_t strchrs(const char* filename, uint8_t ch);
int strcasecmp(const char *s1, const char *s2);
int strncasecmp(const char *s1, const char *s2, uint32_t n);
int strncmp(const char *s1, const char *s2, uint32_t n);
int atoi(const char *str);
int toupper(int c);
char *strdup(const char *s);
char *strncpy(char *dest, const char *src, uint32_t n);
char *strstr(const char *haystack, const char *needle);
int isspace(char c);
int isdigit(char c);
void *calloc(uint32_t nmemb, uint32_t size);
#endif
