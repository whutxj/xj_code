

#ifndef ETH_H_H
#define ETH_H_H

#include "main.h"
#include "cmsis_os.h"
#include "Global.h"
#include "lwip/sockets.h"
int eth_create_socket(int type);

int eth_connect_server(int fd,char *ip,int port);

int eth_write(int fd,char *buf,int len,int time_out);
int eth_read(int fd,char *buf,int len,int time_out);

void eth_dbg_init(uint32_t dbg_port);
int eth_udp_dbg(uint8_t *buf,int len);
void Setup_JWT_background_App(osPriority ThreadPriority,uint8_t StackNum);
#endif


