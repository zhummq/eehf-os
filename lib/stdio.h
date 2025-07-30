#ifndef __LIB_STDIO_H
#define __LIB_STDIO_H
#include "errno.h"
#include "file.h"
#include "stdarg.h"
#include "stdint.h"
uint32_t printf(const char *str, ...);
uint32_t vsprintf(char *str, const char *format, va_list ap);
uint32_t sprintf(char *buf, const char *format, ...);
int snprintf(char *str, uint32_t size, const char *format, ...);
int puts(const char *str);
int fprintf(FILE *stream, const char *format, ...);
int fclose(FILE *file);
FILE *fopen(const char *pathname, const char *mode);
uint32_t fread(void *buff, uint32_t size, uint32_t nmemb, FILE *stream);
uint32_t fwrite(void *buff, uint32_t size, uint32_t nmemb, FILE *stream);
long ftell(FILE *stream);
int remove(const char *filename);
int fseek(FILE *stream, long offset, int whence);
int rename(const char *oldpath, const char *newpath);
int vfprintf(FILE *stream, const char *format, va_list ap);
int vsnprintf(char *str, uint32_t size, const char *format, va_list ap);
int system(const char *command);
int fflush(FILE *stream);
double atof(const char *nptr);
#endif
