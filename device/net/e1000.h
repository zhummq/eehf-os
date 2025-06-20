#ifndef _NET_E1000_
#define _NET_E1000_
#include "pci.h"
#include "stdint.h"
#define INTEL_VEND     0x8086  // Vendor ID for Intel 
#define E1000_DEV      0x100E  //device ID for the e1000 Qemu, Bochs, and VirtualBox emmulated 



#define E1000_NUM_RX_DESC 32
#define E1000_NUM_TX_DESC 8

struct e1000_rx_desc {
        volatile uint64_t addr;
        volatile uint16_t length;
        volatile uint16_t checksum;
        volatile uint8_t status;
        volatile uint8_t errors;
        volatile uint16_t special;
} __attribute__((packed));

struct e1000_tx_desc {
        volatile uint64_t addr;
        volatile uint16_t length;
        volatile uint8_t cso;
        volatile uint8_t cmd;
        volatile uint8_t status;
        volatile uint8_t css;
        volatile uint16_t special;
} __attribute__((packed));
struct e1000_t {
    char name[256];
    uint32_t base;
    mac_addr mac;
    struct pci_device *dev;
    uint8_t eeprom;

//    rx_desc_t *rx;
  //  uint16_t rx_now;
	//desc_buff_t **rx_buff;

	//tx_desc_t *tx;
   // uint16_t tx_now;
	//desc_buff_t **tx_buff;

//task_t *tx_waiter;
};

void e1000_init(void);

#endif // DEBUG
