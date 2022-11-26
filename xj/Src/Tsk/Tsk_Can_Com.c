/******************************************************************************
*  Copyright (C), 1988-2014, Kehua Tech. Co., Ltd.
*  File name:     Tsk_Can_Com.c
*  Author:        lx
*  Version:       V1.00
*  Created on:    2017-01-03
*  Description:   
*  History:
*      Date        Author        Modification
*    1.2014-12-15  
*    2. ...
******************************************************************************/

/******************************************************************************
* Includes
******************************************************************************/
#include "Tsk_Can_Com.h"

#include "Sys_Config.h"
#include "Utility.h"
#include "drv_time.h"
#include "Tsk_Cloud_Com.h"
#include "string.h"
#include "Global.h"

#include "drv_usart.h"
#include "Database.h"

#include "stdarg.h"

#include "MsgQueue.h"
#include "Tsk_Charge_Ctrl.h"
#include <stdlib.h>
#include "Tsk_API_Alarm.h"
#include "Task_API_SpiFlash.h"
#include "Task_SpiFlash.h"
#include "Tsk_API_Card.h"
#include "Update.h"
#include "Tsk_Measure.h"
#include "Tsk_API_Meter.h"
#include "Tsk_TouchDW.h"

#include "Tsk_API_CAN_Protocol.h"

static uint8 ADDR_MY;
const char Can_Ver[3] = {4,0,0};//V4.00

uint8 MutliSendbuf[256] = {0};

typedef struct
{
    uint8 ChargeServerStatus:1;//充电机服务状态
    uint8 CommStatus:1;//充电机报文状态
    uint8 CollectVer[3];//集中器版本号
}Can_Comm;

typedef union 
{
    uint8 Byte;
    struct
    {
        uint8 Report_Param:1;
        uint8 Report_YC1:1;
        uint8 Report_YC2:1;
        uint8 Report_YX1:1;
        uint8 Report_YX2:1;
        uint8 Report_STOP:1;         
    }Bit;
}BYTE_TYPE;

static Can_Comm st_Can_Comm;
static BYTE_TYPE SendFlag;

/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/

static int Report_Stop_Complete(uint8 Reason,uint8 Result,uint8 Ch);

/********************************************************
描    述 : 开始充电应答
输入参数 : 无

输出参数 :
返    回 :
作    者 :lx

创建时间 :17-11-14
*********************************************************/
static void Ack_Start_Charge(uint8 *data,uint8 len)
{
    uint8 ret =0;

    uint8_t buf[8] ={0};
    
    int Ch = data[0];
    
    if(Ch != 0)
    {
        ret = 1;
    }
    else
    {
        if(Get_FaultBit(Ch))
        {
            ret = 1;//start faile
        }
        if(Get_Charge_Ctrl_State(Ch) == STATE_CHARGEING
            || Get_Charge_Ctrl_State(Ch) == STATE_END_CHARGE)
        {
            ret = 1;//start faile
        }
        if(ret == 0)
        {
            UsrAccount stUsrAccount;
            stUsrAccount.Money = 999999;
            if(Ctrl_power_on(Ch,&stUsrAccount) == CTRL_OK)
            {
                ret = 0;
            }
            else
            {
                ret = 1;//start faile
            }
        }
    }
    memcpy(buf,data,2);
    
    buf[2] = ret;
    
    API_Send_Single(4,ID_START_CHARGE_RES,ADDR_COLLECT,ADDR_MY,buf,8);
    
}
/********************************************************
描    述 : 停止应答
输入参数 : 无

输出参数 :
返    回 :
作    者 :lx

创建时间 :17-11-14
*********************************************************/
static void Ack_Stop_Charge(uint8 *data,uint8 len)
{
    uint8 ret =0;
 
    uint8_t buf[8] ={0};
    
    int Ch = data[0];
    
    if(Ch != 0)
    {
        ret = 1;
    }
    else
    {
        if(Get_Charge_Ctrl_State(Ch) == STATE_IDLE)
        {
            ret = 1;
        }
    }
    memcpy(buf,data,2);
    
    buf[2] = ret;
    
    API_Send_Single(4,ID_STOP_RES,ADDR_COLLECT,ADDR_MY,buf,8);

    Report_Stop_Complete(1,0,Ch);
    
    SendFlag.Bit.Report_STOP = 1;
}
/********************************************************
描    述 : 应答版本检测
输入参数 : 无

输出参数 :
返    回 :
作    者 :lx

创建时间 :17-11-14
*********************************************************/
static void Ack_Ver_Check(uint8 *data,uint8 len)
{
    
    uint8_t buf[8] ={0};
    
    memcpy(st_Can_Comm.CollectVer,data + 1,3);
    buf[0] = data[0];
    
    memcpy(buf+1,Can_Ver,3);
    
    API_Send_Single(6,ID_VERION_RES,ADDR_COLLECT,ADDR_MY,buf,8);
    
}
/********************************************************
描    述 : 应答设置时间处理
输入参数 : 无

输出参数 :
返    回 :
作    者 :lx

创建时间 :17-11-14
*********************************************************/
static void Ack_Set_time(uint8 *data,uint8 len)
{
    
 
    int Ch = data[0];
    struct tm  SystemTimer;

    struct tm *SetingTime;

    if(Get_Charge_Ctrl_State(Ch) != STATE_CHARGEING)
    {
        SystemTimer.tm_year = data[2] + 100;
        SystemTimer.tm_mon = data[3] - 1;
        SystemTimer.tm_mday = data[4];
        
        														
        SystemTimer.tm_hour = data[5];	
        SystemTimer.tm_min = data[6];
        SystemTimer.tm_sec = data[7];
        
		time_t time = mktime(&SystemTimer);
		time -= 8*3600;
		SetingTime = localtime(&time);
		
        drv_set_sys_time(*SetingTime);
     
        Notice_Lcd_State(SET_TIME,0);
    }
    
    API_Send_Single(6,ID_SET_TIME_RES,ADDR_COLLECT,ADDR_MY,data,8);
    
}


static void Parse_Report_Param(uint8 *data,uint8 len)
{
    SendFlag.Bit.Report_Param = 0;
}


static int Fill_Param_data(uint8 *data,uint8 Ch)
{
    
}

/********************************************************
描    述 : 上报开始完成
输入参数 : 无

输出参数 :
返    回 :
作    者 :lx

创建时间 :17-11-14
*********************************************************/
static int Report_Start_Complete(uint8 Result,uint8 Ch)
{
    uint8_t buf[8] ={0};
    
    
    buf[0] = Ch + 1;
    buf[1] = 2;
    buf[2] = Result;
    
    API_Send_Single(4,ID_START_OVER,ADDR_COLLECT,ADDR_MY,buf,8);
    
    return 0;
}

/********************************************************
描    述 :上报充电完成
输入参数 : 无

输出参数 :
返    回 :
作    者 :lx

创建时间 :17-11-14
*********************************************************/
static int Report_Stop_Complete(uint8 Reason,uint8 Result,uint8 Ch)
{
    uint8_t buf[8] ={0};
    
    
    buf[0] = Ch + 1;
    buf[1] = Reason;
    buf[2] = Result;
    
    API_Send_Single(4,ID_STOP_OVER,ADDR_COLLECT,ADDR_MY,buf,8); 

    return 0;    
}
/********************************************************
描    述 : 上报充电参数
输入参数 : 无

输出参数 :
返    回 :
作    者 :lx

创建时间 :17-11-14
*********************************************************/
static uint8 Report_Param(void)
{
    static uint8 Step = 0;
    static int Len = 0;
    
    if(Step ==0)
    {
        memset(MutliSendbuf,0,sizeof(MutliSendbuf));
        
        Len = Fill_Param_data(MutliSendbuf,0);
        
        API_Start_Send_Multi(6,ID_SET_TIME_RES,ADDR_COLLECT,ADDR_MY,0);
        
        Step = 1;
    }
    else if(Step == 1)
    {
        if(API_Send_Multi(MutliSendbuf,Len,0) == 2)
        {
            Step = 2;
        }
    }
    else
    {
        Step = 0;
    }
 
    return Step;
}


static void Ack_Set_Service_Status(uint8 *data,uint8 len)
{
    //SendFlag.Bit.Report_Param = 0;
}

/********************************************************
描    述 : Can协议收数据解析
输入参数 : 无

输出参数 :
返    回 :
作    者 :lx

创建时间 :17-11-14
*********************************************************/
static void Can_Parse_Data(uint8 PF,void *data,uint8 len)
{
    switch(PF)
    {
        case ID_START_CHARGE:
        {
            Ack_Start_Charge(data,len);
            break;
        }
        case ID_STOP_CHARGE:
        {
            Ack_Stop_Charge(data,len);
            break;
        }
        case ID_SET_TIME:
        {
            Ack_Set_time(data,len);
            break;
        }
        case ID_CHECK_VER:
        {
            Ack_Ver_Check(data,len);
            break;
        }
        default:break;
    }
}

/********************************************************
描    述 : C填充遥信数据
输入参数 : 无

输出参数 :
返    回 :
作    者 :lx

创建时间 :17-11-14
*********************************************************/
static int Fill_Yx_Data(uint8 *DataBuf,int Ch)
{
    uint8 Status  = Get_Charge_Ctrl_State(Ch);

	MEASURE_DATA MeterData = {0};
    ChargeRecord *pChargeRecord = Get_ChargeRecord(Ch);
    MeterData = Get_Measrue_Data(Ch);  
    int index = 0;
    DataBuf[index++] = Ch;
    
    memcpy(&DataBuf[index],(uint8*)&MeterData.VolageA,2);
    index +=2;
    memcpy(&DataBuf[index],(uint8*)&MeterData.VolageB,2);
    index +=2;
    memcpy(&DataBuf[index],(uint8*)&MeterData.VolageC,2);
    index +=2;
    
    memcpy(&DataBuf[index],(uint8*)&MeterData.CurrentA,2);
    index +=2;
    memcpy(&DataBuf[index],(uint8*)&MeterData.CurrentB,2);
    index +=2;
    memcpy(&DataBuf[index],(uint8*)&MeterData.CurrentC,2);
    index +=2;            
    //卡号
    memcpy(&DataBuf[index],(uint8*)pChargeRecord->CardNo,20);
    index += 20;
    //卡类型
    DataBuf[index++] = pChargeRecord->StartCardType;
    //电表读数
    memcpy(&DataBuf[index],(uint8*)MeterData.Totol_Energy,4);
    
    index +=4;
    
    return index;
    
}
/********************************************************
描    述 : 上报遥信
输入参数 : 无

输出参数 :
返    回 :
作    者 :lx

创建时间 :17-11-14
*********************************************************/
static uint8 Report_Yx_Data(void)
{
    static uint8 Step = 0;
    static int Len = 0;
    
    if(Step ==0)
    {
        memset(MutliSendbuf,0,sizeof(MutliSendbuf));
        
        Len = Fill_Yx_Data(MutliSendbuf,0);
        
        API_Start_Send_Multi(6,ID_SET_TIME_RES,ADDR_COLLECT,ADDR_MY,0);
        
        Step = 1;
    }
    else if(Step == 1)
    {
        if(API_Send_Multi(MutliSendbuf,Len,0) == 2)
        {
            Step = 2;
        }
    }
    else
    {
        Step = 0;
    }
 
    return Step;
    
}


/********************************************************
描    述 : 上报遥测
输入参数 : 无

输出参数 :
返    回 :
作    者 :lx

创建时间 :17-11-14
*********************************************************/
static uint8 Report_Yc_Data(int Ch)
{
    uint8 Sendbuf[8] = {0};
    uint64 AlarmBit = Get_FaultBit(Ch);
    memcpy(Sendbuf,(uint8*)&AlarmBit,8);
    API_Send_Single(6,ID_TEL_SIGN1,ADDR_COLLECT,ADDR_MY,Sendbuf,8); 

    return 1;
}
/********************************************************
描    述 : 上报心跳
输入参数 : 无

输出参数 :
返    回 :
作    者 :lx

创建时间 :17-11-14
*********************************************************/
static void Report_HeatBeat(void)
{
    
    uint8 Sendbuf[8] = {0};
    static uint8 Cnt = 0;
    
    Sendbuf[0] = 0;
    Sendbuf[1] = Cnt++;
    Sendbuf[2] = st_Can_Comm.CommStatus;
    Sendbuf[3] = st_Can_Comm.ChargeServerStatus; 
    
    API_Send_Single(4,ID_HEAT_BEAT1,ADDR_COLLECT,ADDR_MY,Sendbuf,8);
}

/********************************************************
描    述 : 停止完成检测发送
输入参数 : 无

输出参数 :
返    回 :
作    者 :lx

创建时间 :17-11-14
*********************************************************/
static void Report_Stop_Probe(char Ch)
{
    
    static char OldStep;
    
    char Step = Get_Charge_Ctrl_State(Ch) ;
    
    if(Step == STATE_IDLE && OldStep == STATE_END_CHARGE)
    {
        if(SendFlag.Bit.Report_STOP == 0)
        {
            Report_Stop_Complete(2,0,Ch);
        }
        else
        {
            SendFlag.Bit.Report_STOP = 0;
        }        
        
    }
    if(Step!= OldStep)
    {
        OldStep = Step;
    }
    
}
/********************************************************
描    述 : Can协议发送处理
输入参数 : 无

输出参数 :
返    回 :
作    者 :lx

创建时间 :17-11-14
*********************************************************/

static void Can_Send_Proc(void)
{
    static uint32 sendHeatTick;
    static uint32 sendYcTick;
    static char sendflag = 0;
    static uint32 sendMulitTick;
    static uint32 sendYxTick;
    
    if(Tick_Out(&sendHeatTick,1000))//发送心跳
    {
        Report_HeatBeat();
        Tick_Out(&sendHeatTick,00);
    }
    if(Tick_Out(&sendYcTick,1000))
    {//发送遥测
        Report_Yc_Data(0);
        Tick_Out(&sendYcTick,00);
    }
    if(Tick_Out(&sendYxTick,1000))
    {//发送遥信
        sendflag = 1;
        Tick_Out(&sendYxTick,00);
    }    
    if(Tick_Out(&sendMulitTick,100))
    {
        if(sendflag)
        {
            if(Report_Yx_Data() == 2)
            {
                sendflag = 0;
            }
        }
        Tick_Out(&sendMulitTick,00);
    }
    //发送停止完成
    Report_Stop_Probe(0);
}
/********************************************************
描    述 : Can协议收数据解析
输入参数 : 无

输出参数 :
返    回 :
作    者 :lx

创建时间 :17-11-14
*********************************************************/
static  int Can_Recv_Proc(void)
{
    uint8 PF = 0;
    uint8 PS = 0;
    CanRxMsgTypeDef RxMsg;
    static uint16 pakeNum[PORT_NUM] = {0};
    static uint16 rcvLen[PORT_NUM] = {0};
    static uint8 Rcvbuf[PORT_NUM][256]={0};
        
    if(BSP_Can_Read(&RxMsg))
    {
        PF = (RxMsg.ExtId >> 16 )&0XFF;
        PS = (RxMsg.ExtId >> 8 )&0XFF;
        if(PS != ADDR_MY)
        {
            if(PS != 0XFF)
            {
                return 1;
            }
        }
        
        if(PF == ID_START_CHARGE
            ||PF == ID_STOP_CHARGE
            ||PF == ID_SET_TIME
            ||PF == ID_CHECK_VER
            ||PF == ID_SET_SERVER
            ||PF == ID_CTRL_E_LOCK)
        {
            Can_Parse_Data(PF,RxMsg.Data,RxMsg.DLC);
        }
        else
        {
            if(API_Parse_Multi(Rcvbuf[0],RxMsg.ExtId,RxMsg.Data,RxMsg.DLC,0) == 3)
            {
                Can_Parse_Data(PF,Rcvbuf,RxMsg.DLC);
            }
        }
        
        return 1;
    }
    return 0;
}


//==========================================================================================================
// ****描述:	   Msg信息处理函数
// ****参数：    
// ****返回：   
//==========================================================================================================
int Can_Com_Msg_Proc(uint8_t Ch)
{
    uint8 type = 0;
	uint8 cardNo[20];
  
   	MSG_Def  pMsg;	   
    type = MsgQueue_Get(MSG_QUEUE_TASK_CLOUD_COMM,&pMsg);
	if(type == 0)
    return 0;
	Ch = pMsg.Ch + 1;
    switch(pMsg.type)
	{
		case MSG_CLOUD_AUTH_FIRST_CARD:
        {

            break;
        }
        case MSG_CLOUD_START_CRG_SUCCESS:
        {
            Report_Start_Complete(0,Ch);
            break;
        }
        case MSG_CLOUD_START_CRG_FAIL:
        {
            Report_Start_Complete(1,Ch);
            break;
        }
        default:break;
	
    }
    return 1;
    
}


/******************************************************************************
* Data Type Definitions
******************************************************************************/

void Can_Task(const void *p)
{
    
    BSP_Can_Init();
    
    ADDR_MY = Get_SlaverAddr();
    uint32 tick = 0;
    st_Can_Comm.CommStatus = 1;
    
    while(1)
    {
        
        Can_Send_Proc();  
        
        Can_Com_Msg_Proc(0);
        
        if(Can_Recv_Proc())
        {
            tick = HAL_GetTick();
            st_Can_Comm.CommStatus = 1;
        }
        else
        {
            if(abs(HAL_GetTick() - tick) > 3000)
            {
                st_Can_Comm.CommStatus = 0;
            }
            osDelay(5);
        }
    }
}

uint8 Get_Can_Comm_Statu(void)
{
   return st_Can_Comm.CommStatus;
}

void SetupCanCommApp(osPriority ThreadPriority,uint8_t StackNum)
{
	osThreadDef(ModBusThread, Can_Task, ThreadPriority, 0, StackNum*configMINIMAL_STACK_SIZE);
    osThreadCreate (osThread(ModBusThread), NULL); 
}

