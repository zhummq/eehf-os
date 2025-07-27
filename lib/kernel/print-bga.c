#include "print-bga.h"
#include "psf2.h"
#include "timer.h"
static uint32_t cursor_x =0;
static uint32_t cursor_y =0;
static uint8_t is_up;
#define WIDTH_NUM 1024/12
#define HEIGHT_NUM 768/29

void put_char_bga(uint8_t char_asci)
{
  if (char_asci == 0x0d || char_asci == 0xa)
  {
    if (cursor_x + 1 < HEIGHT_NUM)
    {
      cursor_x ++;
      cursor_y = 0;
    } else 
    {
      roll_screen();
      cursor_y = 0;
    }

  } else if (char_asci == 0x8)
  {
    draw_char(cursor_x,cursor_y,' ',0xffffffff,0xff000000);
    if (cursor_y > 0)
    {
      cursor_y --;
    } else 
    {
      if (cursor_x > 0)
      {
        cursor_x --;
      }
    }
    draw_char(cursor_x,cursor_y,' ',0xffffffff,0xff000000);
  } else 
  {
    draw_char(cursor_x,cursor_y,char_asci,0xffffffff,0xff000000);
    if (cursor_y + 1 < WIDTH_NUM)
    {
      cursor_y ++;
    } else 
    {
      if (cursor_x + 1 < HEIGHT_NUM)
      {
        cursor_x ++;
        cursor_y = 0;
      } else 
      {
        roll_screen();
        cursor_y = 0;
      }

    }
 
  }
}


void put_string_bga(char* meaasge)
{
  char c = *meaasge ++;
  while (c != 0)
  {
    put_char_bga((uint8_t)c);
    c = *meaasge ++;
  }
}

void sys_cursor(void)
{
  if (is_up)
  {
    draw_char(cursor_x,cursor_y,' ',0xffffffff,0xff000000);
    is_up = 0;
  } else 
  {
    draw_char(cursor_x,cursor_y,'_',0xffff0000,0xff000000);
    is_up = 1;
  }
}
