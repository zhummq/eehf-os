#include "bga.h"
#include "pci.h"
#include "memory.h"
#include "psf2.h"
#include "string.h"
volatile uint32_t * fb;
uint32_t width = 1024;
static void set_vga_mode(uint16_t width,uint16_t height,uint16_t bpp,uint32_t iobase){
  iobase = iobase + 0x500;
  *(volatile uint16_t *)(iobase + (0<<1)) = 0xb0c5;
  *(volatile uint16_t *)(iobase + (1<<1)) = width;
  *(volatile uint16_t *)(iobase + (2<<1)) = height;
  *(volatile uint16_t *)(iobase + (3<<1)) = bpp;
  *(volatile uint16_t *)(iobase + (4<<1)) = 0x41;
}
void put_piexl(uint32_t x,uint32_t y,uint32_t color){
  fb[x *WIN_WIDTH  + y] = color;
}
void roll_screen(void)
{
  uint32_t dst = (uint32_t)fb;
  uint32_t x=0;
  for (x = 29; x<WIN_HEIGHT; x+=29)
  {
    memcpy((uint32_t*)(dst + (x-29)*WIN_WIDTH),(uint32_t*)(dst+ x*WIN_WIDTH),29*WIN_WIDTH*4);
  }
  memset((uint32_t*)(dst + x*WIN_WIDTH),0,29*WIN_WIDTH*4);
}
void cls_vga(void)
{
  uint32_t* dst = (uint32_t*)fb;
  memset(dst,0,WIN_WIDTH*WIN_HEIGHT*4);
}
void bga_init(void){
  struct pci_device * device = find_device(VGA_QEMU_VENDER_ID,VGA_QEMU_DEVICE_ID);
  pci_set_bars(device);
  map_area(device->bar[0].iobase,device->bar[0].iobase,device->bar[0].size);
  fb = (volatile uint32_t*)device->bar[0].iobase;
  map_area(device->bar[2].iobase,device->bar[2].iobase,device->bar[2].size);
  set_vga_mode(WIN_WIDTH,WIN_HEIGHT,32,device->bar[2].iobase);
}


