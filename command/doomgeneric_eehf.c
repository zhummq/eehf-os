#include "stdio.h"
#include "timer.h"
#include "vga.h"

void DG_Init(void){

}


void DG_DrawFrame(void){
for (uint32_t x = 0; x < DOOMGENERIC_RESX; x++){
    for (uint32_t y = 0; y < DOOMGENERIC_RESY; y++){
      put_piexl(x,y,DG_ScreenBuffer[x * DOOMGENERIC_RESX + y]);
    }
  }
}
void DG_SleepMs(uint32_t ms){
  mtime_sleep(ms);
}
uint32_t DG_GetTicksMs(){
  return get_ms();
}
int DG_GetKey(int* pressed, unsigned char* key){
  
}
void DG_SetWindowTitle(const char * title){

}
