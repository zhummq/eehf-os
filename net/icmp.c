#include "icmp.h"
#include "eth.h"
#include "net.h"
#include "ipv4.h"
#include "string.h"
#include "memory.h"
#include "timer.h"

void icmp_in(struct desc_buff_t * buff){
  struct eth_t * eth = (struct eth_t *)buff->payload;
  struct ipv4_t * ipv4 = (struct ipv4_t *)eth->payload;
  struct icmp_t * icmp = (struct icmp_t *)ipv4->payload;
  switch(icmp->type){
    case ICMP_TYPE_ECHO_REQUEST:
    struct desc_buff_t * buff_reply = (struct desc_buff_t *)sys_malloc(2048);
    uint32_t length_struct = sizeof(struct eth_t) + sizeof(struct ipv4_t) +sizeof(struct icmp_t);
    uint32_t length = buff->length - length_struct;
    memcpy((uint8_t *)((uint8_t *)buff_reply->payload + length_struct),(uint8_t *)icmp->payload,length);
    buff_reply->length = length;
    icmp_out(buff_reply,ICMP_TYPE_ECHO_REPLY,0,ipv4->dst_ip,ipv4->src_ip);
    break;
    case ICMP_TYPE_ECHO_REPLY:
    break;
  }
}

void icmp_out(struct desc_buff_t *buff,uint8_t type,uint8_t code,ip_addr src_ip,ip_addr dst_ip){
  struct eth_t * eth = (struct eth_t *)buff->payload;
  struct ipv4_t * ipv4 = (struct ipv4_t *)eth->payload;
  struct icmp_t * icmp = (struct icmp_t *)ipv4->payload;
  buff->length += sizeof(struct icmp_t);
  icmp->type = type;
  icmp->code = code;
  icmp->checksum = checksum((uint8_t *) icmp,buff->length);
ipv4_out(buff,buff->length,ticks,0,64,IP_TYPE_ICMP,src_ip,dst_ip);
}
