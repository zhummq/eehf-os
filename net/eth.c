#include "stdint.h"
#include "net.h"
#include "eth.h"
#include "arp.h"
#include "string.h"

void eth_in(struct desc_buff_t * buff){
  struct eth_t * eth = (struct eth_t *)buff->payload;
  switch (ntohs(eth->type)){
    case ETH_TYPE_ARP:
    arp_in(buff);
    break;
    case ETH_TYPE_IPV4:
    ipv4_in(buff);
    break;
    case ETH_TYPE_TEST:
    break;
  }
}

void eth_out(mac_addr mac_dst,mac_addr mac_src,uint16_t type,struct desc_buff_t * buff){
  struct eth_t * eth  = (struct eth_t *)buff->payload;
  memcpy(eth->dst,mac_dst,MAC_LEN);
  memcpy(eth->src,mac_src,MAC_LEN);
  eth->type = htons(type);
  buff->length += sizeof(struct eth_t);
  net_packet_out(buff);
}
