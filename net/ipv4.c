#include "ipv4.h"
#include "icmp.h"
#include "udp.h"
#include "net.h"
#include "eth.h"

void ipv4_in(struct desc_buff_t * buff){
  struct eth_t * eth = (struct eth_t *)buff->payload;
  struct ipv4_t * ipv4 = (struct ipv4_t *)eth->payload;
  switch(ipv4->proto){
    case IP_TYPE_ICMP:
    icmp_in(buff);
    break;
    case IP_TYPE_TCP:
    break;
    case IP_TYPE_UDP:
    udp_in(buff);
    break;
  }
}

void ipv4_out(struct desc_buff_t * buff,uint16_t total_len,uint16_t id,uint16_t flag_offset,uint8_t ttl,uint8_t proto,ip_addr src_ip,ip_addr dst_ip){
  struct eth_t * eth = (struct eth_t *)buff->payload;
  struct ipv4_t * ipv4 = (struct ipv4_t *)eth->payload;
  buff->length += sizeof(struct ipv4_t);
  ipv4->ver_ihl = 0x45;
  ipv4->tos = 0;
  ipv4->total_len = htons(total_len + 20);
  ipv4->id = htons(id);
  ipv4->flag_offset = htons(flag_offset);
  ipv4->ttl = ttl;
  ipv4->proto = proto;
  memcpy(ipv4->src_ip,src_ip,IPV4_LEN);
  memcpy(ipv4->dst_ip,dst_ip,IPV4_LEN);
  ipv4->checksum = checksum((uint8_t *)ipv4,sizeof(struct ipv4_t));
  eth_out(get_mac(dst_ip),netifs[1].mac,ETH_TYPE_IPV4,buff);
}
