#ifndef _ETH_H
#define _ETH_H
#include "stdint.h"
#define ETH_TYPE_ARP 0x0806
#define ETH_TYPE_TEST 0x9000
#define ETH_TYPE_IPV4 0x0800
struct eth_t {
  mac_addr dst;
  mac_addr src;
  uint16_t type;
  uint8_t payload[0];
};
void eth_in(struct desc_buff_t * buff);
void eth_out(mac_addr mac_dst,mac_addr mac_src,uint16_t type,struct desc_buff_t * buff);
#endif // !_ETH_H
