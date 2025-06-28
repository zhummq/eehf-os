#ifndef _ARP_H
#define _ARP_H
#include "net.h"
#include "e1000.h"
#include "stdint.h"

#define ARP_OP_REQUEST 0x0001
#define ARP_OP_REPLY 0x0002


struct arp_t {
  uint16_t hardware_type;
  uint16_t proto_type;
  uint8_t hw_len;
  uint8_t proto_len;
  uint16_t op;
  mac_addr src_mac;
  ip_addr src_ip;
  mac_addr dst_mac;
  ip_addr dst_ip;
};
void arp_in(struct desc_buff_t * buff);
void arp_out(mac_addr src_mac,ip_addr src_ip,mac_addr dst_mac,ip_addr dst_ip,struct desc_buff_t * buff ,uint16_t op);
#endif
