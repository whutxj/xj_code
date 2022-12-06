

/******************************************************************************
* Includes
******************************************************************************/
#include "Tsk_API_Meter.h"
#include "drv_usart.h"
#include "bsp_driver_uart.h"
#include "Sys_Config.h"
#include <string.h>
#include <stdio.h>

/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/
#define METER_COM  COM2
#define METER_BOAD_RATE  2400
#define CT 1    //变比
#define GUN_NUM  (Get_DoubelGunEn() + 1)

#define ENERGE_DI  0x00000000

#define VOLT_A_DI  0x02010100
#define VOLT_B_DI  0x02010200
#define VOLT_C_DI  0x02010300
#define CUR_A_DI   0x02020100
#define CUR_B_DI   0x02020200
#define CUR_C_DI   0x02020300
#define CMD_READ        0x11
#define CMD_READ_ADDR   0x13
#define MAX_TIME_OUT    1000 //ms

#define GW_Authenticate    0 // 国网认证电表变比 15
/******************************************************************************
* Data Type Definitions
******************************************************************************/

typedef struct
{
    uint8 Meter_addr[6];
    uint8 Meter_CommStatus;

}Meter_Comm_Data;

/******************************************************************************
* Global Variable Definitions
******************************************************************************/
MEASURE_DATA g_Meate_Data[2];
Meter_Comm_Data g_Meter_Comm_Data[2] = {0};

/*------------------------------------------------------------
操作目的：   dlt-2007协议封装数据包
作    者：      
操作结果：   
函数参数：type 记录参数类别
            
返回值：
            无

版   本:    V1.0

记   录:

------------------------------------------------------------*/
static uint8 Meter_Pack(uint8 *Buf,uint32 DI,uint8 Cmd,const uint8 *data,int Datalen,int Ch)
{
    int index = 0;
    char Sum = 0;
    int i = 0;

    
    Buf[index++] = 0x68;
    
    memcpy(Buf + index,g_Meter_Comm_Data[Ch].Meter_addr,6);
    index += 6;
    Buf[index++] = 0x68;
    Buf[index++] = Cmd;

    if(Cmd != CMD_READ_ADDR)
    {
        Buf[index++] = 4 + Datalen;//数据域长度

        memcpy(Buf + index,&DI,4);
        index += 4;
        if(data != NULL)
        {
            memcpy(Buf + index,data,Datalen);
        }

        index+= Datalen;
    }
    else
    {
        Buf[index++] = 0;//数据域长度
    }

    for(i = 0; i < Datalen + 4;i++)
    {
        Buf[i + 10] += 0x33;
    }

    Sum = Checkout_Sum(Buf,index);

    Buf[index++] = Sum;
    Buf[index++] = 0x16;
    memmove(Buf + 4,Buf,index);
    
    Buf[0] = 0xFE;
    Buf[1] = 0xFE;
    Buf[2] = 0xFE;
    Buf[3] = 0xFE;
    
    return index + 4;
}


/*------------------------------------------------------------
操作目的：   电表读数据
作    者：      
操作结果：   
函数参数：type 记录参数类别
            
返回值：
            无

版   本:    V1.0

记   录:

------------------------------------------------------------*/

static uint8 Meter_Read(uint32 DI, uint8_t* pData,uint8 Ch)
{

    uint8 Send[24] = {0};
    int Len = 0;
    uint8 RcvBuf[32] = {0};
    uint8 FrameBuf[32] = {0};
    uint8 RcvLen = 0;
    int i = 0;
    uint32 ms = 0;
    
    Len = Meter_Pack(Send,DI,CMD_READ,NULL,0,Ch);

    Drv_Uart_Write(METER_COM,(const unsigned char *) Send,Len,100);

    ms = HAL_GetTick();
    
    do
    {
        Len = Drv_Uart_Read(METER_COM,RcvBuf,32,100);

        if(Len > 0)
        {
            for(i = 0; i < Len;i++)
            {

                FrameBuf[RcvLen] = RcvBuf[i];
                
                if(RcvLen == 0)
                {
                    if(FrameBuf[RcvLen] == 0x68)
                    {
                        
                        RcvLen++;
                    }
                }
                else 
                {
                    RcvLen++;
                }
                
                if(RcvLen == 8)
                {
                    if(FrameBuf[7] != 0x68)
                    {
                        RcvLen = 0;
                    }

                    if(memcmp(FrameBuf + 1,g_Meter_Comm_Data[Ch].Meter_addr,6))
                    {
                       RcvLen = 0; 
                    }
                    
                }
                else if(RcvLen >= FrameBuf[9] + 12)
                {
                    char sum = 0;
                   
                    sum = Checkout_Sum(FrameBuf,RcvLen - 2);

                    if(sum == FrameBuf[RcvLen - 2])
                    {
                        
                        memcpy(pData,FrameBuf + 14,4);
                        int i = 0;
                        for(i = 0; i < FrameBuf[9];i++)
                        {
                           pData[i] -= 0x33; 
                        }
                        
                        return TRUE;
                    }
                    
                }
            }

        }
        else
        {
            osDelay(10);
        }

    }while(HAL_GetTick() - ms < MAX_TIME_OUT);
    
    return FALSE;
    
}


/*------------------------------------------------------------
操作目的：   电表读数据
作    者：      
操作结果：   
函数参数：type 记录参数类别
            
返回值：
            无

版   本:    V1.0

记   录:

------------------------------------------------------------*/

static uint8 Meter_Read_Addr(uint32 DI, uint8_t* pData,int Ch)
{

    uint8 Send[24] = {0};
    int Len = 0;
    uint8 RcvBuf[32] = {0};
    uint8 FrameBuf[32] = {0};
    uint8 RcvLen = 0;
    int i = 0;
    uint32 ms = 0;
    
    Len = Meter_Pack(Send,DI,CMD_READ_ADDR,NULL,0,Ch);

    Drv_Uart_Write(METER_COM,(const unsigned char *) Send,Len,100);

    ms = HAL_GetTick();
    
    do
    {
        Len = Drv_Uart_Read(METER_COM,RcvBuf,32,100);

        if(Len > 0)
        {
            for(i = 0; i < Len;i++)
            {

                FrameBuf[RcvLen] = RcvBuf[i];
                
                if(RcvLen == 0)
                {
                    if(FrameBuf[RcvLen] == 0x68)
                    {
                        
                        RcvLen++;
                    }
                }
                else 
                {
                    RcvLen++;
                }
                
                if(RcvLen == 8)
                {
                    if(FrameBuf[7] != 0x68)
                    {
                        RcvLen = 0;
                    }
                    
                }
                else if(RcvLen >= FrameBuf[9] + 12)
                {
                    char sum = 0;
                   
                    sum = Checkout_Sum(FrameBuf,RcvLen - 2);

                    if(sum == FrameBuf[RcvLen - 2])
                    {
                        
                        memcpy(pData,FrameBuf + 10,6);
                        for(i = 0; i < FrameBuf[9];i++)
                        {
                           pData[i] -= 0x33; 
                        }
                        
                        return TRUE;
                    }
                    
                }
            }

        }
        else
        {
            osDelay(10);
        }

    }while(HAL_GetTick() - ms < MAX_TIME_OUT);
    
    return FALSE;
    
}


static void Meter_Loop(void)
{
    int i = 0;
    
    static U32 err[2] = {0};
    
    for(i = 0; i < GUN_NUM;i++)
    {
        U8 Data[4] = {0};

        if(GUN_NUM == 1)
        {
            memset(g_Meter_Comm_Data[0].Meter_addr,0xAA,6);
            Meter_Read_Addr(0,g_Meter_Comm_Data[0].Meter_addr,0);
        }
        
        if(Meter_Read(ENERGE_DI,Data,i))
        {
            err[i] = HAL_GetTick();
            g_Meate_Data[i].Totol_Energy =  Bcd4ToHexUint32(Data);
        }
        
        if(Meter_Read(VOLT_A_DI,Data,i))
        {
            err[i] = HAL_GetTick();
            g_Meate_Data[i].VolageA =  Bcd2ToHex16(Data);
        }       

        if(Get_phaseType() == PHASE_3)
        {
            if(Meter_Read(VOLT_B_DI,Data,i))
            {
                err[i] = HAL_GetTick();
                g_Meate_Data[i].VolageB =  Bcd2ToHex16(Data);
            }
            if(Meter_Read(VOLT_C_DI,Data,i))
            {
                err[i] = HAL_GetTick();
                g_Meate_Data[i].VolageC =  Bcd2ToHex16(Data);
            }
                    
        }

        if(Meter_Read(CUR_A_DI,Data,i))
        {
            err[i] = HAL_GetTick();
            g_Meate_Data[i].CurrentA =  Bcd3ToHex32(Data)/10;//三个小数点 化成2个
            if(Get_phaseType() == PHASE_3)
            {

                g_Meate_Data[i].CurrentA *= CT;//变比

            }
        }
        
        if(Get_phaseType() == PHASE_3)
        {
            if(Meter_Read(CUR_B_DI,Data,i))
            {
                err[i] = HAL_GetTick();
                g_Meate_Data[i].CurrentB =  Bcd3ToHex32(Data)/10;
                if(Get_phaseType() == PHASE_3)
                {

                    g_Meate_Data[i].CurrentB *= CT;//变比

                }                
            }
            if(Meter_Read(CUR_C_DI,Data,i))
            {
                err[i] = HAL_GetTick();
                g_Meate_Data[i].CurrentC =  Bcd3ToHex32(Data)/10; //三个小数点 化成2个
                if(Get_phaseType() == PHASE_3)
                {

                    g_Meate_Data[i].CurrentC *= CT;//变比

                }                
            }           
        }

        if(HAL_GetTick() - err[i] > 10000)
        {
            g_Meter_Comm_Data[i].Meter_CommStatus = FALSE;
        }
        else
        {
            g_Meter_Comm_Data[i].Meter_CommStatus = TRUE;
        }
        
        if(g_Meter_Comm_Data[i].Meter_CommStatus == FALSE)
        {
            g_Meate_Data[i].VolageA = 0;
            g_Meate_Data[i].VolageB = 0;
            g_Meate_Data[i].VolageC = 0;
            g_Meate_Data[i].CurrentA = 0;
            g_Meate_Data[i].CurrentB = 0;
            g_Meate_Data[i].CurrentC = 0;
            USART_Init(METER_COM,STOP_1,P_EVEN,2400);
            osDelay(5000);
        }
    }
}

MEASURE_DATA Get_Meter_Data(uint8 Ch)
{
    return g_Meate_Data[Ch];
}

uint8 Get_Meter_Comm(uint8 Ch)
{
    return g_Meter_Comm_Data[Ch].Meter_CommStatus;
}


void API_Meter_Init(void)
{
    int i = 0;  
    USART_Init(METER_COM,STOP_1,P_EVEN,METER_BOAD_RATE);
    for(i = 0; i < 2;i++)
    {
        g_Meate_Data[i].Totol_Energy = 0;
        g_Meate_Data[i].VolageA = 2200;
        g_Meate_Data[i].VolageB = 2200;
        g_Meate_Data[i].VolageC = 2200;
        g_Meate_Data[i].CurrentA= 0;
        g_Meate_Data[i].CurrentA = 0;
        g_Meate_Data[i].CurrentC = 0;

        g_Meter_Comm_Data[i].Meter_CommStatus = TRUE;
    }    
}
/*------------------------------------------------------------
操作目的：   电表采集任务
作    者：      
操作结果：   
函数参数：type 记录参数类别
            
返回值：
            无

版   本:    V1.0

记   录:

------------------------------------------------------------*/
void API_Meter_Task(void)
{
    Meter_Loop();
    osDelay(10);
}













