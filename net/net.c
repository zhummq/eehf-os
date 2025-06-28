#include "net.h"
#include "stdint.h"
#include "eth.h"
#include "arp.h"
#include "sync.h"
#include "thread.h"
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
  struct netif_t * netif = find_netif(eth->dst);
  if(netif){
    list_append(&netif->tx_buff_list,&buff->node);
    sema_up(&sema_out);
  } else{

    sys_free(buff);
  } 
  
}
static void netif_init(mac_addr mac){
  memcpy(netifs[1].mac,mac,MAC_LEN);
  list_init(&netifs[1].rx_buff_list);
  list_init(&netifs[0].rx_buff_list);
list_init(&netifs[1].tx_buff_list);
  list_init(&netifs[0].tx_buff_list);

}
static void test_arp(void){
  struct desc_buff_t * buff = sys_malloc(2048);
  arp_out(netifs[1].mac,"\xc0\xa8\x01\x02","\xFF\xFF\xFF\xFF\xFF\xFF","\xc0\xa8\x01\x02",buff,ARP_OP_REQUEST);
}
void net_init(void){
  netif_init(e1000.mac);
  sema_init(&sema_in,0);
  sema_init(&sema_out,0);
  thread_start("read_thread",20,read_thread,NULL);
  thread_start("write_thread",20,write_thread,NULL);
 // test_arp();
}


