#include "stdio.h"
#include "file.h"
#include "global.h"
#include "stdint.h"
#include "string.h"
#include "syscall.h"
#include <stdint.h>

int errno = 0;

/* 将整型转换成字符(integer to ascii) */
static uint32_t itoa(uint32_t value, char **buf_ptr_addr, uint8_t base) {
  uint32_t num = 0;
  uint32_t m = value % base; // 求模,最先掉下来的是最低位
  uint32_t i = value / base; // 取整
  if (i) {                   // 如果倍数不为0则递归调用。
    itoa(i, buf_ptr_addr, base);
  }
  if (m < 10) {                     // 如果余数是0~9
    *((*buf_ptr_addr)++) = m + '0'; // 将数字0~9转换为字符'0'~'9'
    num++;
  } else {                               // 否则余数是A~F
    *((*buf_ptr_addr)++) = m - 10 + 'A'; // 将数字A~F转换为字符'A'~'F'
    num++;
  }
  return num;
}

/* 将参数ap按照格式format输出到字符串str,并返回替换后str长度 */
uint32_t vsprintf(char *str, const char *format, va_list ap) {
  char *buf_ptr = str;
  const char *index_ptr = format;
  char index_char = *index_ptr;
  int32_t arg_int;
  char *arg_str;
  while (index_char) {
    if (index_char != '%') {
      *(buf_ptr++) = index_char;
      index_char = *(++index_ptr);
      continue;
    }
    index_char = *(++index_ptr); // 得到%后面的字符
    switch (index_char) {
    case 's':
      arg_str = va_arg(ap, char *);
      strcpy(buf_ptr, arg_str);
      buf_ptr += strlen(arg_str);
      index_char = *(++index_ptr);
      break;
    case 'c':
      *(buf_ptr++) = va_arg(ap, char);
      index_char = *(++index_ptr);
      break;
    case 'i':
    case 'd':
      arg_int = va_arg(ap, int);
      if (arg_int < 0) {
        arg_int =
            0 - arg_int; /* 若是负数, 将其转为正数后,再正数前面输出个负号'-'. */
        *buf_ptr++ = '-';
      }
      itoa(arg_int, &buf_ptr, 10);
      index_char = *(++index_ptr);
      break;
    case 'x':
      arg_int = va_arg(ap, int);
      itoa(arg_int, &buf_ptr, 16);
      index_char = *(++index_ptr); // 跳过格式字符并更新index_char
      break;
    }
  }
  return strlen(str);
}

/* 格式化输出字符串format */
uint32_t printf(const char *format, ...) {
  va_list args;
  va_start(args, format); // 使args指向format
  char buf[1024] = {0};   // 用于存储拼接后的字符串
  vsprintf(buf, format, args);
  va_end(args);
  return write(1, buf, strlen(buf));
}

/* 同printf不同的地方就是字符串不是写到终端,而是写到buf中 */
uint32_t sprintf(char *buf, const char *format, ...) {
  va_list args;
  uint32_t retval;
  va_start(args, format);
  retval = vsprintf(buf, format, args);
  va_end(args);
  return retval;
}

int snprintf(char *str, uint32_t size, const char *format, ...) {
  va_list args;
  int written;

  va_start(args, format);
  written = vsprintf(str, format, args); // 用已有的 vsprintf（或 sprintf）
  va_end(args);

  // 如果写入内容超过了 size，则进行截断
  if ((uint32_t)written >= size && size > 0) {
    str[size - 1] = '\0'; // 确保字符串以 null 结尾
  }

  return written;
}
int puts(const char *str) {
  while (*str) {
    putchar(*str++); // 你自己实现的输出单个字符的函数
  }
  putchar('\n'); // 添加换行
  return 0;
}
int fprintf(FILE *stream, const char *format, ...) {
  char buf[256]; // 可根据需求扩大
  va_list args;
  va_start(args, format);
  int len = snprintf(buf, sizeof(buf), format, args);
  va_end(args);

  if (len < 0)
    return -1;
  return write(stream->fd, buf, len);
}

FILE *fopen(const char *pathname, uint8_t flags) {
  FILE *file = (FILE *)malloc(sizeof(FILE));
  uint32_t fd = open(pathname, flags);
  file->fd = fd;
  return file;
}
int fclose(FILE *file) {
  // free
  int result = close(file->fd);
  free(file);
  return result;
}
uint32_t fread(void *buff, uint32_t size, uint32_t nmemb, FILE *stream) {
  return read(stream->fd, buff, nmemb * size);
}
uint32_t fwrite(void *buff, uint32_t size, uint32_t nmemb, FILE *stream) {
  return write(stream->fd, buff, nmemb * size);
}

long ftell(FILE *stream) { return lseek(stream->fd, 0, SEEK_CUR); }
int remove(const char *filename) {
  return unlink(filename); // 或你自己的系统调用
}
int fseek(FILE *stream, long offset, int whence) {
  return lseek(stream->fd, offset, whence);
}
int rename(const char *oldpath, const char *newpath) { return 1; }
int vfprintf(FILE *stream, const char *format, va_list ap) {
  char *str;
  return vsprintf(str, format, ap);
}
int vsnprintf(char *str, uint32_t size, const char *format, va_list ap) {
  char *buf_ptr = str;
  const char *index_ptr = format;
  char index_char = *index_ptr;
  int32_t arg_int;
  char *arg_str;
  uint32_t written = 0;
  while (index_char && written + 1 < size) {
    if (index_char != '%') {
      written++;
      if (written + 1 >= size) {
        break;
      }
      *(buf_ptr++) = index_char;
      index_char = *(++index_ptr);
      continue;
    }
    index_char = *(++index_ptr); // 得到%后面的字符
    switch (index_char) {
    case 's':
      arg_str = va_arg(ap, char *);
      written += strlen(arg_str);
      if (written + 1 >= size) {
        break;
      }
      strcpy(buf_ptr, arg_str);
      buf_ptr += strlen(arg_str);
      index_char = *(++index_ptr);
      break;
    case 'c':
      written++;
      if (written + 1 >= size) {
        break;
      }
      *(buf_ptr++) = va_arg(ap, char);
      index_char = *(++index_ptr);
      break;
    case 'i':
    case 'd':
      arg_int = va_arg(ap, int);
      if (arg_int < 0) {
        arg_int =
            0 - arg_int; /* 若是负数, 将其转为正数后,再正数前面输出个负号'-'. */
        written++;
        if (written + 1 >= size) {
          break;
        }
        *buf_ptr++ = '-';
      }
      written += itoa(arg_int, &buf_ptr, 10);
      if (written + 1 >= size) {
        break;
      }
      index_char = *(++index_ptr);
      break;
    case 'x':
      // TO DO:written ++?
      arg_int = va_arg(ap, int);
      written += itoa(arg_int, &buf_ptr, 16);
      if (written + 1 >= size) {
        break;
      }
      index_char = *(++index_ptr); // 跳过格式字符并更新index_char
      break;
    }
  }
  str[written] = 0;
  return strlen(str);
}
int system(const char *command) { return 0; }
int fflush(FILE *stream) { return 0; }
double atof(const char *nptr) { return 0; }
