#ifndef _PSF2_H_
#define _PSF2_H_
#include "stdint.h"

struct psf2_head_t {
  uint32_t magic;
  uint32_t version;
  uint32_t head_size;
  uint32_t flags;
  uint32_t length;
  uint32_t glyph_size;
  uint32_t glyph_height;
  uint32_t glyph_width;
};
void draw_char(int x,int y,uint8_t c,uint32_t fg,uint32_t bg);
#endif
