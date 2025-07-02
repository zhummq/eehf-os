#ifndef _NET_H
#define _NET_H
#include "stdint.h"
#include "list.h"
#define ARP_MAP_NUM 32
static inline uint16_t ntohs(uint16_t netshort){
  return ((netshort & 0xff00) >> 8) | ((netshort & 0x00ff) << 8);
}
static inline uint16_t htons(uint16_t hostshort){
  return ((hostshort & 0xff00) >> 8) | ((hostshort & 0x00ff) << 8);
}
struct arp_map_t{
  ip_addr ip;
  mac_addr mac;
  uint8_t used;
};
struct netif_t{
  mac_addr mac;
  ip_addr ip;
  struct list rx_buff_list;
  struct list tx_buff_list;
  struct arp_map_t arp_map[ARP_MAP_NUM];
};
struct desc_buff_t {
  struct list_elem node;
  uint16_t length;
  uint8_t payload[0];
}__attribute__((aligned(16)));
extern struct netif_t netifs[4];

uint16_t checksum(uint8_t *data,uint32_t length);
void set_arp_map(ip_addr ip,mac_addr mac);
char *get_mac(ip_addr ip);
void net_packet_out(struct desc_buff_t * buff);
void net_packet_in(struct desc_buff_t * buff);
void net_init(void);
void test_dhcp(void);
#endif // !_NET_H
