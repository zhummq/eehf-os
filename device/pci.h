#ifndef _DECVICE_PCI_H
#define _DECVICE_PCI_H
#include "stdint.h"


#define PCI_ADDR 0xcf8
#define PCI_DATA 0xcfc
struct pci_bar_t {
  int8_t index;
  uint8_t type;
  uint32_t iobase;
  uint32_t size;
  uint32_t viobase;
};

struct pci_device {
uint16_t device_id;
uint16_t vender_id;
uint16_t status;
uint16_t command;
uint8_t calsscode;
uint8_t subclass;
uint8_t progIf;
uint8_t revisionId;
uint8_t bist;
uint8_t headerType;
uint8_t latencyTimer;
uint8_t cacheLineSize;
uint8_t bus;
uint8_t slot;
uint8_t func;
struct pci_bar_t bar[6];
};

struct pci_devices {
  int next;
  struct pci_device devices[100];
};
void pci_init(void);
struct pci_device * find_device(uint16_t vender_id,uint16_t device_id);
uint32_t pci_read_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void pci_write_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t data);
void pci_set_bars(struct pci_device * device);
void pci_enable_busmastering(struct pci_device * device);
uint8_t pci_interrupt(struct pci_device * device);
extern struct pci_devices device_arry;

#define PCI_BAR_TYPE_MEM 0
#define PCI_BAR_IO_MASK (~0x3)
#define PCI_BAR_MEM_MASK (~0xf)
#define PCI_BAR_TYPE_IO 1
#define PCI_COMMAND_MASTER      (1 << 2)
#endif
