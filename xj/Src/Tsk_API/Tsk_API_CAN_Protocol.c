

/******************************************************************************
* Includes
******************************************************************************/
#include "Tsk_API_CAN_Protocol.h"
#include "bsp_can.h"
#include "Sys_Config.h"
#include <string.h>
#include <stdio.h>


/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/

static uint32 g_Multi_ID[PORT_NUM] = {0};
static uint8 g_Multi_Step[PORT_NUM] = {0};
static uint8 g_Parse_Multi_Step[PORT_NUM] = {0};

#define STEP_START 0
#define STEP_ING   1
#define STEP_END   2

/******************************************************************************
* Data Type Definitions
******************************************************************************/



/******************************************************************************
* Global Variable Definitions
******************************************************************************/

static uint16 Cal_Check_Sum(uint8 *Data,uint16 len)
{
    uint16 Sum = 0;
    
    int i = 0;
    
    for(i = 0; i < len;i++)
    {
        Sum += Data[i];
    }
    
    return Sum;
}

uint8 API_Send_Single(uint8 P,uint8 PF,uint8 PS,uint8 SA,uint8 *Data,uint8 Len)
{
    uint32 ID = 0;
    
    CanTxMsgTypeDef TxMsg ;
    ID = SA;
    ID+= (PS<<8);
    ID+= (PF <<16);
    ID+= (P & 0x7)<<23;
    TxMsg.DLC = Len;
    TxMsg.ExtId = ID;
    TxMsg.IDE = CAN_ID_EXT;
    TxMsg.RTR = CAN_RTR_DATA;
    memcpy(TxMsg.Data,Data,8);
    
    return BSP_Can_write(TxMsg);     
    
}
/*------------------------------------------------------------
操作目的：   开始发送 多包
作    者：      
操作结果：   
函数参数：P 优先级 ，PF 类型，PS 源地址 SA目的地址， Ch通道
            
返回值：
            无
版   本:    V1.0

记   录:

------------------------------------------------------------*/
void API_Start_Send_Multi(uint8 P,uint8 PF,uint8 PS,uint8 SA,int Ch)
{
    g_Multi_Step[Ch] = STEP_START;

    g_Multi_ID[Ch] = 0;

    g_Multi_ID[Ch] = SA;
    g_Multi_ID[Ch]+= (PS<<8);
    g_Multi_ID[Ch]+= (PF <<16);
    g_Multi_ID[Ch]+= (P & 0x7)<<23;
}

/*------------------------------------------------------------
操作目的：   发送 多包 需要周期调用
作    者：      
操作结果：   
函数参数：Data 数据 ，Len长度，Ch通道
            
返回值：
            STEP_START 开始发送第一包
            STEP_ING 正在发送
            STEP_END 发送结束 可以发送下一包
版   本:    V1.0

记   录:

------------------------------------------------------------*/
uint8 API_Send_Multi(uint8 *Data,uint16 Len,int Ch)
{
    static uint8 SN[1] = {0};    //当前序列号
    uint8 Total = 0;            //总包数

    uint32 ID = 0;              //发送ID
    int sendLen = 0;            //发送长度
    
    
    CanTxMsgTypeDef TxMsg ;     //can 发送数据
    ID = g_Multi_ID[Ch];        // 
    
    TxMsg.ExtId = ID;
    TxMsg.IDE = CAN_ID_EXT;
    TxMsg.RTR = CAN_RTR_DATA;
    
    Total = (Len + 5) / 7; //5 = 1byte总帧数 + 2byte 总长度 + 2byte 校验
    
    if(((Len + 5) % 7) != 0)
    {
       Total += 1; 
    }
    
    switch(g_Multi_Step[Ch])
    {
        case STEP_START:
        {
            sendLen = 8;
            SN[Ch] = 1;
            TxMsg.Data[0] = SN[Ch];
            TxMsg.Data[1] = Total;
            TxMsg.Data[2] = Len;
            TxMsg.Data[3] = (Len >> 8);
            TxMsg.DLC = sendLen;
            memcpy(TxMsg.Data + 4,Data,4);
            g_Multi_Step[Ch] = STEP_ING;
            
            BSP_Can_write(TxMsg);
            
            break;
        }
        case STEP_ING:
        {
            int index = 0;

            uint16 Sum = Cal_Check_Sum(Data,Len);
            uint16 Remain = 0;//(Len + 5) - (SN[Ch] * 7);
            uint16 Pos = 1;
    
            if(SN[Ch] >= 1)
            {
                index = (SN[Ch] - 1) * 7 + 4;         
                Remain = (Len + 5) - (SN[Ch]) * 7;
            }

            SN[Ch]++;
            TxMsg.Data[0] = SN[Ch];
            
            if(Remain == 1)
            {              
                
                TxMsg.Data[Pos ++] = (Sum >>8);
                sendLen = Pos;
                TxMsg.DLC = sendLen;
                
            }
            else if(Remain == 2)
            {
                TxMsg.Data[Pos ++] = Sum;
                TxMsg.Data[Pos ++] = (Sum >>8);
                sendLen = Pos;
                TxMsg.DLC = sendLen;
                            
            }
            else if(Remain == 8)
            {
                memcpy(TxMsg.Data + Pos,Data+ index,6);
                Pos += 6;
                
                TxMsg.Data[Pos ++] = Sum;
                sendLen = Pos;
                TxMsg.DLC = sendLen;            
            }
            else if(Remain < 8)
            {
                
                memcpy(TxMsg.Data + Pos,Data+ index,Remain - 2);
                Pos += (Remain - 2);
                
                TxMsg.Data[Pos ++] = Sum;
                TxMsg.Data[Pos ++] = (Sum >>8);
                sendLen = Pos;
                TxMsg.DLC = sendLen;
                                 
            }
            else
            {
                memcpy(TxMsg.Data + Pos,Data+ index,7);
                TxMsg.DLC = 8;                
            }
            
            if(SN[Ch] >= Total)
            {
                g_Multi_Step[Ch] = STEP_END;
            }

            BSP_Can_write(TxMsg);
            break;
        }
        case STEP_END:
        {
            break;
        }
    }   
    return g_Multi_Step[Ch];
}

/*------------------------------------------------------------
操作目的：   
作    者： 
操作结果：   
函数参数：
            
返回值：
            无

版   本:    V1.0

记   录:

------------------------------------------------------------*/
uint8 API_Parse_Multi(uint8 *RcvBuf,uint32 ID,uint8 *Data,uint16 Len,int Ch)
{
    static uint32 OldId = 0;
    static char TotlPake = 0;
    static uint16 TotlLen = 0;
    static char RcvPake = 0;
    static uint16 RclLen = 0;
    uint16 Sum = 0;
    
    if(OldId != ID || (g_Parse_Multi_Step[Ch] == 0)
        || g_Parse_Multi_Step[Ch] == 3)
    {
        if(Data[0] == 1)
        {
            g_Parse_Multi_Step[Ch] = 1;
            OldId = ID;
         
        }
        else
        {
            g_Parse_Multi_Step[Ch] = 0;
            OldId = 0;
            TotlPake = 0;
            TotlLen = 0;
            RcvPake = 0;
            RclLen = 0;
            Sum = 0;
        }
    }
    if(g_Parse_Multi_Step[Ch] == 1)
    {
        
        TotlPake = Data[1];
        RcvPake = 1;
        TotlLen = Data[2] + Data[3] * 256 + 2;//2 byte sum
        memcpy(RcvBuf,Data + 4,4);
        RclLen = 4;
        g_Parse_Multi_Step[Ch] = 2;           
    }
    else if(g_Parse_Multi_Step[Ch] == 2)
    {
        if(Data[0] !=  RcvPake + 1)
        {
            g_Parse_Multi_Step[Ch] = 0;
            OldId = 0;
            TotlPake = 0;
            TotlLen = 0;
            RcvPake = 0;
            RclLen = 0;
            Sum = 0;            
        }
        else
        {
            RcvPake = Data[0];
            if(RcvPake != TotlPake)
            {
                memcpy(RcvBuf + RclLen,Data + 1,7);
                RclLen += 7;
            }
            else
            {
                char len = TotlLen - RclLen;
                
                if(len < 7 && len >0)
                {                    
                    memcpy(RcvBuf + RclLen,Data + 1,len);
                    RclLen += len;
                    Sum = RcvBuf[RclLen - 2] + RcvBuf[RclLen - 1] * 256;
                    if(Sum == Cal_Check_Sum(RcvBuf,RclLen - 2))
                    {
                        g_Parse_Multi_Step[Ch] = 3;
                        OldId = 0;
                        TotlPake = 0;
                        TotlLen = 0;
                        RcvPake = 0;
                        RclLen = 0;
                        Sum = 0;  
                    }
                    else
                    {
                        g_Parse_Multi_Step[Ch] = 0;
                        OldId = 0;
                        TotlPake = 0;
                        TotlLen = 0;
                        RcvPake = 0;
                        RclLen = 0;
                        Sum = 0;                          
                    }
                }
                else
                {
                    g_Parse_Multi_Step[Ch] = 0;
                    OldId = 0;
                    TotlPake = 0;
                    TotlLen = 0;
                    RcvPake = 0;
                    RclLen = 0;
                    Sum = 0;                      
                }
                
            }
        }
    }
    
    return g_Parse_Multi_Step[Ch];
}

/*------------------------------------------------------------
操作目的：   
作    者：      
操作结果：   
函数参数：
            
返回值：
            无

版   本:    V1.0

记   录:

------------------------------------------------------------*/


