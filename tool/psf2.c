#include "psf2.h"
#include "font.h"
#include "bga.h"

void draw_char(int x,int y,uint8_t c,uint32_t fg,uint32_t bg){
  
  struct psf2_head_t * psf2_head = (struct psf2_head_t *) Solarize_12x29_psf;
  uint32_t offset = psf2_head->head_size + c * psf2_head->glyph_size;
  uint32_t width_byte = psf2_head->glyph_size / psf2_head->glyph_height;
  uint32_t x_piexl = x * psf2_head->glyph_height;
  uint32_t y_piexl = y * (psf2_head->glyph_width);
  for (uint32_t i = 0;i < psf2_head->glyph_height; i++){
    uint32_t count = 0;
    for (uint32_t j = 0;j < width_byte;j++){
      uint8_t bitmap = *(Solarize_12x29_psf+offset);
      uint8_t umask = 0x80;
      for (int h = 0;h < 8;h++){
        if (count == psf2_head->glyph_width){
          break;
        }
        uint8_t mask = (umask >> h);
        put_piexl(x_piexl,y_piexl + count,(mask & bitmap)?fg:bg);
        count++;
      }
      offset ++;
    }
    x_piexl ++;
  }
}
