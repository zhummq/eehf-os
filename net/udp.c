#include "udp.h"
#include "ipv4.h"
#include "net.h"
#include "timer.h"
#include "eth.h"
#include "memory.h"
#include "dhcp.h"

void udp_in(struct desc_buff_t *buff){
  struct eth_t * eth =  (struct eth_t *)buff->payload;
  struct ipv4_t * ipv4 = (struct ipv4_t *)eth->payload;
  struct udp_t * udp = (struct udp_t *)ipv4->payload;
  uint16_t server_port = ntohs(udp->src_port);
  uint16_t client_port = ntohs(udp->dst_port);
  if (server_port == 67 && client_port == 68){
    dhcp_in(buff);
  } else {
    sys_free(buff);
  }
}


void udp_out(struct desc_buff_t * buff,uint16_t src_port,uint16_t dst_port,uint16_t length,ip_addr src_ip,ip_addr dst_ip){
  struct eth_t * eth =  (struct eth_t *)buff->payload;
  struct ipv4_t * ipv4 = (struct ipv4_t *)eth->payload;
  struct udp_t * udp = (struct udp_t *)ipv4->payload;
  buff->length += sizeof(struct udp_t);
  udp->src_port = htons(src_port);
  udp->dst_port = htons(dst_port);

  length += sizeof(struct udp_t);
  udp->length = htons(length);
 
// need socket ip,...
  ipv4_out(buff,length,ticks,0x0,64,IP_TYPE_UDP,src_ip,dst_ip);
}
