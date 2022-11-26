
#include "lwip/sockets.h"
#include "eth_comm.h"
#include "string.h"
#include "lwip.h"
#include "lwip/sockets.h"
#include <stdio.h>
#include "lwip/netdb.h"
#include <string.h>
#include "lwip/api.h"
#include "Utility.h"
#include "Tsk_JWT.h"
#include "cmsis_os.h"
#include "MsgQueue.h"
#include "BSP_LED_Function.h"
#include "Tsk_Charge_Ctrl.h"

#define D_TCP			0
#define D_UDP			1
#define link_way  0

int eth_create_socket(int type)
{
	int fd = 0;
    uint8 block = 0;
	if(type == D_TCP)
	{
		fd = socket(AF_INET, SOCK_STREAM, 0);
	}
	else
	{
		fd = socket(AF_INET, SOCK_DGRAM, 0);
	}
    ioctlsocket(fd,FIONBIO,&block);
	return fd;
}


int eth_connect_server(int fd,char *ip,int port)
{
    struct sockaddr_in s_add;
    struct hostent *pURL = NULL;
    
  memset(&s_add,0,sizeof(struct sockaddr_in));
  s_add.sin_family=AF_INET;
  s_add.sin_addr.s_addr= inet_addr(ip);
  s_add.sin_port=htons(port);

//    memset(&s_add,0,sizeof(s_add)); 
//    pURL = gethostbyname(ip);
//    if(pURL ==NULL)
//    {
//        dbg_printf(1,"connect gethostbyname fail !\r\n");
//        return 0;      
//    }
//    s_add.sin_family=AF_INET;
//    s_add.sin_addr.s_addr = *((unsigned long*) pURL->h_addr_list[0]);
//    s_add.sin_port=htons(port);
    
    dbg_printf(1,"eth connct ip %s port %d",ip,port);

    if(-1 == connect(fd,(struct sockaddr *)(&s_add), sizeof(struct sockaddr)))
    {
        dbg_printf(1,"connect fail !\r\n");
        return 0;
    }
    else
    {
        dbg_printf(1,"connect ok");
        return 1;
    }

}


int eth_write(int fd,char *buf,int len,int time_out)
{
	fd_set wfd;
	struct timeval tv;
	int nfd = 0;

	tv.tv_usec = 00;
	tv.tv_sec = 1;

//  FD_ZERO(&wfd);
//	FD_SET(fd,&wfd);

//  	nfd = select(fd + 1, NULL, &wfd, NULL, &tv);

//	if(nfd < 0)
//	{
//		return -1;
//	}
//  else if(nfd == 0)
//	{
//	return 0;
//	}
//	else
//  {
//		nfd = write(fd,buf,len);
//  }
    nfd = lwip_send(fd,buf,len,MSG_DONTWAIT);
	return nfd;
}

/*��ȷ����*/
int eth_read(int fd,char *buf,int len,int time_out)
{
    fd_set rfd;
    struct timeval tv;
	int nfd = 0;
//
    tv.tv_usec = 00;
    tv.tv_sec = 1;

//    FD_ZERO(&rfd);
//  	FD_SET(fd,&rfd);
////
//  	nfd = select(fd + 1,&rfd, NULL,  NULL, &tv);

//  	if(nfd < 0)
//  	{
//  		return -1;
//		}
//  	else if(nfd == 0)
//  	{
//  		return 0;
//  	}
//	else
	{
		nfd = read(fd,buf,len);
	}
	return nfd;
}

u8_t eth_daemon_process(u8_t rx_cmd)
{
  u8_t online_state;

  if (online_state == 0)
  {

  }
  else
  {

  }

  return online_state;
}
/*

	健网科技 -- JWT

					*/

void eth_test_code(void)
{
	static uint32 tick[3] = {0};
  struct netconn *conn;
  struct netbuf  *TCPNetbuf;
  err_t  myerr;
	uint8  rx_cmd = 0;
	uint32  offline_check;
  u8_t   online_state = 0;
  u8_t   log_in_times = 0;
	
  uint32* Text;
  uint8*  data;
	
  static ip_addr_t serverip;       //目标机IP
  static unsigned short serverport;       //目标机端口号
	co_dword dword;
	
  serverip.addr = htonl(0x784F06E9); //htonl(0x784CD748);//htonl(0xC0A8016A);  //120.79.6.233   //192.168.1.106 
  serverport = Get_Server_Port();//8015;//5008;
  conn = netconn_new(NETCONN_TCP);      /* 创建TCP连接  */
  netconn_bind(conn,NULL,9000);           /* 绑定本地地址和监听的端口号 */

  myerr = netconn_connect(conn,&serverip,serverport);      //连接主机 cnn，目标机IP，端口号
  if (myerr == -1)
  {
    dbg_printf(1," TCP connected err ");
  }
  else
  {
    dbg_printf(1," TCP connected ok ");
  }

	TCPNetbuf =netbuf_new();
  while(1)
  {
		uint8 	 Ch = 0;
		uint8    MsgFlag = 0;
    MSG_Def  Msg;

    MsgFlag = MsgQueue_Get(MSG_QUEUE_TASK_CLOUD_COMM, &Msg);
    if (MsgFlag)
    {
      Ch = Msg.Ch;
    }
		
    if (online_state == 0)
    {
      if (log_in_times <= 4)
      {
				if(Tick_Out(&tick[0],10000))
        {            
         	Tick_Out(&tick[0],0);
					log_in_times++;
        	Text = (uint32*)JWT_tx_0x01_login(data);
        	netbuf_ref(TCPNetbuf, Text, 37);
        	myerr = netconn_write(conn, Text, 37, NETCONN_NOCOPY);
					if (myerr != ERR_OK)
					{
						log_in_times = 5;
					}
				}
      }
      else
      {
        myerr = netconn_delete(conn);
        if (myerr == ERR_OK)
        {
          dbg_printf(1," TCP close ok ");
        }
        else
        {
          log_in_times = 0;
          dbg_printf(1," TCP close err ");
        }
        osDelay(100);

        conn = netconn_new(NETCONN_TCP);
        netconn_bind(conn,NULL,6666);
        myerr = netconn_connect(conn,&serverip,serverport);
        if (myerr == -1)
        {
          dbg_printf(1," TCP connected err ");
        }
        else
        {
          log_in_times = 0;
          dbg_printf(1," TCP connected ok ");
        }
      }

      netconn_listen(conn);
      if (netconn_recv(conn, &TCPNetbuf) == ERR_OK)
      {
        dbg_printf(1," offline RX DATA ");
				if (JWT_rx_handler(TCPNetbuf->p->payload) == 0x81)
				{
					online_state = 1u;

					Text = (uint32*)JWT_tx_setting_response(0x8E, 0x00, data);
					netbuf_ref(TCPNetbuf, Text, 17);
        	netconn_write(conn, Text, 17, NETCONN_NOCOPY);
					osDelay(200);
					
					Text = (uint32*)JWT_tx_0x0B_billing_rule_query_response(data);
					netbuf_ref(TCPNetbuf, Text, 75);
        	netconn_write(conn, Text, 75, NETCONN_NOCOPY);	

					osDelay(200);
					Text = (uint32*)JWT_tx_0x1F_gun_status(Ch, data);
					netbuf_ref(TCPNetbuf, Text, 18);
        	netconn_write(conn, Text, 18, NETCONN_NOCOPY);
				}
      }
    }
    else
    {
			LED_Function(NET_LED, LED_On);
			if (MsgFlag)
  		{
    		switch (Msg.type)
    		{
      case JWT_card_start:
        {
          if (g_JWT_info_t.card_start_rx_flag != 1)
          {
          	Text = (uint32*)JWT_tx_0x04_card_charge_start(Ch, data);
						netbuf_ref(TCPNetbuf, Text, 38);
        		netconn_write(conn, Text, 38, NETCONN_NOCOPY);
          }
        }
        break;

      case JWT_card_end:
        {
          if (g_JWT_info_t.card_stop_rx_flag != 1)
          {
          	Text = (uint32*)JWT_tx_0x05_card_charge_stop(Ch, data);
						netbuf_ref(TCPNetbuf, Text, 33);
        		netconn_write(conn, Text, 33, NETCONN_NOCOPY);
          }
        }
        break;

      case JWT_APP_start_success:
        {
        	Text = (uint32*)JWT_tx_0x86_app_charge_start_response(Ch, 0, 0, data);
					netbuf_ref(TCPNetbuf, Text, 25);
        	netconn_write(conn, Text, 25, NETCONN_NOCOPY);
        }
        break;

      case JWT_APP_start_fail:
        {
        	Text = (uint32*)JWT_tx_0x86_app_charge_start_response(Ch, 2, 0, data);
					netbuf_ref(TCPNetbuf, Text, 25);
        	netconn_write(conn, Text, 25, NETCONN_NOCOPY);
        }
        break;

      case JWT_charge_progress:
        {
          if (g_Charge_Ctrl_Manager[Ch].CurState == STATE_CHARGEING)
          {
          	if(Tick_Out(&tick[0],10000))
        		{            
         			Tick_Out(&tick[0],0);
          		Text = (uint32*)JWT_tx_0x09_charge_progress(Ch, data);
							netbuf_ref(TCPNetbuf, Text, 120);
        			netconn_write(conn, Text, 120, NETCONN_NOCOPY);
						
            	osDelay(200);

							Text = (uint32*)JWT_tx_0x0A_charge_progress_data(Ch, data);
							netbuf_ref(TCPNetbuf, Text, 148);
        			netconn_write(conn, Text, 148, NETCONN_NOCOPY);
          	}
          }
        }
        break;

      case JWT_warning:
        {
          uint16 warning_code;

          dword.b[1] = Msg.data[0];
          dword.b[0] = Msg.data[1];
          warning_code = dword.w[0];

          dword.b[3] = Msg.data[3];
          dword.b[2] = Msg.data[4];
          dword.b[1] = Msg.data[5];
          dword.b[0] = Msg.data[6];

					Text = (uint32*)JWT_tx_0x0D_charger_warning(Ch, warning_code, Msg.data[2], dword.v, data);
					netbuf_ref(TCPNetbuf, Text, 30);
        	netconn_write(conn, Text, 30, NETCONN_NOCOPY);
        }
        break;

      case JWT_QR_code:
        {
        	Text = (uint32*)JWT_tx_0x1C_QR_request(Ch, data);
					netbuf_ref(TCPNetbuf, Text, 17);
        	netconn_write(conn, Text, 17, NETCONN_NOCOPY);
        }
        break;

      case JWT_gun_status_change:
        {
        	Text = (uint32*)JWT_tx_0x1F_gun_status(Ch, data);
					netbuf_ref(TCPNetbuf, Text, 18);
        	netconn_write(conn, Text, 18, NETCONN_NOCOPY);
        }
        break;

			case JWT_charge_record:
				{
				}
				break;
			
      default:
        break;
      }
    }
			
			if(Tick_Out(&tick[1],15000))
      {            
        Tick_Out(&tick[1],0);
				Text = (uint32*)JWT_tx_0x0C_heart_beat(Ch,data);
      	netbuf_ref(TCPNetbuf, Text, 18);
      	netconn_write(conn, Text, 18, NETCONN_NOCOPY);
      	//osDelay(10000);
			}

          
			if (netconn_recv(conn, &TCPNetbuf) == ERR_OK)
      {
				offline_check = 0;
				rx_cmd = JWT_rx_handler(TCPNetbuf->p->payload);			
      }
			else
			{
				offline_check++;
				if (offline_check >= 300)
				{
					online_state = 0;
					offline_check = 0;
					LED_Function(NET_LED,LED_Off);
				}
			}
			
			if (g_Charge_Ctrl_Manager[Ch].CurState == STATE_CHARGEING)
  		{
    		memcpy(&g_RecordData.endStartTime[0], &g_JWT_info_t.heart_beat_time[0], 6);
  		}
  		else if ((g_RecordData.payMoney > 0) && (g_RecordData.payMoney <= 5) && (g_Charge_Ctrl_Manager[Ch].LastState == STATE_CHARGEING))
  		{
    		g_RecordData.payMoney++;

				Text = (uint32*)JWT_tx_0x08_charge_record(Ch, data);
				netbuf_ref(TCPNetbuf, Text, 125);
      	netconn_write(conn, Text, 125, NETCONN_NOCOPY);
  		}
    }

		//netbuf_free(TCPNetbuf);
		netbuf_delete(TCPNetbuf);
  }
}

osThreadId JWT_Thread_Id;

void Setup_JWT_background_App(osPriority ThreadPriority,uint8_t StackNum)
{
	//static 

  osThreadDef(JWT_thread, Cloud_Task, ThreadPriority, 0, StackNum*configMINIMAL_STACK_SIZE);

  //osThreadDef(JWT_thread, uart_net_daemon, ThreadPriority, 0, StackNum*configMINIMAL_STACK_SIZE);
  JWT_Thread_Id = osThreadCreate (osThread(JWT_thread), NULL);

  //JWT_Thread_Id = osThreadCreate(osThread(JWT_thread), NULL);
}







