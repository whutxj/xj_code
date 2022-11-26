

/******************************************************************************
*
*  File name:     Tsk_Charge_Ctrl.h
*  Author:
*  Version:       V1.00
*  Created on:    2017-01-05
*  Description:
*  History:
*      Date        Author        Modification
*    1.2017-01-03
*    2. ...
******************************************************************************/
#ifndef TSK_CHARGE_CTRL_H
#define TSK_CHARGE_CTRL_H

/******************************************************************************
* Includes
******************************************************************************/
#include "main.h"
#include "cmsis_os.h"
#include "Global.h"
#include "Utility.h"
#include <time.h>
#include "Tsk_Api_Card.h"
#include "Tsk_API_Alarm.h"
#include "Tsk_API_LCD.h"
#include "BSP_LED_Function.h"
/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/
#define MAX_CHARGE_TIME (3600 * 24)
#define CARD_TIME_GAP    1
#define CONSUM_TIME_GAP    5
#define CUR_MIN_DISCONNECT    100

#define OPT_TIME_OUT 60 //ˢ���󲻶���ʱ��
#define END_YET  0x00
#define NOT_END  0xf0

typedef enum
{
    STATE_IDLE,         //�����д���״̬
    STATE_CHARGEING,    //���״̬��
    STATE_END_CHARGE,   //���ֹͣ״̬
    STATE_BOOK,         //���ԤԼ״̬
}STATE;

typedef enum
{
    CTRL_OPEN_LOCK  = 0x20,     //���ƿ�������
    CTRL_START_CHARGE,          //���������Ϣ
    CTRL_CONTUNE_CHARGE,         //���������Ϣ
    CTRL_STOP_CHARGE,           //���������Ϣ
    CTRL_BOOK_CHARGE,           //ԤԼ�����Ϣ
    CTRL_BOOK_CANCEL,           //ȡ��ԤԼ�����Ϣ
    AUTH_ONLINE_CARD,          //������

}CHARGE_REMOUTE;




typedef enum
{
    LEVEL_9_V,                  //9V�U
    LEVEL_6_V,                  //6V
    LEVEL_12_V,                 //12V
}CAR_LINK_LEVER;

typedef enum
{
    PAY_YET,//�Ѿ�����
    UN_PAY,

}PAY_STATUS;

typedef enum
{
    CHARGE_OPT_NULL,
    BY_APP_START,                 //app ����
    BY_CARD_START,                //������
    BY_ON_LINE_AUTH,              //������֤
    BY_APP_STOP,                 //app ����
    BY_CARD_STOP,                //������
    BY_OTEHER_STOP,              //������ʽ����
    BY_CARD_BOOK,
    BY_APP_BOOK,
    CHARGR_OPT_TIME_OUT,              //������ʱ
}CHARGE_OPT;



typedef enum
{
    AUTO_MODE = 1,     //�Զ�ģʽ
    MONEY_MODE,     //���ģʽ
    TIME_MODE,     //ʱ��ģʽ
    ENERGE_MODE,   //����ģʽ
    BOOK_MODE,     //ԤԼģʽ
}CHARGE_MODE;


typedef enum
{
    CTRL_OK,
    SYS_BUSY,       //
    SYS_BOOK_ING,           //
    SYS_LACK_MONEY,
}CHARGE_CLTR_RET;


/******************************************************************************
* Data Type Definitions
******************************************************************************/
#pragma pack(1)
typedef struct
{
    uint32  StartTime;          //��ʼʱ�]
    uint8   CardNo[16];         //��翨�� asc�m6�ֽ�
    uint8  ChargeType;        //������� 0 ����m1 app ���
    uint8  ChargeMode;        //���ģʽ 0 �Զ� 1 ʱ��ģʽ 2 ����ģʽ 3 ���ģʽ
    uint8  StartCardType;     //��翨����
    uint8  EndCardType;       //��翨����
    uint32 ChargeTime;        //ʱ��
    uint32 StartEnerge;         //��ʼ���v.01
    uint32 Balance;             //���
    uint32 ListID;
    uint32 EndEnerge;           //��������0.01
    uint16 MaxVolt;             //������0.1
    uint16 MaxCurent;           //�����0.01
    uint16 ChargeMoney;         //����� 0.01
    uint8  StopReason;           //ֹͣԭ��
    uint32 EndTime;            //����ʱ��
    uint8  SectionNum;          //��Сʱһ�����������
    uint16 SectionEnerge[48];  //���������ĵ��܂�0.01
    uint32 crgServiceFee;
    uint32 totalFee;
    uint8 PayFlag;              //�����۷ѱ�ʶ
    uint8 Report;               //�ϱ���ʶ
    uint8 EndFlag;              //������ʶ

    //Ԥ��
    uint8 Reverse[16];          //Ԥ��
}ChargeRecord;
#pragma pack()
typedef struct
{
    uint8 LastState;            //��һ��״̬��
    uint8 CurState;             //��ǰ״̬��
    uint32 StateStamp;          //״̬���л�ʱ�䷿

    uint32 LockStamp;          //������ʱ�䷿�жϳ�ʱ
    uint8 ChargeType;           //��������
    uint8 ChargeMode;           //��緽ʽ
    uint8 ChargeContinu;        //������һ�γ�m
    uint32 ChargeModeVal;       //��緽ʽ��
	uint32 ListId;
    uint32 BookTime;            //ԤԼ���ʱ��
    CARD_DATA CardData;
	uint8_t continue_cnt;
  uint8_t continue_flag;
	uint32  current_1A_tick;
  uint8_t pause_charge;
}Charge_Ctrl;

typedef struct
{
    uint32 Money;
	uint32 ListID;
    uint8 CrgMode;           //��緽ʽ
    uint32 CrgModeVal;       //��緽ʽ��
    uint8 AccountNo[16];

}UsrAccount;

typedef enum
{

    STOP_REASON_CURR_LOW_TIMEOUT = 100,//������С��ʱ
    STOP_REASON_REACH_SET_STOP,			//�����趨��ֹͣ����
    STOP_REASON_MANNAL_STOP,			//�ֶ���ֹ
    STOP_REASON_CAR_STOP,				//������ֹͣ
    STOP_BALANCE_LACK,					//����
    STOP_MAX_CHARGE_TIME,				//�ﵽ���������ʱ�]
    STOP_OFF_LINE,       				//�������
    STOP_CANCEL_CHARGE,     			//ȡ�����   ��̨���ƻ�
		STOP_REASON_JUMP_GUN_STOP,			//??
		STOP_REASON_CHARGE_DRAW_GUN_STOP//����а�ǹ
}STOP_REASON;

/******************************************************************************
* Global Variable Definitions
******************************************************************************/
 extern Charge_Ctrl g_Charge_Ctrl_Manager[2];
 extern ChargeRecord gChargeRecord[2];

#ifdef __cplusplus
 extern "C" {
#endif
/******************************************************************************
* Function Prototype Definitions
******************************************************************************/
uint8 Get_Charge_StartType(uint8 Ch);
uint8 Get_Charge_Ctrl_State(uint8 Ch);
ChargeRecord *Get_ChargeRecord(uint8 Ch);
int Ctrl_power_on(uint8 Ch,UsrAccount *Account);

void Ctrl_power_off(uint8 Reason,uint8 Ch);
void SetupChargeCtrlApp(osPriority ThreadPriority,uint8_t StackNum);
void Ctrl_Cancel_Book_charge(uint8 Ch);
void Ctrl_Book_charge(uint32 Time,uint8* Usr,uint8 Ch);
void Ctrl_Continu_Charge(uint8 Ch);
uint32 Get_Book_Time(uint8 Ch);
uint8 Get_Continue_Charge(uint8 Ch);
int  Updata_UsrAccount(uint8 *CardNo,uint32 Money,int Ch);
osThreadId Get_Charge_Ctrl_Task_Id(void);
uint8 Dwin_Lcd_State(uint8 LcdState,uint8 Ch);

#ifdef __cplusplus
}
#endif

#endif

