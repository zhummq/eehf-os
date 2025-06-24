#include "e1000.h"
#include "interrupt.h"
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
}
static void rx_init(void){
  uint32_t base = e1000.dev->bar[0].viobase;

 e1000.rx = (struct rx_desc_t *)sys_malloc(sizeof(struct rx_desc_t) * E1000_NUM_RX_DESC );

  e1000.rx_now = 0;
  *(volatile uint32_t *)(base + E1000_RDBAL) = addr_v2p((uint32_t)e1000.rx); 
  *(volatile uint32_t *)(base + E1000_RDBAH) = 0;
  *(volatile uint32_t *)(base + E1000_RDLEN) = sizeof(struct rx_desc_t) * E1000_NUM_RX_DESC;

  *(volatile uint32_t *)(base + E1000_RDH) = 0;
  *(volatile uint32_t *)(base + E1000_RDT) = E1000_NUM_RX_DESC - 1;

  for (int i = 0; i < E1000_NUM_RX_DESC; i++){
    e1000.rx[i].status = 0;
    e1000.rx[i].addr = (uint32_t)addr_v2p((uint32_t)sys_malloc(2048));
  }

  uint32_t value = RCTL_EN | RCTL_LBM_NONE | RTCL_RDMTS_HALF | RCTL_BAM | RCTL_SECRC | RCTL_BSIZE_2048;
  *(volatile uint32_t *)(base + E1000_RCTL) = value;
}
static void tx_init(void){
  uint32_t base = e1000.dev->bar[0].viobase;

  e1000.tx = (struct tx_desc_t *)sys_malloc(sizeof(struct tx_desc_t) * E1000_NUM_TX_DESC );

  e1000.tx_now = 0;
  *(volatile uint32_t *)(base + E1000_TDBAL) = addr_v2p((uint32_t)e1000.tx); 
  *(volatile uint32_t *)(base + E1000_TDBAH) = 0;
  *(volatile uint32_t *)(base + E1000_TDLEN) = sizeof(struct tx_desc_t) * E1000_NUM_TX_DESC;

  *(volatile uint32_t *)(base + E1000_TDH) = 0;
  *(volatile uint32_t *)(base + E1000_TDT) = E1000_NUM_TX_DESC - 1;

  for (int i = 0; i < E1000_NUM_TX_DESC; i++){
    e1000.rx[i].status = 1<<0;
    e1000.rx[i].addr =0; 
  }
  uint32_t value = TCTL_EN | TCTL_PSP | TCTL_RTLC | (0x10 << TCTL_CT) | (0x40 << TCTL_COLD);
  *(volatile uint32_t *)(base + E1000_TCTL) = value;
} 

static void e1000_reset(void){
  uint32_t base = e1000.dev->bar[0].viobase;

  e1000_read_mac();

  // start link

  *(volatile uint32_t *)(base + REG_CTRL) = *(volatile uint32_t *)(base + REG_CTRL) | CTRL_SLU;
  // multicast
  for (int i = E1000_MAT0; i < E1000_MAT1; i+=4){
    *(volatile uint32_t *)(base + i) = 0;
  }
  // stop interrupt
  *(volatile uint32_t *)(base + E1000_IMS) = 0;
  rx_init();
  tx_init();
  int value = IMS_RXT0 | IMS_RXO | IMS_RXDMT0 | IMS_RXSEQ | IMS_LSC | IMS_TXQE | IMS_TXDW | IMS_TXD_LOW;

  *(volatile uint32_t *)(base + E1000_IMS) = value;
  

}
static void e1000_handler_irq(void){

  printk("777777\n");
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

e1000_reset();
  
  uint8_t e1000_irq = pci_interrupt(e1000.dev) + 0x20;
register_handler(e1000_irq, e1000_handler_irq);



  

}
