#include "font.h"
#include "fs.h"
#include "memory.h"
#define FONT_SIZE 29728

uint8_t* Solarize_12x29_psf;

void font_init(void)
{
  int32_t fd = sys_open("/font",O_RDONLY);
  Solarize_12x29_psf = sys_malloc(FONT_SIZE);
  sys_read(fd,Solarize_12x29_psf,FONT_SIZE);
  sys_close(fd);
}
