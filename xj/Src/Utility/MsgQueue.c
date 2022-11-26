

#include "MsgQueue.h"
#include "queue.h"
#include <stdlib.h>
#define MEM_MALOOC	1
#include "string.h"

#define MSG_DBG 0

xQueueHandle MsgEvent[MAX_MSG_QUEUE];


/********************************************************
��    �� : ��ʼ����Ϣ���� ����freertos �ṩ��API

������� : taskId �̵߳�id ��Msg ��Ϣ���� 


������� :��
��    �� :-1 ���ͳ���  1 ���ͳɹ�

����ʱ�� :15-8-26
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
��    �� : ��ָ�����̷߳���һ����Ϣ

������� : taskId �̵߳�id ��Msg ��Ϣ���� 


������� :��
��    �� :-1 ���ͳ���  1 ���ͳɹ�

����ʱ�� :15-8-26
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
��    �� : ��ָ�����̷߳���һ����Ϣ

������� : taskId �̵߳�id ��


������� :Msg ��Ϣ���� 
��    �� :TRUE �н��յ�  FALSE ����Ϣ����

����ʱ�� :15-8-26
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


