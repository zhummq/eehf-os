#include "dhcp.h"
#include "net.h"
#include "memory.h"
#include "string.h"

ip_addr requested_ip,server_ip; 
void dhcp_in(struct desc_buff_t * buff){
  struct eth_t * eth =  (struct eth_t *)buff->payload;
  struct ipv4_t * ipv4 = (struct ipv4_t *)eth->payload;
  struct udp_t * udp = (struct udp_t *)ipv4->payload;
  struct dhcp_t * dhcp = (struct dhcp_t *)udp->payload;
  uint8_t op = dhcp->op;
  switch(op){
    case DHCP_REPLY:
      struct dhcp_option_t * option = (struct dhcp_option_t *)dhcp->options;
      uint8_t type = *option->data;
      switch(type){
        case OPTIONS_ACK:
        break;
        case OPTIONS_OFFER:
          // get offer ip
        memcpy(requested_ip,dhcp->yiaddr,IPV4_LEN);
          // server ip
        
        struct desc_buff_t * buff_dhcp = (struct desc_buff_t *)sys_malloc(2048);
        //dhcp_out(buff_dhcp,socket,DHCP_REQUEST,xid,OPTIONS_REQUEST);
        break;
      }
    break;
    case DHCP_REQUEST:
    break;
  }

}
static uint16_t dhcp_discover(struct dhcp_option_t * option){
  struct dhcp_option_t * old_option = option;
  uint8_t * data = option->data;
  option->code = OPTIONS_MESSAGE_TYPE;
  option->length = 1;
  *data++ = OPTIONS_DISCOVER; 
  option = (struct dhcp_option_t *)data;
  data = option->data;
  option->code = PARA_REQUEST_LIST;
  option->length = 4;
  *data++ = SUBNET_MASK;
  *data++ = ROUTER;
  *data++ = DNS;
  *data++ = DOMAIN;
  *data++ = DHCP_OPTION_END;
  return data - old_option->data + 2;
}
static uint16_t dhcp_request(struct dhcp_option_t * option){
  struct dhcp_option_t * old_option = option;
  uint8_t *data  = option->data;
  option->code = OPTIONS_MESSAGE_TYPE;
  option->length = 1;
  *data++ = OPTIONS_REQUEST; 
  option = (struct dhcp_option_t *)data;
  data = option->data;
  option->code = REQUESTED_IP_ADDR; 
  option->length = 4;
  memcpy(data,requested_ip,IPV4_LEN);
  option = (struct dhcp_option_t *)data;
  data = option->data;
  option->code = SERVER_IP;
  option->length = 4;
  memcpy(data,server_ip,IPV4_LEN);
  option = (struct dhcp_option_t *)data;
  data = option->data;
  option->code = PARA_REQUEST_LIST;
  option->length = 4;
  *data++ = SUBNET_MASK;
  *data++ = ROUTER;
  *data++ = DNS;
  *data++ = DOMAIN;
  *data++ = DHCP_OPTION_END;
  return data - old_option->data +2;
}
void dhcp_out(struct desc_buff_t *buff,struct socket_t *socket,uint8_t op,uint32_t xid,uint8_t message_type){
  struct eth_t * eth =  (struct eth_t *)buff->payload;
  struct ipv4_t * ipv4 = (struct ipv4_t *)eth->payload;
  struct udp_t * udp = (struct udp_t *)ipv4->payload;
  struct dhcp_t * dhcp = (struct dhcp_t *)udp->payload;
  dhcp->op = op;
  dhcp->htype = 1;
  dhcp->hlen = 6;
  dhcp->hops = 0;
  dhcp->xid = xid;
  dhcp->secs = 0;
  dhcp->flags = htons(0x8000);
  memcpy(dhcp->ciaddr,"\x00\x00\x00\x00",IPV4_LEN);
  memcpy(dhcp->yiaddr,"\x00\x00\x00\x00",IPV4_LEN);
  memcpy(dhcp->siaddr,"\x00\x00\x00\x00",IPV4_LEN);
  memcpy(dhcp->giaddr,"\x00\x00\x00\x00",IPV4_LEN);
  memcpy(dhcp->chaddr,netifs[1].mac,MAC_LEN);
  memset(dhcp->sname,0,192);
  memcpy(dhcp->magic,"\x63\x82\x53\x63",4);
  uint16_t length = 0;
  length += sizeof(struct dhcp_t);
  struct dhcp_option_t * option = (struct dhcp_option_t *)dhcp->options; 
  switch(message_type){
    case OPTIONS_DISCOVER:
    length += dhcp_discover(option);
    break;
    case OPTIONS_REQUEST:
    length += dhcp_request(option);
    break;
  }
  buff->length += length;
  udp_out(buff,socket->src_port,socket->dst_port,length,socket->src_ip,socket->dst_ip);
  }
 
