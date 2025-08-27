#include "doomgeneric.h"
#include "doomkeys.h"
#include "stdint.h"
#include "syscall.h"
static int32_t is_exten = 0;
static unsigned char convert_doomkey(uint16_t keycode) {
  switch (keycode) {
  case 0xe04d:
    return KEY_RIGHTARROW;
  case 0xe04b:
    return KEY_LEFTARROW;
  case 0xe048:
    return KEY_UPARROW;
  case 0xe050:
    return KEY_DOWNARROW;
  case 0x001e:
    return KEY_STRAFE_L;
  case 0x0020:
    return KEY_STRAFE_R;
  case 0x0039:
    return KEY_USE;
  case 0x001d:
    return KEY_FIRE;
  case 0x0001:
    return KEY_ESCAPE;
  case 0x001c:
    return KEY_ENTER;
  case 0x000f:
    return KEY_TAB;
  case 0x003b:
    return KEY_F1;
  case 0x003c:
    return KEY_F2;
  case 0x003d:
    return KEY_F3;
  case 0x003e:
    return KEY_F4;
  case 0x003f:
    return KEY_F5;
  case 0x0040:
    return KEY_F6;
  case 0x0041:
    return KEY_F7;
  case 0x0042:
    return KEY_F8;
  case 0x0043:
    return KEY_F9;
  case 0x0044:
    return KEY_F10;
  case 0x0057:
    return KEY_F11;
  case 0x0058:
    return KEY_F12;
  case 0x000e:
    return KEY_BACKSPACE;
  case 0x0034:
    return KEY_PAUSE;
  default:
    return keycode & 0xff;
  }
}
void DG_Init(void) {}
void DG_DrawFrame(void) {
  draw_piexl(DOOMGENERIC_RESY, DOOMGENERIC_RESX, (uint32_t)DG_ScreenBuffer);
}
void DG_SleepMs(uint32_t ms) { msleep(ms); }
uint32_t DG_GetTicksMs() { return get_ms(); }
int DG_GetKey(int *pressed, unsigned char *key) {
  uint16_t keycode = (uint8_t)get_keycode();
  if (keycode) {

    if (keycode == 0xe0) {
      is_exten = 1;
      return 0;
    }
    if (is_exten) {
      keycode = ((0xe000) | (keycode));
      is_exten = 0;
    }
    *pressed = !(keycode & 0x80);
    *key = convert_doomkey(keycode & 0xff7f);
    return 1;
  }
  return 0;
}
void DG_SetWindowTitle(const char *title) {}
int main(int argc, char **argv) {
  doomgeneric_Create(argc, argv);

  while (1) {
    doomgeneric_Tick();
  }
  return 0;
}
