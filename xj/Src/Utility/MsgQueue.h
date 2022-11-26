
#ifndef MSG_H
#define MSG_H

#include "Global.h"
#include "FreeRtos.h"


#define taskMESG_QUEUE_LENGTH   4
#define MSG_DATA_SIZE			32


typedef enum
{
	MSG_QUEUE_TASK_CHARGE_CTRL, //充电任务
	MSG_QUEUE_TASK_LCD,     	//触摸屏线程
	MSG_QUEUE_TASK_CLOUD_COMM,	//云平台任务
	MSG_QUEUE_TASK_MODBUS,	    //桩间通讯任务
//	MSG_QUEUE_TASK_MODBUS_M,	//桩间主机通讯任务
	MSG_QUEUE_TASK_FLASH,	    //flash存储任务
	MAX_MSG_QUEUE,		        //最大的消息队列数量	
}MSG_TypeDef;


typedef struct
{
	uint8 Ch;//消息通道
	uint8 type;//消息类型

	uint8 data[MSG_DATA_SIZE];//消息数据
	
}MSG_Def;

typedef enum
{
	MSG_ETH_START_CRG = 0xa0,//开始充电
	MSG_ETH_STOP_CRG,

	MSG_ETH_BOOK,
	MSG_ETH_CANCEL_BOOK,

	MSG_ETH_CARD_AUTH,
}ETH_MSG;


/********************************************************
描    述 : 初始化消息队列 利于freertos 提供的API

输入参数 : taskId 线程的id 。Msg 消息类型 


输出参数 :无
返    回 :-1 发送出错  1 发送成功

创建时间 :15-8-26
*********************************************************/

uint8  MsgQueue_Init(void);

/********************************************************
描    述 : 向指定的线程发送一个消息

输入参数 : taskId 线程的id 。Msg 消息类型 


输出参数 :无
返    回 :-1 发送出错  1 发送成功

创建时间 :15-8-26
*********************************************************/

uint8 MsgQueue_Send(MSG_TypeDef taskId,MSG_Def Msg);

/********************************************************
描    述 : 向指定的线程发送一个消息

输入参数 : taskId 线程的id 。


输出参数 :Msg 消息类型 
返    回 :TRUE 有接收到  FALSE 无消息接收

创建时间 :15-8-26
*********************************************************/
uint8 MsgQueue_Get(MSG_TypeDef taskId,MSG_Def *pMsg);



#endif










