
#ifndef _TSK_API_CARD_C
#define _TSK_API_CARD_C

/******************************************************************************
* Include Files
******************************************************************************/
#include "Tsk_API_Card.h"
#include "drv_usart.h"
#include "bsp_driver_uart.h"
#include <string.h>
#include "Utility.h"
#include "sys_Config.h"
#include <time.h>
#define M2_STX_H 0xAA
#define M2_STX_L 0xBB

#define M2_DEVICE_H 0x00
#define M2_DEVICE_L 0x00

#define CARD_PORT  COM5
#define DBG_CARD  1

#define BLOCK_INDEX_CARD_NO  4
#define BLOCK_INDEX_BANLANCE 5
#define BLOCK_INDEX_STATU    6


#define MAX_FAULT_TIME     20
#define INTERVAL   8
/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/

static uint8 g_Chargeing_CardNo[2][8];//充电过程中的卡号
static uint8 g_CommTick = 0;
static uint8 g_CommFault = 0;
/******************************************************************************
* Static Function Prototype Definitions
******************************************************************************/
/********************************************************
描    述 : M2协议封装发送数据帧函数

输入参数 : SendBuf:包缓冲去
            Cmd 操作命令
            pData 数据域
            Len 数据域 长度

输出参数 :无
返    回 :-1 发送出错  1 发送成功

作    者 :

创建时间 :18-06-07
*********************************************************/

static uint16 M2_Card_Pack(uint8 *SendBuf,uint16 Cmd,uint8 *pData,uint16 Len)
{
    int index = 0;
    uint8 Sum = 0;
    int i = 0;
    uint16 DataLen = 0;
    uint8 Buf[64] = {0};
    
    Buf[index++] = M2_STX_H;
    Buf[index++] = M2_STX_L;
    
    DataLen = Len + 5;

    Buf[index++] = DataLen;
    Buf[index++] = DataLen>>8;
    
    
    Buf[index++] = M2_DEVICE_H; 
    Buf[index++] = M2_DEVICE_L;
    
    Buf[index++] = Cmd >>8;
    Buf[index++] = Cmd;

    if(pData != NULL)
    {
        memcpy(Buf + index,pData,Len);
    }
    
    index += Len;
    Sum = DataXorCheck(Buf + 4,index - 4);
    Buf[index++] = Sum;

    index = 0;
    
    //转码
    memcpy(SendBuf + index,Buf,8);
    index += 8;
    for(i = 0;i < Len;i++)
    {
        if(Buf[i + 8] == 0xAA)
        {
            SendBuf[index++] = 0XAA;
            SendBuf[index++] = 0X00;
        }
        else
        {
            SendBuf[index++] = Buf[i + 8];
        }
    }
    
    SendBuf[index++]= Buf[8+Len];

    return index;
}
/********************************************************
描    述 : M2协议收数据解析
输入参数 : 无

输出参数 :pData 数据域
返    回 :TRUE:接收解析正确 
          FALSE :错误
作    者 :

创建时间 :18-06-07
*********************************************************/

static uint16 M2_Card_Parse(uint8 *pData,int timeout)
{
    uint32 ms = 0;
    uint8 FramLen = 0;
    uint8 DataLen = 0;
    uint8 rcvLen = 0;
    uint8 rcvBuf[64] = {0};
    uint8 DataBuf[64] = {0};
    int i = 0;
    uint8 Sum = 0;

    ms  = HAL_GetTick();
    
    do
    {
        rcvLen = Drv_Uart_Read(CARD_PORT,rcvBuf,64,100);

        if(rcvLen > 0)
        {
            for(i = 0;i < rcvLen;i++)
            {
                if(FramLen == 0)
                {
                    if(rcvBuf[i] == M2_STX_H)
                    {
                        DataBuf[FramLen++] = rcvBuf[i];
                        continue;
                    }
                }
                if(FramLen == 1)
                {
                    if(rcvBuf[i] == M2_STX_L)
                    {
                        DataBuf[FramLen++] = rcvBuf[i];
                        continue;
                    }
                }
                if((FramLen >= 2)&&(FramLen<=3))
                {
                    DataBuf[FramLen++] = rcvBuf[i];
                    continue;
                }
               
                if(FramLen >= 3)
                {
                    int i = 3;
                    for(i = 4;i< rcvLen;i++)
                    {
                        if(rcvBuf[i] == M2_STX_H && rcvBuf[i + 1] == 00)
                        {
                           DataBuf[FramLen++] = rcvBuf[i];
                           i += 1;
                        }
                        else
                        {
                            DataBuf[FramLen++] = rcvBuf[i];
                        }
                    }
                    
                    DataLen = DataBuf[2]  + (DataBuf[3]<< 8);
                    //
                    if(FramLen == DataLen + 4)
                    {
                        Sum = DataXorCheck(DataBuf + 4,DataLen - 1);

                        if(Sum == DataBuf[FramLen - 1])
                        {
                            memcpy(pData,DataBuf + 8,DataLen - 5);
                            g_CommTick = 0;     //clear
                            g_CommFault = FALSE;//clear
                            return TRUE;
                        }

                        return FALSE;
                    }
                    else
                    {
                        if(FramLen > sizeof(DataBuf))
                        {
                            FramLen = 0;
                        }
                    }
                }
                    
               
                if(FramLen > sizeof(DataBuf))
                {
                    FramLen = 0;
                }
            }
            
        }
        else
        {
            osDelay(5);
        }
               
    }while(HAL_GetTick() - ms < timeout);
    
    if(g_CommTick > MAX_FAULT_TIME)
    {
        g_CommTick = MAX_FAULT_TIME;
        g_CommFault = TRUE;
    }
    else
    {
        g_CommFault = FALSE;
    }
    g_CommTick++;
    return FALSE;
}
static char Last = 0;//用于检测一直刷卡就不动作
/********************************************************
描    述 : M2协议寻卡操作
输入参数 : 无

输出参数 :cardType 卡类型
返    回 :TRUE:接收解析正确 
          FALSE :错误
作    者 :

创建时间 :18-06-07
*********************************************************/

static uint8 M2_Find_Card(uint16 *cardType)
{
    uint8 SendBuf[32] = {0};
    uint8 Data[16] = {0};
    uint8 RcvData[16] = {0};
    int len = 0;
	
	static uint32 tick = 0;
    static uint32 last_tick = 0;
    if(Tick_Out(&tick,300))
    {
        Tick_Out(&tick,0);
    }
    else
    {
        return FIND_CARD_ERR;
    }
    
    Data[0] = 0x52;
    
    len = M2_Card_Pack(SendBuf,CMD_OPRATION_FIND_CARD,Data,1);
    Drv_Uart_Write(CARD_PORT,(const uint8 *)SendBuf,len,1);
    if(M2_Card_Parse(RcvData,40) == TRUE)
    {
        if(RcvData[0] == 0 && Last == 0)
        {
            if(cardType != NULL)
            {//参数检测
                memcpy(cardType,RcvData + 1,2);
            }
            
			//Last = 1;
            last_tick = time(NULL);
            return FIND_CARD_OK;
        }
		else
		{
            if(RcvData[0] != 0 )
            {
                Last = 0;
            }
            if(time(NULL)- last_tick > INTERVAL)
            {
                Last = 0;
            }
		}
        //dbg_printf(DBG_CARD,"no card");
        return FIND_CARD_NONE;
    }   
    return FIND_CARD_ERR;  
}

/********************************************************
描    述 : M2协议冲突检测
输入参数 : 无

输出参数 :cardID 卡ID号
返    回 :TRUE:接收解析正确 
          FALSE :错误
作    者 :

创建时间 :18-06-07
*********************************************************/

static uint8 M2_Confilict_Opt(uint8 *cardID)
{
    uint8 SendBuf[32] = {0};
    uint8 Data[16] = {0};
    uint8 RcvData[16] = {0};
    int len = 0;
    Data[0] = 0x04;
    
    len = M2_Card_Pack(SendBuf,CMD_OPRATION_PREVENT_CONFLICT,Data,1);
    Drv_Uart_Write(CARD_PORT,(const uint8 *)SendBuf,len,1);
    if(M2_Card_Parse(RcvData,400) == TRUE)
    {
        if(RcvData[0] == 0)
        {
            memcpy(cardID,RcvData + 1,4);
            return TRUE;
        }
    }
    dbg_printf(DBG_CARD,"Confilict err");
    return FALSE;
}

/********************************************************
描    述 : M2协议冲突检测
输入参数 : 无

输出参数 :cardID 卡ID号
返    回 :TRUE:接收解析正确 
          FALSE :错误
作    者 :

创建时间 :18-06-07
*********************************************************/

uint8 M2_Set_ReadWrite_Mode(void)
{
    uint8 SendBuf[32] = {0};
    uint8 Data[16] = {0};
    uint8 RcvData[16] = {0};
    int len = 0;
    Data[0] = 0x01;
    
    len = M2_Card_Pack(SendBuf,CMD_OPRATION_READ_WRITE,Data,1);
    Drv_Uart_Write(CARD_PORT,(const uint8 *)SendBuf,len,1);
    if(M2_Card_Parse(RcvData,400) == TRUE)
    {
        if(RcvData[0] == 0)
        {
            return TRUE;
        }
        
    }
    return FALSE;
}


/********************************************************
描    述 : M2协议冲突检测
输入参数 : 无

输出参数 :cardID 卡ID号
返    回 :TRUE:接收解析正确 
          FALSE :错误
作    者 :

创建时间 :18-06-07
*********************************************************/

static uint8 M2_Choice_Opt(uint8 *cardID)
{
    uint8 SendBuf[32] = {0};

    uint8 RcvData[16] = {0};
    int len = 0;
    
    
    len = M2_Card_Pack(SendBuf,CMD_OPRATION_CHOICE_CARD,cardID,4);
    Drv_Uart_Write(CARD_PORT,(const uint8 *)SendBuf,len,1);
    if(M2_Card_Parse(RcvData,400) == TRUE)
    {
        if(RcvData[0] == 0)
        {
            
            return TRUE;
        }
    }
    dbg_printf(DBG_CARD,"Choice card err");
    return FALSE;
}
/********************************************************
描    述 : M2协议验证密码
输入参数 : key 密码
           block 块号

输出参数 :cardID 卡ID号
返    回 :TRUE:接收解析正确 
          FALSE :错误
作    者 :

创建时间 :18-06-07
*********************************************************/

static uint8 M2_Auth_Opt(uint8 *key,int block)
{
    uint8 SendBuf[32] = {0};
    uint8 Data[16] = {0};
    uint8 RcvData[16] = {0};
    int len = 0;
    
    Data[1] = block;
    Data[0] = 0x60;
    
    memcpy(Data + 2,key,6);
    
    len = M2_Card_Pack(SendBuf,CMD_OPRATION_AUTHENTICATE_PASSWORD,Data,8);
    Drv_Uart_Write(CARD_PORT,(const uint8 *)SendBuf,len,1);
    if(M2_Card_Parse(RcvData,400) == TRUE)
    {
        if(RcvData[0] == 0)
        {
            
            return TRUE;
        }          
    }
    dbg_printf(DBG_CARD,"Auth key err");
    return FALSE;
}


/********************************************************
描    述 : M2协议读卡操作
输入参数 : key 密码
           block 块号

输出参数 :cardID 卡ID号
返    回 :TRUE:接收解析正确 
          FALSE :错误
作    者 :

创建时间 :18-06-07
*********************************************************/

static uint8 M2_Read_Opt(uint8 *pData,int block)
{
    uint8 SendBuf[32] = {0};
    uint8 Data[16] = {0};
    uint8 RcvData[16] = {0};
    int len = 0;
    
    Data[0] = block; 
    
    len = M2_Card_Pack(SendBuf,CMD_OPRATION_READ_CARD,Data,1);
    Drv_Uart_Write(CARD_PORT,(const uint8 *)SendBuf,len,1);
    if(M2_Card_Parse(RcvData,400) == TRUE)
    {
        if(RcvData[0] == 0)
        {
            memcpy(pData,RcvData + 1,16);
            
            return TRUE;
        }       
    }
    dbg_printf(DBG_CARD,"read card err");
    return FALSE;
}


/********************************************************
描    述 : M2协议读卡操作
输入参数 : key 密码
           block 块号

输出参数 :cardID 卡ID号
返    回 :TRUE:接收解析正确 
          FALSE :错误
作    者 :

创建时间 :18-06-07
*********************************************************/

static uint8 M2_Write_Opt(uint8 *pData,int block)
{
    uint8 SendBuf[32] = {0};
    uint8 Data[20] = {0};
    uint8 RcvData[16] = {0};
    int len = 0;
    
    Data[0] = block; 
    memcpy(Data + 1,pData,16);
    len = M2_Card_Pack(SendBuf,CMD_OPRATION_WRITE_CARD,Data,17);
    Drv_Uart_Write(CARD_PORT,(const uint8 *)SendBuf,len,1);
    if(M2_Card_Parse(RcvData,400) == TRUE)
    {
        if(RcvData[0] == 0)
        {
            if(M2_Read_Opt(RcvData,block))
            {
                if(memcmp(pData,RcvData,16) == 0)
                {
                    return TRUE;
                }
            }            
            return FALSE;
        }        
    }
    dbg_printf(DBG_CARD,"write card err");
    return FALSE;
}

/********************************************************
描    述 : M2协议初始化值操作
输入参数 : key 密码
           block 块号

输出参数 :cardID 卡ID号
返    回 :TRUE:接收解析正确 
          FALSE :错误
作    者 :

创建时间 :18-06-07
*********************************************************/

uint8 M2_Init_Block(uint32 iData,int block)
{
    uint8 SendBuf[32] = {0};
    uint8 Data[16] = {0};
    uint8 RcvData[16] = {0};
    int len = 0;
    
    Data[0] = block; 
    memcpy(Data + 1,(uint8*)&iData,4);
    len = M2_Card_Pack(SendBuf,CMD_OPRATION_INIT,Data,5);
    Drv_Uart_Write(CARD_PORT,(const uint8 *)SendBuf,len,1);
    if(M2_Card_Parse(RcvData,400) == TRUE)
    {
        if(RcvData[0] == 0)
        {
            
            return TRUE;
        }
        
    }
    return FALSE;
}

/********************************************************
描    述 : M2协议减值操作
输入参数 : key 密码
           block 块号

输出参数 :cardID 卡ID号
返    回 :TRUE:接收解析正确 
          FALSE :错误
作    者 :

创建时间 :18-06-07
*********************************************************/

static uint8 M2_Dec_Value(uint32 iData,int block)
{
    uint8 SendBuf[32] = {0};
    uint8 Data[16] = {0};
    uint8 RcvData[16] = {0};
    int len = 0;
    
    Data[0] = block; 
    memcpy(Data + 1,(uint8*)&iData,4);
    len = M2_Card_Pack(SendBuf,CMD_OPRATION_DEC_VAL,Data,5);
    Drv_Uart_Write(CARD_PORT,(const uint8 *)SendBuf,len,1);
    if(M2_Card_Parse(RcvData,400) == TRUE)
    {
        if(RcvData[0] == 0)
        {
            
            return TRUE;
        }       
    }
    dbg_printf(DBG_CARD,"dec val err");
    return FALSE;
}


/********************************************************
描    述 : M2协议减值操作
输入参数 : key 密码
           block 块号

输出参数 :cardID 卡ID号
返    回 :TRUE:接收解析正确 
          FALSE :错误
作    者 :

创建时间 :18-06-07
*********************************************************/

uint8 M2_Add_Value(uint32 iData,int block)
{
    uint8 SendBuf[32] = {0};
    uint8 Data[16] = {0};
    uint8 RcvData[16] = {0};
    int len = 0;
    
    Data[0] = block; 
    memcpy(Data + 1,(uint8*)&iData,4);
    len = M2_Card_Pack(SendBuf,CMD_OPRATION_DEC_VAL,Data,5);
    Drv_Uart_Write(CARD_PORT,(const uint8 *)SendBuf,len,1);
    if(M2_Card_Parse(RcvData,400) == TRUE)
    {
        if(RcvData[0] == 0)
        {
            
            return TRUE;
        }
        
    }
    return FALSE;
}

/********************************************************
描    述 : M2协议减值操作
输入参数 : key 密码
           block 块号

输出参数 :cardID 卡ID号
返    回 :TRUE:接收解析正确 
          FALSE :错误
作    者 :

创建时间 :18-06-07
*********************************************************/

static uint8 M2_Read_Value(int *iData,int block)
{
    uint8 SendBuf[32] = {0};
    uint8 Data[16] = {0};
    uint8 RcvData[16] = {0};
    int len = 0;
    
    Data[0] = block; 
    
    len = M2_Card_Pack(SendBuf,CMD_OPRATION_READING,Data,1);
    Drv_Uart_Write(CARD_PORT,(const uint8 *)SendBuf,len,1);
    if(M2_Card_Parse(RcvData,400) == TRUE)
    {
        if(RcvData[0] == 0)
        {
            memcpy((uint8*)iData,RcvData + 1,4);
            return TRUE;
        }       
    }
    dbg_printf(DBG_CARD,"read val err");
    return FALSE;
}


/********************************************************
描    述 : M2协议一键读
输入参数 : key 密码
           block 块号

输出参数 :cardID 卡ID号
返    回 :TRUE:接收解析正确 
          FALSE :错误
作    者 :

创建时间 :18-06-07
*********************************************************/

static uint8 M2_Fast_Read(int block,uint8 *Key,uint8 *pData)
{
    uint8 SendBuf[32] = {0};
    uint8 Data[16] = {0};
    uint8 RcvData[16] = {0};
    int len = 0;

    Data[0] = 0x60; 
    Data[1] = block; 
    memcpy(Data + 2,Key,6);
    
    len = M2_Card_Pack(SendBuf,CMD_OPRATION_FAST_READ,Data,8);
    Drv_Uart_Write(CARD_PORT,(const uint8 *)SendBuf,len,1);
    if(M2_Card_Parse(RcvData,400) == TRUE)
    {
        if(RcvData[0] == 0)
        {
            if(pData != NULL)
            {
                memcpy(pData,RcvData + 1,16);
            }
            
            return TRUE;
        }
        
    }
    
    return FALSE;
}


/********************************************************
描    述 : M2协议读卡号
输入参数 : 

输出参数 :PCardData  卡数据
返    回 :TRUE:接收解析正确 
          FALSE :错误
作    者 :

创建时间 :18-06-07
*********************************************************/

uint8 M2_Search_Card(CARD_DATA *PCardData)
{
   
    uint8 Key[6] = {0};

    uint8 RcvData[16] = {0};

    Get_Card_Key(Key);

    if(M2_Find_Card(NULL) == FIND_CARD_OK)
    {
        if(M2_Fast_Read(BLOCK_INDEX_CARD_NO,Key,RcvData) == TRUE)
        {
            memcpy(PCardData->Card_no,RcvData,8);
            PCardData->CardType = RcvData[9];
            
            return TRUE;
        }
    }

    return FALSE;
}
/********************************************************
描    述 : M2d第一次充电刷卡操作
输入参数 : Ch ,那个通道

输出参数 :PCardData  卡数据
返    回 : START_CRG_OPT_CARD类型
    
作    者 :

创建时间 :18-06-07
*********************************************************/

START_CRG_OPT_CARD M2_Start_Charge_Card(CARD_DATA *PCardData,uint8 Ch)
{
    uint8 CardType = 0;
    uint8 Key[6] = {0};
    int Balance = 0;//分负数和正数
    START_CRG_OPT_CARD ret = (START_CRG_OPT_CARD)0;
    
    uint8 RcvData[16] = {0};
    uint8 Id[4] = {0};
    ret = OPT_ERR;
    Get_Card_Key(Key);
    
    if(M2_Find_Card(NULL) == FIND_CARD_OK)
    {
        if(M2_Confilict_Opt(Id))
        {
            if(M2_Choice_Opt(Id))
            {
                if(M2_Auth_Opt(Key,BLOCK_INDEX_BANLANCE))
                {
                                            
                    if(M2_Read_Opt(RcvData,BLOCK_INDEX_CARD_NO) == TRUE)
                    {
                        memcpy(PCardData->Card_no,RcvData,8);
                        CardType = RcvData[8];
                        PCardData->CardType = CardType;
                        if(CardType  == BUSINESS_CARD)
                        {//一般用户卡
                            if(M2_Read_Opt(RcvData,BLOCK_INDEX_STATU) == TRUE)
                            {//读卡状态
                                if(RcvData[0] == STATUS_LOCK)
                                {
                                    ret =  OPT_LOCK;
                                }
                                else
                                {                        
                                    
                                    if(M2_Read_Value(&Balance,BLOCK_INDEX_BANLANCE) == TRUE)
                                    {               
                                        if(Balance >= MIN_ALLOW_MONEY)
                                        {
                                            RcvData[0] = STATUS_LOCK;
                                            
                                            M2_Write_Opt(RcvData,BLOCK_INDEX_STATU);
                                            {
                                                PCardData->Balance = Balance;
                                                ret =   OPT_OK; 
                                            }
                                        }
                                        else
                                        {
                                            ret =   OPT_BALANCE_LACK;
                                        }                             
                                    }                                    

                                }  
                            }
                        }
                        else if(CardType == SUPER_MANAGE)
                        {//超级管理卡
                            ret =   OPT_OK;
                        }
                        else if(CardType == USR_CARD)
                        {//一般管理卡
                            
                            if(M2_Read_Value(&Balance,BLOCK_INDEX_BANLANCE) == TRUE)
                            {
                                if(Balance >= MIN_ALLOW_MONEY)
                                {
                                    PCardData->Balance = Balance;

                                    ret =   OPT_OK;
                                }
                                else
                                {
                                    return OPT_BALANCE_LACK;
                                }
                                
                            }                
                        }
                        else if(CardType == ONLINE_CARD)
                        {//在线卡
                            ret =   OPT_OK;
                        }          
                    }
                }
            }
        }
    }
    if(OPT_OK == ret)
    {
        memcpy(g_Chargeing_CardNo[Ch],PCardData->Card_no,8);
        Last = 1;
    }
    
    return ret;
}



/********************************************************
描    述 : M2协议补充交易寻卡操作
输入参数 : 无

输出参数 :cardType 卡类型
返    回 :TRUE:接收解析正确 
          FALSE :错误
作    者 :

创建时间 :18-06-07
*********************************************************/

static uint8 M2_Find_Card_Add(uint16 *cardType)
{
    uint8 SendBuf[32] = {0};
    uint8 Data[16] = {0};
    uint8 RcvData[16] = {0};
    int len = 0;

    Data[0] = 0x52;
    
    len = M2_Card_Pack(SendBuf,CMD_OPRATION_FIND_CARD,Data,1);
    Drv_Uart_Write(CARD_PORT,(const uint8 *)SendBuf,len,1);
    if(M2_Card_Parse(RcvData,50) == TRUE)
    {
        if(RcvData[0] == 0 )
        {
            if(cardType != NULL)
            {//参数检测
                memcpy(cardType,RcvData + 1,2);
            }

            return FIND_CARD_OK;
        }

        return FIND_CARD_NONE;
    }   
    return FIND_CARD_ERR;  
}

/********************************************************
描    述 : M2第二次结束充电刷卡操作
输入参数 : Ch ,那个通道
            Record 写入到卡里面的记录数据
输出参数 :无
返    回 : END_OPT_CARD类型
    
作    者 :

创建时间 :18-06-07
*********************************************************/

END_OPT_CARD M2_End_Charge_Card(CARD_RECORD Record,int Ch)
{
    uint8 CardType = 0;
    uint8 Key[6] = {0};
    
    uint8 CardNo[8] = {0};
    uint8 RcvData[16] = {0};
    END_OPT_CARD ret = END_ERR;
    uint8 Id[4] = {0};
    Get_Card_Key(Key);
    
    if(M2_Find_Card(NULL) == FIND_CARD_OK)
    {
        if(M2_Confilict_Opt(Id))
        {
            if(M2_Choice_Opt(Id))
            {
                if(M2_Auth_Opt(Key,BLOCK_INDEX_BANLANCE))
                {
                    
                    if(M2_Read_Opt(RcvData,BLOCK_INDEX_CARD_NO) == TRUE)
                    {
                        memcpy(CardNo,RcvData,8);
                        CardType = RcvData[8];
                        if(CardType == SUPER_MANAGE)
                        {//超级管理卡
                            
                            ret = END_OK;
                        }
                        else if(Record.type == CardType && CardType == ONLINE_CARD)
                        {
                            ret = END_OK;
                        }
                        else if(CardType == USR_CARD || CardType  == BUSINESS_CARD)
                        {//一般管理卡
                            
                            if(memcmp(g_Chargeing_CardNo[Ch],CardNo,8) == 0)
                            {//卡号匹配
                                
                                //add            
                                RcvData[0] = STATUS_UNLOCK; 
                                memcpy(RcvData + 1,(uint8*)&Record,sizeof(CARD_RECORD));
                                
                                if(M2_Write_Opt(RcvData,BLOCK_INDEX_STATU))
                                {
                                    if(Record.Money <= 0)
                                    {
                                        ret = END_OK;
                                    }
                                    else if(M2_Dec_Value(Record.Money,BLOCK_INDEX_BANLANCE) == TRUE)
                                    {
                                        ret = END_OK;
                                        
                                    }
                                    else
                                    {
                                        ret = END_ERR;
                                    }                                    
                                }
                                else
                                {
                                    ret = END_ERR;
                                }                                
                            }
                            else
                            {
                                return END_NOT_MATCH;
                            }               
                        }
                        else if(Record.type != CardType)
                        {
                            return END_NOT_MATCH;
                        }
                    }
                }
               
            }
        }
    }
    if(ret == END_OK)
    {
        Last = 1;
    }
    
    return ret;
}

/********************************************************
描    述 : M2第二次补扣充电刷卡操作
输入参数 : Ch ,那个通道
            Record 写入到卡里面的记录数据
输出参数 :无
返    回 : END_OPT_CARD类型
    
作    者 :

创建时间 :18-06-07
*********************************************************/

END_OPT_CARD M2_Add_Charge_Card(CARD_RECORD Record,int Ch)
{
    uint8 CardType = 0;
    uint8 Key[6] = {0};
    
    uint8 CardNo[8] = {0};
    uint8 RcvData[16] = {0};
    END_OPT_CARD ret = END_ERR;
    uint8 Id[4] = {0};
    Get_Card_Key(Key);
    if(M2_Find_Card_Add(NULL) == FIND_CARD_OK)
    {
        if(M2_Confilict_Opt(Id))
        {
            if(M2_Choice_Opt(Id))
            {
                if(M2_Auth_Opt(Key,BLOCK_INDEX_BANLANCE))
                {
                    
                    if(M2_Read_Opt(RcvData,BLOCK_INDEX_CARD_NO) == TRUE)
                    {
                        memcpy(CardNo,RcvData,8);
                        CardType = RcvData[8];
                        if(CardType == USR_CARD || CardType  == BUSINESS_CARD)
                        {//一般管理卡     
                            RcvData[0] = STATUS_UNLOCK; 
                            memcpy(RcvData + 1,(uint8*)&Record,sizeof(CARD_RECORD));
                            
                            if(M2_Write_Opt(RcvData,BLOCK_INDEX_STATU))                            
                            {//卡号匹配
                                if(Record.Money == 0)
                                {
                                    ret = END_OK;
                                }
                                else if(M2_Dec_Value(Record.Money,BLOCK_INDEX_BANLANCE) == TRUE)
                                {
                                    ret = END_OK;
                                    
                                }
                            }            
                        }
                    }
                }
               
            }
        }
    }    

    return ret;
}

uint8 Get_CardCommStatu_M2(void)
{
    return (g_CommFault == FALSE )? TRUE:FALSE;
}

uint8 Set_Charge_Card_No_M2(uint8 *CardNo,uint8 Ch)
{
    
    asc2hex(g_Chargeing_CardNo[Ch],CardNo,16);

    return 0;
}
void M2_Card_Read_init(void)
{
    M2_Set_ReadWrite_Mode();    
}

#endif
