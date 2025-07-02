#ifndef _ICMP_H_
#define _ICMP_H_
#include "stdint.h"
#include "net.h"

#define ICMP_TYPE_ECHO_REQUEST 8
#define ICMP_TYPE_ECHO_REPLY 0
struct icmp_t {
  uint8_t type;
  uint8_t code;
  uint16_t checksum;
  uint8_t payload[0];
};
void icmp_in(struct desc_buff_t * buff);
void icmp_out(struct desc_buff_t *buff,uint8_t type,uint8_t code,ip_addr src_ip,ip_addr dst_ip);

#endif
