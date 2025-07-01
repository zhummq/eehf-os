#ifndef _SOCKET_H_
#define _SOCKET_H_
#include "stdint.h"

struct socket_t {
  ip_addr src_ip;
  uint16_t src_port;
  ip_addr dst_ip;
  uint16_t dst_port;
};
int32_t socket_create(ip_addr src_ip,uint16_t src_port,ip_addr dst_ip,uint16_t dst_port);
#endif
