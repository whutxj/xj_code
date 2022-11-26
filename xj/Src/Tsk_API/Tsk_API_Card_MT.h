/******************************************************************************
*  Copyright (C), 1988-2014, Kehua Tech. Co., Ltd.
*  File name:     Tsk_API_Card.c
*  Author:        lx
*  Version:       V1.00
*  Created on:    2016-12-22
*  Description:   
*  History:
*      Date        Author        Modification
*    1.2014-12-15  
*    2. ...
******************************************************************************/
#ifndef _TSK_API_CARD_H
#define _TSK_API_CARD_H

/******************************************************************************
* Include Files
******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "Global.h"


#define MIN_ALLOW_MONEY    200 //��?D??��D��3?��??�� 

/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/

typedef enum
{
    USR_CARD = 1,   //??��?��??��?����o��D����??��?2��?����D???��������?
    SUPER_MANAGE,   //??��D����???��?T???��������?
    MANAGE_CARD,    //��D����??��??T???��������?
    ONLINE_CARD,    //?��???��
    
}CARD_TYPE;         //?������D��


typedef enum
{
    STATUS_UNLOCK,//û��
    STATUS_LOCK,  //����
    
}CARD_STATUS;     //��״̬


typedef enum
{
    OPT_OK,             //���ɹ�
    OPT_BALANCE_LACK,   //����
    OPT_LOCK,           //������
    OPT_ERR,            //������
    
}START_CRG_OPT_CARD;    //�������״̬

typedef enum
{
    END_OK,             //���ɹ�
    END_NOT_MATCH,      //��ƥ��
    END_ERR,            //������
    
}END_OPT_CARD;          //�������״̬


typedef enum
{
    CMD_OPRATION_FIND_CARD              =0X0102,//Ѱ������
    CMD_OPRATION_PREVENT_CONFLICT       =0X0202,//����ͻ
    CMD_OPRATION_CHOICE_CARD            =0X0302,//ѡ��
    CMD_OPRATION_AUTHENTICATE_PASSWORD  =0X0702,//��֤����
    CMD_OPRATION_READ_CARD              =0X0802,//����
    CMD_OPRATION_WRITE_CARD             =0X0902,//д��
    CMD_OPRATION_DORMANT                =0X0402,//����
    CMD_OPRATION_HARDWARE_VERSION       =0X0401,//Ӳ���汾��
    CMD_OPRATION_BUZZER                 =0X0601,//������
    CMD_OPRATION_INIT                   =0X0A02,//��ʼ��ֵ
    CMD_OPRATION_READING                =0X0B02,//��ֵ  CMD_OPRATION_RECHARGE               =0X0D02,//��ֵ  CMD_OPRATION_DEVALUE                =0X0C02,//��ֵ  CMD_OPRATION_ONEKEY_READ_CARD       =0X0806,//һ������  CMD_OPRATION_ONEKEY_WRITE_CARD      =0X0906,//һ��д��  CMD_OPRATION_ONEKEY_CHANGE_PASSWORD =0X0906,//һ��������    CMD_OPRATION_READ_ONLY              =0X0106,//ֻ��ģʽ  CMD_OPRATION_READ_WRITE             =0X0106,//��дģʽ
    CMD_OPRATION_READ_FLASH             =0X0A06,//��flash����
    CMD_OPRATION_WRITE_FLASH            =0X0B06,//дflash����
    CMD_OPRATION_FAST_READ              =0X0806,//һ������
    CMD_OPRATION_READ_WRITE             =0X0106,
    CMD_OPRATION_DEC_VAL                =0X0c02,
    CMD_OPRATION_ADD_VAL                =0X0d02,
}CMD_OPRATION_M2;

typedef enum
{
    FIND_CARD_NONE,        //�޿�
    FIND_CARD_OK,          //�п�
    FIND_CARD_ERR,         //ͨѶ����
}FIND_OPT_CODE;

typedef struct
{ 
    uint8   Card_no[16];    //����
    int  Balance;        //���
    uint8   CardType;       //������
    uint8   CardStatus;     //��״̬
}CARD_DATA;

typedef struct
{
    uint32  startTime;      //��ʼʱ��
    uint32  EndTime;        //�������ʱ��
    uint16   Money;         //���ѽ��
    uint16   Energe;        //����
    uint8    type;
}CARD_RECORD;

/******************************************************************************
* Static Function Prototype Definitions
******************************************************************************/

uint8 API_Search_Card(CARD_DATA *PCardData);

START_CRG_OPT_CARD API_Start_Charge_Card(CARD_DATA *PCardData,uint8 Ch);

END_OPT_CARD API_End_Charge_Card(CARD_RECORD Record,int Ch);

uint8 Get_CardCommStatu(void);
uint8 M2_Set_ReadWrite_Mode(void);
END_OPT_CARD API_Add_Charge_Card(CARD_RECORD Record,int Ch);
void M2_Card_Read_init(void);
uint8 Set_Charge_Card_No(uint8 *CardNo,uint8 Ch);
#endif
