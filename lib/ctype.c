#include "ctype.h"
int toupper(int c) {
  if (c >= 'a' && c <= 'z') {
    return c - ('a' - 'A'); // 或者：return c - 32;
  }
  return c;
}

char *strncpy(char *dest, const char *src, uint32_t n) {
  uint32_t i;
  for (i = 0; i < n && src[i] != '\0'; i++) {
    dest[i] = src[i];
  }
  for (; i < n; i++) {
    dest[i] = '\0';
  }
  return dest;
}
char *strstr(const char *haystack, const char *needle) {
  if (!*needle)
    return (char *)haystack;

  for (; *haystack; haystack++) {
    const char *h = haystack;
    const char *n = needle;
    while (*h && *n && *h == *n) {
      h++;
      n++;
    }
    if (!*n)
      return (char *)haystack;
  }
  return NULL;
}

int isspace(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' ||
         c == '\r';
}
int isdigit(char c) { return c >= '0' && c <= '9'; }
