#include "arp.h"
#include "string.h"
#include "eth.h"
#include "net.h"
void arp_in(struct desc_buff_t * buff){
  struct eth_t *eth = (struct eth_t *)buff->payload;
  struct arp_t *arp = (struct arp_t *)eth->payload;
  switch (ntohs(arp->op)) {
    case ARP_OP_REQUEST:
      struct desc_buff_t * buff_reply = (struct desc_buff_t *)sys_malloc(2048);
      arp_out(netifs[1].mac,netifs[1].ip,arp->src_mac,arp->src_ip,buff_reply,ARP_OP_REPLY);
      sys_free(buff);// broast
    break;
      // map for ip,mac
    case ARP_OP_REPLY:
      set_arp_map(arp->src_ip,arp->src_mac);
      sys_free(buff);
    break;
  }
}




void arp_out(mac_addr src_mac,ip_addr src_ip,mac_addr dst_mac,ip_addr dst_ip,struct desc_buff_t * buff ,uint16_t op){
  struct eth_t *eth = (struct eth_t *)buff->payload;
  struct arp_t *arp = (struct arp_t *)eth->payload;
  buff->length += sizeof(struct arp_t);
  arp->hardware_type = htons(1);
  arp->proto_type = htons(ETH_TYPE_IPV4);
  arp->hw_len = 6;
  arp->proto_len = 4;
  arp->op = htons(op);
  memcpy(arp->src_mac,src_mac,MAC_LEN);
  memcpy(arp->src_ip,src_ip,IPV4_LEN);
  memcpy(arp->dst_mac,dst_mac,MAC_LEN);
  memcpy(arp->dst_ip,dst_ip,IPV4_LEN);
  eth_out(dst_mac,src_mac,ETH_TYPE_ARP,buff);
}
