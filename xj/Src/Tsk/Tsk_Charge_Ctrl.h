

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

#define OPT_TIME_OUT 60 //刷卡后不动作时间
#define END_YET  0x00
#define NOT_END  0xf0

typedef enum
{
    STATE_IDLE,         //充电空闲待机状态
    STATE_CHARGEING,    //充电状态，
    STATE_END_CHARGE,   //充电停止状态
    STATE_BOOK,         //充电预约状态
}STATE;

typedef enum
{
    CTRL_OPEN_LOCK  = 0x20,     //控制开锁消恿
    CTRL_START_CHARGE,          //启动充电消息
    CTRL_CONTUNE_CHARGE,         //继续充电消息
    CTRL_STOP_CHARGE,           //启动充电消息
    CTRL_BOOK_CHARGE,           //预约充电消息
    CTRL_BOOK_CANCEL,           //取消预约充电消息
    AUTH_ONLINE_CARD,          //在线卿

}CHARGE_REMOUTE;




typedef enum
{
    LEVEL_9_V,                  //9V桿
    LEVEL_6_V,                  //6V
    LEVEL_12_V,                 //12V
}CAR_LINK_LEVER;

typedef enum
{
    PAY_YET,//已经付费
    UN_PAY,

}PAY_STATUS;

typedef enum
{
    CHARGE_OPT_NULL,
    BY_APP_START,                 //app 启动
    BY_CARD_START,                //卡启势
    BY_ON_LINE_AUTH,              //在线认证
    BY_APP_STOP,                 //app 启动
    BY_CARD_STOP,                //卡启势
    BY_OTEHER_STOP,              //其他方式结束
    BY_CARD_BOOK,
    BY_APP_BOOK,
    CHARGR_OPT_TIME_OUT,              //操作超时
}CHARGE_OPT;



typedef enum
{
    AUTO_MODE = 1,     //自动模式
    MONEY_MODE,     //金额模式
    TIME_MODE,     //时间模式
    ENERGE_MODE,   //电量模式
    BOOK_MODE,     //预约模式
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
    uint32  StartTime;          //开始时闿
    uint8   CardNo[16];         //充电卡号 asc砿6字节
    uint8  ChargeType;        //充电类型 0 卡充甿1 app 充电
    uint8  ChargeMode;        //充电模式 0 自动 1 时间模式 2 电量模式 3 金额模式
    uint8  StartCardType;     //充电卡类垿
    uint8  EndCardType;       //充电卡类垿
    uint32 ChargeTime;        //时间
    uint32 StartEnerge;         //开始电胿.01
    uint32 Balance;             //余额
    uint32 ListID;
    uint32 EndEnerge;           //结束电能0.01
    uint16 MaxVolt;             //最大电县0.1
    uint16 MaxCurent;           //最大电浿0.01
    uint16 ChargeMoney;         //充电金额 0.01
    uint8  StopReason;           //停止原因
    uint32 EndTime;            //结束时间
    uint8  SectionNum;          //半小时一个区间的数量
    uint16 SectionEnerge[48];  //区间数量的电能倿0.01
    uint32 crgServiceFee;
    uint32 totalFee;
    uint8 PayFlag;              //正常扣费标识
    uint8 Report;               //上报标识
    uint8 EndFlag;              //结束标识

    //预留
    uint8 Reverse[16];          //预留
}ChargeRecord;
#pragma pack()
typedef struct
{
    uint8 LastState;            //上一个状态机
    uint8 CurState;             //当前状态机
    uint32 StateStamp;          //状态机切换时间房

    uint32 LockStamp;          //开锁的时间房判断超时
    uint8 ChargeType;           //启动类型
    uint8 ChargeMode;           //充电方式
    uint8 ChargeContinu;        //继续上一次充甿
    uint32 ChargeModeVal;       //充电方式倿
	uint32 ListId;
    uint32 BookTime;            //预约充电时间
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
    uint8 CrgMode;           //充电方式
    uint32 CrgModeVal;       //充电方式倿
    uint8 AccountNo[16];

}UsrAccount;

typedef enum
{

    STOP_REASON_CURR_LOW_TIMEOUT = 100,//电流过小超时
    STOP_REASON_REACH_SET_STOP,			//到达设定的停止条件
    STOP_REASON_MANNAL_STOP,			//手动中止
    STOP_REASON_CAR_STOP,				//车主动停止
    STOP_BALANCE_LACK,					//余额不足
    STOP_MAX_CHARGE_TIME,				//达到最大允许充电时闿
    STOP_OFF_LINE,       				//网络掉线
    STOP_CANCEL_CHARGE,     			//取消充电   后台不计贿
		STOP_REASON_JUMP_GUN_STOP,			//??
		STOP_REASON_CHARGE_DRAW_GUN_STOP//充电中拔枪
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

