#include "pci.h"
#include "global.h"
#include "stdint.h"
#include "io.h"
struct pci_devices device_arry;
static uint32_t pci_addr(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset){
  uint32_t address;
  uint32_t lbus = (uint32_t)bus;
  uint32_t lslot = (uint32_t)slot;
  uint32_t lfunc = (uint32_t)func;
  uint32_t loffset  = (uint32_t)offset;


  address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (loffset & 0xFC) | ((uint32_t)0x80000000));

  return address;
}


uint32_t pci_read_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset){
    uint32_t address = pci_addr(bus,slot,func,offset); 
    outl(PCI_ADDR, address);

  return inl(PCI_DATA);
}
uint8_t pci_interrupt(struct pci_device * device){
  uint32_t data = pci_read_config(device->bus,device->slot,device->func,0x3c);
  return data & 0xff;
}

static void pci_scan(void){
  int bus, slot, func = 0;
  for (bus = 0; bus < 256; bus++){
    for (slot = 0; slot < 32; slot++){
      for (func = 0; func <8; func++){
      uint32_t value = pci_read_config(bus,slot,func,0);
      uint32_t venderId = value & 0x0000ffff;
      if (venderId != 0x0000ffff){
        // invalid value
        int next = device_arry.next;
        struct pci_device * device = &device_arry.devices[next];
        device_arry.next++;

        device->device_id = value >> 16;
        device->vender_id = venderId;
        device->bus = bus;
        device->slot = slot;
        device->func = func;


      }
      }
    }
  }
}


void pci_write_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t data){
  uint32_t address = pci_addr(bus,slot,func,offset);
  outl(PCI_ADDR, address);
  outl(PCI_DATA, data);
}
struct pci_device * find_device(uint16_t vender_id,uint16_t device_id){
  int end = device_arry.next;
  int i = 0;
  for (i=0; i<end; i++){
    struct pci_device * device = &device_arry.devices[i];
    if (vender_id == device->vender_id && device_id == device->device_id){
      return device;
    }
  }
  return NULL;
}


void pci_enable_busmastering(struct pci_device * device){
  uint32_t data = pci_read_config(device->bus,device->slot,device->func,0x4);
  data |= PCI_COMMAND_MASTER;
  pci_write_config(device->bus,device->slot,device->func,0x4,data);
}
static uint32_t pci_size(uint32_t base, uint32_t mask){
   uint32_t size = base & mask;
   return ~size + 1;
}


void pci_set_bars(struct pci_device * device){
  for (int i = 0; i<6; i++){
    struct pci_bar_t * bar = &device->bar[i];
    bar->index = -1;
    uint8_t addr = 0x10 + (i << 2);
    uint32_t value = pci_read_config(device->bus,device->slot,device->func,addr);
    pci_write_config(device->bus,device->slot,device->func,addr,-1);
    uint32_t len = pci_read_config(device->bus,device->slot,device->func,addr);
    pci_write_config(device->bus,device->slot,device->func,addr,value);


        if (value == 0)
            continue;
        if (len == 0)
            continue;
    if(value & 1){
      bar->iobase = value & PCI_BAR_IO_MASK;
      bar->index = i;
      bar->type = PCI_BAR_TYPE_IO;
      bar->size = pci_size(len,PCI_BAR_IO_MASK);
    } else {
      bar->iobase = value & PCI_BAR_MEM_MASK;
      bar->index = i;
      bar->type = PCI_BAR_TYPE_MEM;
      bar->size = pci_size(len,PCI_BAR_MEM_MASK);
    }

  }
}


void pci_init(void){
pci_scan();
}
