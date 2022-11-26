
#ifndef MSG_H
#define MSG_H

#include "Global.h"
#include "FreeRtos.h"


#define taskMESG_QUEUE_LENGTH   4
#define MSG_DATA_SIZE			32


typedef enum
{
	MSG_QUEUE_TASK_CHARGE_CTRL, //�������
	MSG_QUEUE_TASK_LCD,     	//�������߳�
	MSG_QUEUE_TASK_CLOUD_COMM,	//��ƽ̨����
	MSG_QUEUE_TASK_MODBUS,	    //׮��ͨѶ����
//	MSG_QUEUE_TASK_MODBUS_M,	//׮������ͨѶ����
	MSG_QUEUE_TASK_FLASH,	    //flash�洢����
	MAX_MSG_QUEUE,		        //������Ϣ��������	
}MSG_TypeDef;


typedef struct
{
	uint8 Ch;//��Ϣͨ��
	uint8 type;//��Ϣ����

	uint8 data[MSG_DATA_SIZE];//��Ϣ����
	
}MSG_Def;

typedef enum
{
	MSG_ETH_START_CRG = 0xa0,//��ʼ���
	MSG_ETH_STOP_CRG,

	MSG_ETH_BOOK,
	MSG_ETH_CANCEL_BOOK,

	MSG_ETH_CARD_AUTH,
}ETH_MSG;


/********************************************************
��    �� : ��ʼ����Ϣ���� ����freertos �ṩ��API

������� : taskId �̵߳�id ��Msg ��Ϣ���� 


������� :��
��    �� :-1 ���ͳ���  1 ���ͳɹ�

����ʱ�� :15-8-26
*********************************************************/

uint8  MsgQueue_Init(void);

/********************************************************
��    �� : ��ָ�����̷߳���һ����Ϣ

������� : taskId �̵߳�id ��Msg ��Ϣ���� 


������� :��
��    �� :-1 ���ͳ���  1 ���ͳɹ�

����ʱ�� :15-8-26
*********************************************************/

uint8 MsgQueue_Send(MSG_TypeDef taskId,MSG_Def Msg);

/********************************************************
��    �� : ��ָ�����̷߳���һ����Ϣ

������� : taskId �̵߳�id ��


������� :Msg ��Ϣ���� 
��    �� :TRUE �н��յ�  FALSE ����Ϣ����

����ʱ�� :15-8-26
*********************************************************/
uint8 MsgQueue_Get(MSG_TypeDef taskId,MSG_Def *pMsg);



#endif










