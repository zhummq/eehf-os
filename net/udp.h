#ifndef _UDP_H_
#define _UDP_H_
#include "stdint.h"
#include "net.h"

struct udp_t {
  uint16_t src_port;
  uint16_t dst_port;
  uint16_t length;
  uint16_t checksum;
  uint8_t payload[0];
};
void udp_in(struct desc_buff_t *buff);
void udp_out(struct desc_buff_t * buff,uint16_t src_port,uint16_t dst_port,uint16_t length,ip_addr src_ip,ip_addr dst_ip);
#endif

