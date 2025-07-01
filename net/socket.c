#include "socket.h"
#include "memory.h"
#include "string.h"
#include "file.h"


int32_t socket_create(ip_addr src_ip,uint16_t src_port,ip_addr dst_ip,uint16_t dst_port){
  struct socket_t * socket = (struct socket_t *)sys_malloc(sizeof(struct socket_t));
  memcpy(socket->src_ip,src_ip,IPV4_LEN);
  socket->src_port = src_port;
  memcpy(socket->dst_ip,dst_ip,IPV4_LEN);
  socket->dst_port = dst_port;
  int32_t global_fd = get_free_slot_in_global();
  file_table[global_fd].fd_pos = (uint32_t)socket;
  return pcb_fd_install(global_fd);
}
