/* USER CODE BEGIN Header */
/**
 ******************************************************************************
  * File Name          : LWIP.c
  * Description        : This file provides initialization code for LWIP
  *                      middleWare.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "lwip.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#if defined ( __CC_ARM )  /* MDK ARM Compiler */
#include "lwip/sio.h"
#endif /* MDK ARM Compiler */
#include "ethernetif.h"

/* USER CODE BEGIN 0 */
//#ifndef LWIP_DNS
//#define LWIP_DNS
//#endif
#include "main.h"
#include "lwip/dns.h"
#include "Sys_Config.h"
/* USER CODE END 0 */
/* Private function prototypes -----------------------------------------------*/
static void ethernet_link_status_updated(struct netif *netif);
/* ETH Variables initialization ----------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN 1 */

/* Variables Initialization */
#define USRDHCP //使用DHCP
/* DHCP process states */
#define DHCP_START 1
#define DHCP_WAIT_ADDRESS 2
#define DHCP_ADDRESS_ASSIGNED 3
#define DHCP_TIMEOUT 4
#define DHCP_LINK_DOWN 5
#define MAX_DHCP_TRIES 4
struct netif gnetif;
ip4_addr_t ipaddr;
ip4_addr_t netmask;
ip4_addr_t gw;
ip4_addr_t DnsSer;
uint8_t IP_ADDRESS[4];
uint8_t NETMASK_ADDRESS[4];
uint8_t GATEWAY_ADDRESS[4];
__IO uint8_t DHCP_state; 
/* USER CODE END 1 */

/* Variables Initialization */
struct netif gnetif;
ip4_addr_t ipaddr;
ip4_addr_t netmask;
ip4_addr_t gw;

/* USER CODE BEGIN 2 */

void MX_LWIP_Set_IP(void)
{

    //ip4_addr_t DnsSer;
    unsigned char TmpBuf[4] = {0};
    Get_Local_Ip(TmpBuf);
    IP4_ADDR(&ipaddr, TmpBuf[0], TmpBuf[1],TmpBuf[2],TmpBuf[3]);
    Get_NetMask(TmpBuf);
    IP4_ADDR(&netmask,TmpBuf[0], TmpBuf[1],TmpBuf[2],TmpBuf[3]);
    Get_GetAway(TmpBuf);
    IP4_ADDR(&gw, TmpBuf[0], TmpBuf[1],TmpBuf[2],TmpBuf[3]);  
    netif_set_addr(&gnetif,&ipaddr,&netmask,&gw);
    Get_DomainSer(TmpBuf);
    IP4_ADDR(&DnsSer, TmpBuf[0], TmpBuf[1],TmpBuf[2],TmpBuf[3]);
    dns_setserver(1,&DnsSer);    
}
struct netif *pnetif = &gnetif;//(struct netif *) argument;
void DHCP_Pool(void const * argument)
{
    unsigned char TmpBuf[4] = {0};
    uint8 iptab[4] = {0};
    uint32_t IPaddress;
		ip_addr_t *ip_addr;
		struct dhcp *dhcp_try;
    uint8 AutoIp = Get_AutoIP_En();
    SYS_CONFIG_PARAM *pSysParam = GetTmp_Config_Param();

    if(AutoIp == TRUE)
    {//手动获取直接返回
        return ;
    }
    
    switch (DHCP_state)
    {
        case DHCP_START:
          {
            pnetif->ip_addr.addr = 0;
            pnetif->netmask.addr = 0;
            pnetif->gw.addr = 0;
            IPaddress = 0;
            dhcp_start(pnetif);
            DHCP_state = DHCP_WAIT_ADDRESS;
          }
          break;
          
        case DHCP_WAIT_ADDRESS:
          {        
            /* Read the new IP address */
            IPaddress = pnetif->ip_addr.addr;
            
            if (IPaddress!=0) 
            {
              DHCP_state = DHCP_ADDRESS_ASSIGNED;	
              
             ip_addr = (ip_addr_t *)dns_getserver(0);
              /* Stop DHCP */
              dhcp_stop(pnetif);

              iptab[3] = (uint8_t)(IPaddress >> 24);
              iptab[2] = (uint8_t)(IPaddress >> 16);
              iptab[1] = (uint8_t)(IPaddress >> 8);
              iptab[0] = (uint8_t)(IPaddress);
              memcpy((uint8*)pSysParam->LocalIp,(uint8*)iptab,4);
                
              IPaddress = pnetif->gw.addr;
              iptab[3] = (uint8_t)(IPaddress >> 24);
              iptab[2] = (uint8_t)(IPaddress >> 16);
              iptab[1] = (uint8_t)(IPaddress >> 8);
              iptab[0] = (uint8_t)(IPaddress);                ;
              memcpy((uint8*)pSysParam->GetAway,(uint8*)iptab,4);

              IPaddress = pnetif->netmask.addr;
              iptab[3] = (uint8_t)(IPaddress >> 24);
              iptab[2] = (uint8_t)(IPaddress >> 16);
              iptab[1] = (uint8_t)(IPaddress >> 8);
              iptab[0] = (uint8_t)(IPaddress);                ;
              memcpy((uint8*)pSysParam->NetMask,(uint8*)iptab,4);                
                   
              IPaddress  = ip_addr->addr;
              iptab[3] = (uint8_t)(IPaddress >> 24);
              iptab[2] = (uint8_t)(IPaddress >> 16);
              iptab[1] = (uint8_t)(IPaddress >> 8);
              iptab[0] = (uint8_t)(IPaddress);
              memcpy((uint8*)pSysParam->DomainSer,(uint8*)iptab,4); 
              
            }
            else
            {
              /* DHCP timeout */
							dhcp_try = netif_dhcp_data(pnetif);
              if (dhcp_try->tries > MAX_DHCP_TRIES)
              {
                DHCP_state = DHCP_TIMEOUT;
                
                /* Stop DHCP */
                dhcp_stop(pnetif);
                
                /* Static address used */
                  Get_Local_Ip(TmpBuf);
                  IP4_ADDR(&ipaddr, TmpBuf[0], TmpBuf[1],TmpBuf[2],TmpBuf[3]);
                  dbg_printf(1,"auto ip %d.%d.%d.%d",TmpBuf[0] ,TmpBuf[1] , TmpBuf[2] ,TmpBuf[3] ); 
                  Get_NetMask(TmpBuf);
                  IP4_ADDR(&netmask,TmpBuf[0], TmpBuf[1],TmpBuf[2],TmpBuf[3]);
                  Get_GetAway(TmpBuf);
                  IP4_ADDR(&gw, TmpBuf[0], TmpBuf[1],TmpBuf[2],TmpBuf[3]);
                  netif_set_addr(pnetif, &ipaddr , &netmask, &gw);
                  Get_DomainSer(TmpBuf);
                  IP4_ADDR(&DnsSer, TmpBuf[0], TmpBuf[1],TmpBuf[2],TmpBuf[3]);
                  dns_setserver(1,&DnsSer);
                  dbg_printf(1,"auto dns %d.%d.%d.%d",TmpBuf[0] ,TmpBuf[1] , TmpBuf[2] ,TmpBuf[3] );
              }
            }
          }
          break;
        case DHCP_ADDRESS_ASSIGNED:
        {
            IPaddress = pnetif->ip_addr.addr;
            if (IPaddress==0)
            {
                DHCP_state = DHCP_START;
            }                
        }
          break;
          
        default: break;
  }       
}

void Get_Auto_Ip(char *IP)
{
    uint32_t IPaddress;
    char iptab[4];
     
    IPaddress = pnetif->ip_addr.addr;
    iptab[0] = (uint8_t)(IPaddress >> 24);
    iptab[1] = (uint8_t)(IPaddress >> 16);
    iptab[2] = (uint8_t)(IPaddress >> 8);
    iptab[3] = (uint8_t)(IPaddress);

    sprintf(IP,"%d.%d.%d.%d",iptab[3],iptab[2],iptab[1],iptab[0]);
    
}

void ethernetif_notify_conn_changed(struct netif *netif)
{
  ip4_addr_t l_ipaddr;
  ip4_addr_t l_netmask;
  ip4_addr_t l_gw;

  if(netif_is_link_up(netif))
  {
#ifdef USE_DHCP
#ifdef USE_LCD
    LCD_UsrLog ("The network cable is now connected \n");
#else
    BSP_LED_Off(LED2);
    BSP_LED_On(LED1);
#endif /* USE_LCD */
    /* Update DHCP state machine */
    DHCP_state = DHCP_START;
#else
    IP4_ADDR(&l_ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
    IP4_ADDR(&l_netmask, NETMASK_ADDR0, NETMASK_ADDR1 , NETMASK_ADDR2, NETMASK_ADDR3);
    IP4_ADDR(&l_gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);    

#ifdef USE_LCD        
    uint8_t iptxt[20];
    
    sprintf((char*)iptxt, "%d.%d.%d.%d", IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
    LCD_UsrLog ("The network cable is now connected \n");
    LCD_UsrLog ("Static IP address: %s\n", iptxt);
#else
//    BSP_LED_Off(LED2);
//    BSP_LED_On(LED1);
#endif /* USE_LCD */
#endif /* USE_DHCP */   
    
    netif_set_addr(netif, &l_ipaddr , &l_netmask, &l_gw);
    
    /* When the netif is fully configured this function must be called.*/
    netif_set_up(pnetif);     
  }
  else
  {
#ifdef USE_DHCP
    /* Update DHCP state machine */
    DHCP_state = DHCP_LINK_DOWN;
#endif /* USE_DHCP */
    
    /*  When the netif link is down this function must be called.*/
    netif_set_down(pnetif);
    
#ifdef USE_LCD
    LCD_UsrLog ("The network cable is not connected \n");
#else
//    BSP_LED_Off(LED1);
//    BSP_LED_On(LED2);
#endif /* USE_LCD */    
  }
}
/* USER CODE END 2 */

/**
  * LwIP initialization function
  */
void MX_LWIP_Init(void)
{
  /* Initilialize the LwIP stack with RTOS */
  tcpip_init( NULL, NULL );

  /* IP addresses initialization with DHCP (IPv4) */
  ipaddr.addr = 0;
  netmask.addr = 0;
  gw.addr = 0;

  /* add the network interface (IPv4/IPv6) with RTOS */
  netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);

  /* Registers the default network interface */
  netif_set_default(&gnetif);

  if (netif_is_link_up(&gnetif))
  {
    /* When the netif is fully configured this function must be called */
    netif_set_up(&gnetif);
  }
  else
  {
    /* When the netif link is down this function must be called */
    netif_set_down(&gnetif);
  }

  /* Set the link callback function, this function is called on change of link status*/
  netif_set_link_callback(&gnetif, ethernet_link_status_updated);

  /* Create the Ethernet link handler thread */
/* USER CODE BEGIN H7_OS_THREAD_DEF_CREATE_CMSIS_RTOS_V1 */
  osThreadDef(EthLink, ethernet_link_thread, osPriorityBelowNormal, 0, configMINIMAL_STACK_SIZE *2);
  osThreadCreate (osThread(EthLink), &gnetif);
/* USER CODE END H7_OS_THREAD_DEF_CREATE_CMSIS_RTOS_V1 */

  /* Start DHCP negotiation for a network interface (IPv4) */
  dhcp_start(&gnetif);

/* USER CODE BEGIN 3 */

/* USER CODE END 3 */
}

#ifdef USE_OBSOLETE_USER_CODE_SECTION_4
/* Kept to help code migration. (See new 4_1, 4_2... sections) */
/* Avoid to use this user section which will become obsolete. */
/* USER CODE BEGIN 4 */

/* USER CODE END 4 */
#endif

/**
  * @brief  Notify the User about the network interface config status
  * @param  netif: the network interface
  * @retval None
  */
static void ethernet_link_status_updated(struct netif *netif)
{
  if (netif_is_up(netif))
  {
/* USER CODE BEGIN 5 */
/* USER CODE END 5 */
  }
  else /* netif is down */
  {
/* USER CODE BEGIN 6 */
/* USER CODE END 6 */
  }
}

#if defined ( __CC_ARM )  /* MDK ARM Compiler */
/**
 * Opens a serial device for communication.
 *
 * @param devnum device number
 * @return handle to serial device if successful, NULL otherwise
 */
sio_fd_t sio_open(u8_t devnum)
{
  sio_fd_t sd;

/* USER CODE BEGIN 7 */
  sd = 0; // dummy code
/* USER CODE END 7 */

  return sd;
}

/**
 * Sends a single character to the serial device.
 *
 * @param c character to send
 * @param fd serial device handle
 *
 * @note This function will block until the character can be sent.
 */
void sio_send(u8_t c, sio_fd_t fd)
{
/* USER CODE BEGIN 8 */
/* USER CODE END 8 */
}

/**
 * Reads from the serial device.
 *
 * @param fd serial device handle
 * @param data pointer to data buffer for receiving
 * @param len maximum length (in bytes) of data to receive
 * @return number of bytes actually received - may be 0 if aborted by sio_read_abort
 *
 * @note This function will block until data can be received. The blocking
 * can be cancelled by calling sio_read_abort().
 */
u32_t sio_read(sio_fd_t fd, u8_t *data, u32_t len)
{
  u32_t recved_bytes;

/* USER CODE BEGIN 9 */
  recved_bytes = 0; // dummy code
/* USER CODE END 9 */
  return recved_bytes;
}

/**
 * Tries to read from the serial device. Same as sio_read but returns
 * immediately if no data is available and never blocks.
 *
 * @param fd serial device handle
 * @param data pointer to data buffer for receiving
 * @param len maximum length (in bytes) of data to receive
 * @return number of bytes actually received
 */
u32_t sio_tryread(sio_fd_t fd, u8_t *data, u32_t len)
{
  u32_t recved_bytes;

/* USER CODE BEGIN 10 */
  recved_bytes = 0; // dummy code
/* USER CODE END 10 */
  return recved_bytes;
}
#endif /* MDK ARM Compiler */

