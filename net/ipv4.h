#ifndef _IPV4_H_
#define _IPV4_H_
#include "stdint.h"
#include "net.h"
#include "string.h"
#define IP_TYPE_ICMP 1
#define IP_TYPE_TCP 6
#define IP_TYPE_UDP 17
struct ipv4_t {
  uint8_t ver_ihl;
  uint8_t tos;
  uint16_t total_len;
  uint16_t id;
  uint16_t flag_offset;
  uint8_t ttl;
  uint8_t proto;
  uint16_t checksum;
  ip_addr src_ip;
  ip_addr dst_ip;
  uint8_t payload[0];
};
void ipv4_in(struct desc_buff_t * buff);
void ipv4_out(struct desc_buff_t * buff,uint16_t total_len,uint16_t id,uint16_t flag_offset,uint8_t ttl,uint8_t proto,ip_addr src_ip,ip_addr dst_ip);

#endif
