

/******************************************************************************
*  
*  File name:     Tsk_Cloud_Com.h
*  Author:        
*  Version:       V1.00
*  Created on:    2017-01-03
*  Description:   
*  History:
*      Date        Author        Modification
*    1.2014-12-15  
*    2. ...
******************************************************************************/
#ifndef TSK_CLOUF_COM_H
#define TSK_CLOUF_COM_H

/******************************************************************************
* Includes
******************************************************************************/
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "Global.h"
#include "mbdatatype.h"
#include "Sys_Config.h"
#include "Utility.h"
#include "drv_time.h"
#include "Tsk_API_Card.h"

#define DBG_CLOUD_COMM 1 //调试开关

#define CHARGER_ID_LEN 10
#define MAX_LEN_RCV 256

/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/

#define TYPE_BUSINESS_CARD 0x01
#define TYPE_MANAGER  0x51
#define TYPE_ON_LINE  0x70
#define TYPE_APP_USR  0x71
/******************************************************************************
* Data Type Definitions
******************************************************************************/

typedef struct
{
	uint32 GprsBaudRate;
	uint8 GprsPortIp[9];
	uint8 GprsType;
	uint8 APN[6];       //3gnet
	uint8 ApnUser[6];
	uint8 ApnPassword[6];
}COMM_GPRS_PAPA;
typedef enum
{
	SP_GPRS,
	ROD_3G,
	OTHER_GPRS,
}WIRELESS_TYPE;

typedef enum
{
	GPRS_IDLE        ,
	GPRS_PARA_CFG    ,
	GPRS_CONNECTING  ,
	GPRS_CONNECTED   ,
	GPRS_LOGINING    ,
	GPRS_LOGIN_OK    ,
}GPRS_LINK_STATUS;

typedef enum
{
    ETHERNET_CMD_NONE=0X00,                                     //
    ETHERNET_CMD_HEART=0X01,                                    //心跳
    ETHERNET_CMD_LOGIN=0X02,                                    //登陆
    ETHERNET_CMD_OUT=0X03,                                      //退出
    ETHERNET_CMD_HOST_READ=0X04,                                //主站读数据
    ETHERNET_CMD_HOST_WRITE=0X05,                               //主站写数据
    ETHERNET_CMD_TERMINAL_UP_HISTORY_DATA=0X06,                 //终端主动上传历史数据
    ETHERNET_CMD_TERMIANL_UP_ALARM_DATA=0X07,                   //终端主动上传告警数据
    ETHERNET_CMD_HOST_BROADCAST_TIME=0X08,                      //主站广播对时
    ETHERNET_CMD_HOST_GATHER_REAL_DATA=0X9,                     //主站采集实时数据
    ETHERNET_CMD_START_OR_STOP_CONTROL=0X10,                    //启停控制
    ETHERNET_CMD_HOST_CHECK_NOUP_ALARM=0X11,                    //查询未上传告警
    ETHERNET_CMD_HOST_CHECK_NOUP_HISTORY=0X12,                  //查询未上传记录
    ETHERNET_CMD_TERMINAL_UP_DATA=0X13,                         //终端主动上传数据单元
    ETHERNET_CMD_TERMINAL_UP_REAL_DATA=0X14,                    //主动上传实时数据
    
    ETHERNET_CMD_BOOK_CRG = 0x20,                               //
    ETHERNET_CMD_RUSH_CARD = 0x30,                              //
    
    ETHERNET_CMD_UPDDATA = 0xa0,                                //升级
    ETHERNET_CMD_UPDDATA_TCP_START = 0xa1,                                //开始升级
    ETHERNET_CMD_UPDDATA_TCP_DATA = 0xa2,                                //开始升级
    ETHERNET_CMD_UPDDATA_TCP_END = 0xa3,                                //开始升级
}ETHERNET_CMD;  
typedef enum
{
	TERMINAL_TYPE_GROUND_AC,	
	TERMINAL_TYPE_HANG_AC,
	TERMINAL_TYPE_DC_SINGLE_S,
	TERMINAL_TYPE_DC_DOUBLE_S,
	TERMINAL_TYPE_DC_SINGLE,
	TERMINAL_TYPE_DC_DOUBLE,	
	TERMINAL_TYPE_DC_FOUR,
	TERMINAL_TYPE_DC_BOX_S,
	TERMINAL_TYPE_DC_BOX_D,
    TERMINAL_TYPE_ENVIRONMENT,  //环境监控
    TERMINAL_TYPE_AC_DOUBLE,    //双枪一体式交流
    TERMINAL_TYPE_AC_3PHASE,    //三相交流
    TERMINAL_TYPE_AC_HP,        //流水灯
}TERMINAL_TYPE;
typedef enum
{
    e_LOGIN_OUT,//未登陆
	e_LOGIN_IN,//已经登录
}LOGIN_STATUS;

typedef enum
{
    e_CRG_IMD = 0,//立即充电
    e_CRG_BOOK,   //预约充电
}CLOD_CRG_TYPE;


typedef struct
{
	uint32 startTime;
	LOGIN_STATUS  loginStatus;
	uint32	heartStartTime;
	uint32  heartInterval;
	uint8  connectStatus;

	uint8  linkType;
	uint32 heartTick;
}ETHNET_DATA;

typedef enum
{
	TERMINAL_LINK_STATUS_IDLE,
	TERMINAL_LINK_STATUS_CONNECTED,
	TERMINAL_LINK_STATUS_LOGINING,
	TERMINAL_LINK_STATUS_LOGIN_OK,
}TERMINAL_LINK_STATUS;
typedef enum
{
	MSG_CLOUD_START_CRG_SUCCESS,
	MSG_CLOUD_START_CRG_FAIL,
	MSG_CLOUD_STOP_CRG_SUCCESS,
	MSG_CLOUD_STOP_CRG_FAIL,

	MSG_CLOUD_BOOK_OK,
	MSG_CLOUD_BOOK_FAIL,
	MSG_CLOUD_CANCEL_BOOK_OK,
	MSG_CLOUD_CANCEL_BOOK_FAIL,
	MSG_CLOUD_AUTH_FIRST_CARD ,//充电开始验证卡片
	MSG_CLOUD_START_CRG,	//	
}CLOUD_MSG_MAIN;

#define DEVICE_TYPE  TERMINAL_TYPE_HANG_AC //设备类型 壁挂式

typedef struct
{
	uint8 server_ip[30];
	uint16 server_port;
	uint8 usr[20];
	uint8 pass[20];
	uint8 path[50];
    uint8 version[20];  //新增软件版本2016.09.20
	
}updata_info;

typedef struct
{
	
///////////////////////////////
    uint8   ReConnectTimes;  
	uint8   LinkStatus;	
  uint8   WireDevCommStatus;
	/////////////////////////////////
}UP_CTX;

void Cloud_Task(const void *p);
/******************************************************************************
* Global Variable Definitions
******************************************************************************/
extern CARD_DATA g_CardData[2];
extern int g_connect_flag;
osThreadId Get_Clod_Task_Id(void);
uint8 Get_Cloud_Comm_Statu(void);
uint8 Get_Cloud_Updata_Statu(void);
void SetupCloudCommApp(osPriority ThreadPriority,uint8_t StackNum);
#ifdef __cplusplus
     extern "C" {





#endif
#ifdef __cplusplus
}
#endif

#endif

