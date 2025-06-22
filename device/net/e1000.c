#include "e1000.h"
#include "memory.h"
#include "pci.h"
#include "global.h"
#include "stdio-kernel.h"
struct e1000_t e1000;
static void e1000_eeprom_detect(void){
  volatile uint32_t * base = ( volatile uint32_t *)(e1000.dev->bar[0].viobase + 0x14);
  *base = 0x1;
  for (int i = 0;i < 1000 && !e1000.eeprom; i++){
    uint32_t value = *(base);
    if(value & 0x10){
      e1000.eeprom = 1;
    } else {
      e1000.eeprom = 0;
    }
  }
}


static uint16_t e1000_eeprom_read(uint8_t addr){
  volatile uint32_t * base =(volatile uint32_t *) (e1000.dev->bar[0].viobase + 0x14);
  uint32_t tmp;
  if (e1000.eeprom){
    *base =  1 | (uint32_t)addr << 8;
    while (!((tmp = *base) & (1<<4)));
  } else {
    *base = 1 | (uint32_t)addr << 2;
    while (!((tmp = *base) & (1<<1)));

  }
  return (tmp >> 16) & 0xffff;
}


static void e1000_read_mac(void){
  uint16_t val;
  e1000_eeprom_detect();
  if(e1000.eeprom){
    val = e1000_eeprom_read(0);
    e1000.mac[0] = val & 0xff;
    e1000.mac[1] = val >> 8;
     val = e1000_eeprom_read(1);
    e1000.mac[2] = val & 0xff;
    e1000.mac[3] = val >> 8;
     val = e1000_eeprom_read(2);
    e1000.mac[4] = val & 0xff;
    e1000.mac[5] = val >> 8;
  } else{
     char *mac = (char *)e1000.dev->bar[0].iobase + 0x5400;
        for (int i = 5; i >= 0; i--)
        {
            e1000.mac[i] = mac[i];
        }
  }
  while(1);
}
void e1000_init(void){
  
  struct pci_device* device = find_device(INTEL_VEND,E1000_DEV);
  if (!device){
    printk("66666\n");
    while(1);
  }
  e1000.dev = device;
  pci_set_bars(device);
  struct pci_bar_t * bar = &e1000.dev->bar[0];
  if (bar->index < 0){
    printk("66666\n");
    while(1);
  }
  map_area(MMIOADDR ,bar->iobase,bar->size); 
  bar->viobase = MMIOADDR;

  pci_enable_busmastering(e1000.dev);

  e1000_read_mac();
}
