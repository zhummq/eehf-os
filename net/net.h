#ifndef _NET_H
#define _NET_H
#include "stdint.h"
#include "list.h"
static inline uint16_t ntohs(uint16_t netshort){
  return ((netshort & 0xff00) >> 8) | ((netshort & 0x00ff) << 8);
}
static inline uint16_t htons(uint16_t hostshort){
  return ((hostshort & 0xff00) >> 8) | ((hostshort & 0x00ff) << 8);
}
struct netif_t{
  mac_addr mac;
  struct list rx_buff_list;
  struct list tx_buff_list;
};
struct desc_buff_t {
  struct list_elem node;
  uint16_t length;
  uint8_t payload[0];
}__attribute__((aligned(16)));
void net_packet_out(struct desc_buff_t * buff);
void net_packet_in(struct desc_buff_t * buff);
void net_init(void);
#endif // !_NET_H
