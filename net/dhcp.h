#ifndef _DHCP_H_
#define _DHCP_H_
#include "stdint.h"
#include "net.h"
#include "eth.h"
#include "socket.h"
#include "ipv4.h"
#include "udp.h"
#define DHCP_OPTION_END 255
#define DHCP_REQUEST 1
#define DHCP_REPLY 2

#define OPTIONS_MESSAGE_TYPE 53
#define OPTIONS_DISCOVER 1
#define OPTIONS_OFFER 2
#define OPTIONS_REQUEST 3
#define OPTIONS_ACK 4
#define PARA_REQUEST_LIST 55
#define SUBNET_MASK 1
#define ROUTER 3
#define DNS 6
#define DOMAIN 15
#define REQUESTED_IP_ADDR 50
#define SERVER_IP 54

#define DHCP_CLIENT_PORT 68
#define DHCP_SERVER_PORT 67
struct dhcp_t {
  uint8_t op;
  uint8_t htype;
  uint8_t hlen;
  uint8_t hops;
  uint32_t xid;
  uint16_t secs;
  uint16_t flags;
  ip_addr ciaddr;
  ip_addr yiaddr;
  ip_addr siaddr;
  ip_addr giaddr;
  mac_addr chaddr;
  char pad [10];
  char sname[64];
  char file[128];
  uint8_t magic[4];
  uint8_t options[0];
};

struct dhcp_option_t {
  uint8_t code;
  uint8_t length;
  uint8_t data[0];
};
void dhcp_out(struct desc_buff_t *buff,struct socket_t *socket,uint8_t op,uint32_t xid,uint8_t message_type);
void dhcp_in(struct desc_buff_t * buff);
#endif
