#ifndef _NET_E1000_
#define _NET_E1000_
#include "pci.h"
#include "stdint.h"
#include "net.h"
#define INTEL_VEND     0x8086  // Vendor ID for Intel 
#define E1000_DEV      0x100E  //device ID for the e1000 Qemu, Bochs, and VirtualBox emmulated 
#define MMIOADDR 0xfeb80000   // Virtual addr for mmio
#define REG_CTRL 0x0000
#define CTRL_SLU (1<<6) // set link up
#define E1000_MAT0 0x5200
#define E1000_MAT1 0x5400 // multicast
#define E1000_IMS 0xd0
#define E1000_ICR 0xc0
#define E1000_RDBAL 0x2800
#define E1000_RDBAH 0x2804
#define E1000_RDLEN 0x2808
#define E1000_RDH 0x2810
#define E1000_RDT 0x2818



#define E1000_RCTL 0x100
#define RCTL_EN 1<<1
#define RCTL_UPE 1<<3
#define RCTL_LBM_NONE 0b00<<6
#define  RTCL_RDMTS_HALF 0b00<<8
#define RCTL_BAM 1<<15
#define RCTL_SECRC 1<<26
#define RCTL_BSIZE_2048 0<<16

#define E1000_NUM_RX_DESC 32
#define E1000_NUM_TX_DESC 8



#define E1000_TDBAL 0x3800
#define E1000_TDBAH 0x3804
#define E1000_TDLEN 0x3808
#define E1000_TDH 0x3810
#define E1000_TDT 0x3818

#define TCTL_EN 1<<1
#define TCTL_PSP 1<<3
#define TCTL_RTLC 1<<24
#define TCTL_CT 4
#define TCTL_COLD 12

#define E1000_TCTL 0x400

#define IMS_RXT0 1<<7
#define IMS_RXO 1<<6
#define IMS_RXDMT0 1<<4
#define IMS_RXSEQ 1<<3
#define IMS_LSC 1<<2
#define IMS_TXQE 1<<1
#define IMS_TXDW 1<<0
#define IMS_TXD_LOW 1<<15


#define TCMD_EOP 1<<0
#define TCMD_RPS 1<<4
#define TCMD_RS 1<<3
#define TCMD_IFCS 1<<1

struct rx_desc_t {
        volatile uint64_t addr;
        volatile uint16_t length;
        volatile uint16_t checksum;
        volatile uint8_t status;
        volatile uint8_t errors;
        volatile uint16_t special;
} __attribute__((packed));

struct tx_desc_t {
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

   struct rx_desc_t *rx;
  uint16_t rx_now;
	struct desc_buff_t **rx_buff;

	struct tx_desc_t *tx;
   uint16_t tx_now;
	struct desc_buff_t **tx_buff;

//task_t *tx_waiter;
};
extern struct e1000_t e1000;
void e1000_init(void);
void send_packet(struct desc_buff_t * buff);
#endif // DEBUG
