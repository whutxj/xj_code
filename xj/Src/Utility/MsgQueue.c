

#include "MsgQueue.h"
#include "queue.h"
#include <stdlib.h>
#define MEM_MALOOC	1
#include "string.h"

#define MSG_DBG 0

xQueueHandle MsgEvent[MAX_MSG_QUEUE];


/********************************************************
描    述 : 初始化消息队列 利于freertos 提供的API

输入参数 : taskId 线程的id 。Msg 消息类型 


输出参数 :无
返    回 :-1 发送出错  1 发送成功

创建时间 :15-8-26
*********************************************************/

uint8  MsgQueue_Init()
{

	int i = 0;

	for(i = 0; i< MAX_MSG_QUEUE;i++)
	{
		MsgEvent[i] = xQueueCreate( taskMESG_QUEUE_LENGTH,4);

		if(MsgEvent[i] == NULL)
		{
			return FALSE;
		}
	}

	return TRUE;
}

/********************************************************
描    述 : 向指定的线程发送一个消息

输入参数 : taskId 线程的id 。Msg 消息类型 


输出参数 :无
返    回 :-1 发送出错  1 发送成功

创建时间 :15-8-26
*********************************************************/

uint8 MsgQueue_Send(MSG_TypeDef taskId,MSG_Def Msg)
{
	int ret = 0;

	void *pMsg;
	
	if(taskId > MAX_MSG_QUEUE)
		return FALSE;

#ifdef MEM_MALOOC
	pMsg = (void*)malloc(sizeof(MSG_Def));
	
	if(pMsg == NULL)
	{
		
		return FALSE;
	}
    memcpy(pMsg,(uint8*)&Msg,sizeof(Msg));
#else
    {
        pMsg = &Msg;
    }
#endif
    
	ret =  xQueueSend(MsgEvent[taskId],&pMsg,1);

	if(ret != pdPASS)
	{
	#ifdef MEM_MALOOC
		free(pMsg);
	#endif
		return FALSE;
	}
	
	return TRUE;
}

/********************************************************
描    述 : 向指定的线程发送一个消息

输入参数 : taskId 线程的id 。


输出参数 :Msg 消息类型 
返    回 :TRUE 有接收到  FALSE 无消息接收

创建时间 :15-8-26
*********************************************************/
uint8 MsgQueue_Get(MSG_TypeDef taskId,MSG_Def *pMsg)
{
	void **buf;
	void *msg;
    
	uint8 ret = 0;
	
    buf = &msg;
    
	ret = xQueueReceive(MsgEvent[taskId],&(*buf),1);
    
	if(ret == pdTRUE)
	{
		memcpy(pMsg,(uint8*)msg,sizeof(MSG_Def));
	#ifdef MEM_MALOOC
		free(msg);
	#endif			
		return TRUE;
	}	
	return FALSE;	
}


