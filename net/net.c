#include "net.h"
#include "stdint.h"
#include "eth.h"
#include "arp.h"
#include "sync.h"
#include "thread.h"
#include "ipv4.h"
#include "timer.h"
#include "dhcp.h"
#include "socket.h"
#include "udp.h"
#include "string.h"
#include "e1000.h"
struct netif_t netifs[4];
struct semaphore sema_in,sema_out;
static void read_thread(void * arg UNUSED){
  while(1){
    int count = 0;
    for (int i =0;i<2;i++){
      struct netif_t * netif = &netifs[i];
      if (list_empty(&netif->rx_buff_list)){
        continue;
      } else {
        struct list_elem * nod =list_pop(&netif->rx_buff_list);
        struct desc_buff_t * buff = elem2entry(struct desc_buff_t,node,nod);
        eth_in(buff);
        count++;
      }
    }
    if(count == 0){
      sema_down(&sema_in);//?????
    }
  }
}
static void write_thread(void * arg UNUSED){
  while(1){
    int count = 0;
    for (int i=0;i<2;i++){
      struct netif_t * netif = &netifs[i];
      if(list_empty(&netif->tx_buff_list)){
        continue;
      } else {
        struct list_elem * nod = list_pop(&netif->tx_buff_list);
        struct desc_buff_t * buf = elem2entry(struct desc_buff_t,node,nod);
        send_packet(buf);
      }
      count ++;
    }
    if(count == 0){
      sema_down(&sema_out);
    }
  }
}
static struct netif_t * find_netif(mac_addr mac){
  if (!memcmp(mac,"\xFF\xFF\xFF\xFF\xFF\xFF",MAC_LEN)){
    return &netifs[1];
  }
  for(int i = 0; i < 4;i++){
    if (!memcmp(netifs[i].mac,mac,MAC_LEN)){
      return &netifs[i];
    }
  }
  return NULL;
}
void net_packet_in(struct desc_buff_t * buff){
  struct eth_t * eth = (struct eth_t *)buff->payload;
  struct netif_t * netif = find_netif(eth->dst);
  if (netif){
  list_append(&netif->rx_buff_list,&buff->node);
    sema_up(&sema_in);
  } else {
   sys_free(buff); 
  }
}
void net_packet_out(struct desc_buff_t * buff){
  struct eth_t * eth = (struct eth_t *)buff->payload;
  struct netif_t * netif = find_netif(eth->src);
  if(netif){
    list_append(&netif->tx_buff_list,&buff->node);
    sema_up(&sema_out);
  } else{

    sys_free(buff);
  } 
}
static void netif_init(mac_addr mac){
  memcpy(netifs[1].mac,mac,MAC_LEN);
  memcpy(netifs[1].ip,"\xc0\xa8\x0b\x02",IPV4_LEN);
  set_arp_map("\xc0\xa8\x0b\x01","\xce\x77\x25\x98\x52\x7b");
  list_init(&netifs[1].rx_buff_list);
  list_init(&netifs[0].rx_buff_list);
  list_init(&netifs[1].tx_buff_list);
  list_init(&netifs[0].tx_buff_list);

}
void test_dhcp(void){
  struct desc_buff_t * buff = sys_malloc(2048);

  arp_out(netifs[1].mac,"\xc0\xa8\x0b\x02","\xce\x77\x25\x98\x52\x7b","\xc0\xa8\x0b\x84",buff,ARP_OP_REPLY);
 
 struct socket_t * socket =(struct socket_t * )sys_malloc(sizeof(struct socket_t)); 
 socket->src_port = 68;
 socket->dst_port = 67;
 ip_addr src_ip= {'\x0','\x0','\x0','\x0'};
 ip_addr dst_ip = {'\xff','\xff','\xff','\xff'};
 set_arp_map(dst_ip,"\xFF\xFF\xFF\xFF\xFF\xFF");
 memcpy(socket->src_ip,src_ip,IPV4_LEN);
 memcpy(socket->dst_ip,dst_ip,IPV4_LEN);
// dhcp_out(buff,socket,DHCP_REQUEST,ticks,OPTIONS_DISCOVER);
}
void set_arp_map(ip_addr ip,mac_addr mac){
  struct netif_t *netif = &netifs[1];
  for (int i = 0;i < ARP_MAP_NUM;i++){
    if (!netif->arp_map[i].used){
      memcpy(netif->arp_map[i].ip,ip,IPV4_LEN);
      memcpy(netif->arp_map[i].mac,mac,MAC_LEN);
      netif->arp_map[i].used = 1;
      break;
    }
  }

}
char * get_mac(ip_addr ip){
  struct netif_t* netif = &netifs[1];
  for (int i = 0;i < ARP_MAP_NUM;i++){
    if (netif->arp_map[i].used && !memcmp(netif->arp_map[i].ip,ip,IPV4_LEN)){
      return netif->arp_map[i].mac;
    }
  }
  return NULL;
}
uint16_t checksum(uint8_t *data,uint32_t length){
 uint32_t checksum = 0;

    for (uint32_t i = 0; i < length; i += 2) {
        uint16_t word = data[i] << 8;
        if (i + 1 < length) {
            word |= data[i + 1];
        }
        checksum += word;
    }

    // 不断将高 16 位加到低 16 位
    while (checksum >> 16) {
        checksum = (checksum & 0xFFFF) + (checksum >> 16);
    }

    return htons((uint16_t)(~checksum));
}
void net_init(void){
  netif_init(e1000.mac);
  sema_init(&sema_in,0);
  sema_init(&sema_out,0);
  thread_start("read_thread",31,read_thread,NULL);
  thread_start("write_thread",31,write_thread,NULL);
  
}


