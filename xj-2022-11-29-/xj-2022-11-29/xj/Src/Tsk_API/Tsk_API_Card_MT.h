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


#define MIN_ALLOW_MONEY    200 //×?D??êDí3?μ??μ 

/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/

typedef enum
{
    USR_CARD = 1,   //??è?ó??§?¨￡oóDóà??￡?2￠?òóD???¨±êê?
    SUPER_MANAGE,   //??óDóà???￡?T???¨±êê?
    MANAGE_CARD,    //óDóà??￡??T???¨±êê?
    ONLINE_CARD,    //?ú???¨
    
}CARD_TYPE;         //?¨ààDí


typedef enum
{
    STATUS_UNLOCK,//没锁
    STATUS_LOCK,  //锁定
    
}CARD_STATUS;     //卡状态


typedef enum
{
    OPT_OK,             //读成功
    OPT_BALANCE_LACK,   //余额不足
    OPT_LOCK,           //被锁定
    OPT_ERR,            //读出错
    
}START_CRG_OPT_CARD;    //结束充电状态

typedef enum
{
    END_OK,             //读成功
    END_NOT_MATCH,      //不匹配
    END_ERR,            //读出错
    
}END_OPT_CARD;          //结束充电状态


typedef enum
{
    CMD_OPRATION_FIND_CARD              =0X0102,//寻卡命令
    CMD_OPRATION_PREVENT_CONFLICT       =0X0202,//防冲突
    CMD_OPRATION_CHOICE_CARD            =0X0302,//选卡
    CMD_OPRATION_AUTHENTICATE_PASSWORD  =0X0702,//认证密码
    CMD_OPRATION_READ_CARD              =0X0802,//读卡
    CMD_OPRATION_WRITE_CARD             =0X0902,//写卡
    CMD_OPRATION_DORMANT                =0X0402,//休眠
    CMD_OPRATION_HARDWARE_VERSION       =0X0401,//硬件版本号
    CMD_OPRATION_BUZZER                 =0X0601,//蜂鸣器
    CMD_OPRATION_INIT                   =0X0A02,//初始化值
    CMD_OPRATION_READING                =0X0B02,//读值  CMD_OPRATION_RECHARGE               =0X0D02,//充值  CMD_OPRATION_DEVALUE                =0X0C02,//减值  CMD_OPRATION_ONEKEY_READ_CARD       =0X0806,//一键读卡  CMD_OPRATION_ONEKEY_WRITE_CARD      =0X0906,//一键写卡  CMD_OPRATION_ONEKEY_CHANGE_PASSWORD =0X0906,//一键改密码    CMD_OPRATION_READ_ONLY              =0X0106,//只读模式  CMD_OPRATION_READ_WRITE             =0X0106,//读写模式
    CMD_OPRATION_READ_FLASH             =0X0A06,//读flash数据
    CMD_OPRATION_WRITE_FLASH            =0X0B06,//写flash数据
    CMD_OPRATION_FAST_READ              =0X0806,//一键读卡
    CMD_OPRATION_READ_WRITE             =0X0106,
    CMD_OPRATION_DEC_VAL                =0X0c02,
    CMD_OPRATION_ADD_VAL                =0X0d02,
}CMD_OPRATION_M2;

typedef enum
{
    FIND_CARD_NONE,        //无卡
    FIND_CARD_OK,          //有卡
    FIND_CARD_ERR,         //通讯错误
}FIND_OPT_CODE;

typedef struct
{ 
    uint8   Card_no[16];    //卡号
    int  Balance;        //余额
    uint8   CardType;       //卡类型
    uint8   CardStatus;     //卡状态
}CARD_DATA;

typedef struct
{
    uint32  startTime;      //开始时间
    uint32  EndTime;        //结束充电时间
    uint16   Money;         //消费金额
    uint16   Energe;        //电能
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
