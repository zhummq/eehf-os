#include "strings.h"
int strcasecmp(const char *s1, const char *s2) {
  while (*s1 && *s2) {
    char c1 = *s1++;
    char c2 = *s2++;

    if (c1 >= 'A' && c1 <= 'Z')
      c1 += 'a' - 'A';
    if (c2 >= 'A' && c2 <= 'Z')
      c2 += 'a' - 'A';

    if (c1 != c2)
      return (unsigned char)c1 - (unsigned char)c2;
  }

  return (unsigned char)*s1 - (unsigned char)*s2;
}
int strncasecmp(const char *s1, const char *s2, uint32_t n) {
  while (n-- > 0) {
    unsigned char c1 = *s1++;
    unsigned char c2 = *s2++;

    if (c1 >= 'A' && c1 <= 'Z')
      c1 += 'a' - 'A';
    if (c2 >= 'A' && c2 <= 'Z')
      c2 += 'a' - 'A';

    if (c1 != c2)
      return c1 - c2;

    if (c1 == '\0') // 遇到结束符
      break;
  }

  return 0;
}

int strncmp(const char *s1, const char *s2, uint32_t n) {
  while (n--) {
    unsigned char c1 = (unsigned char)*s1++;
    unsigned char c2 = (unsigned char)*s2++;

    if (c1 != c2)
      return c1 - c2;
    if (c1 == '\0')
      return 0;
  }
  return 0;
}
