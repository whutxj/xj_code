

/******************************************************************************
* Includes
******************************************************************************/
#include "main.h"
#include "Tsk_Measure.h"
#include "Tsk_API_Meter.h"
#include <string.h>
#include <stdio.h>
#include "cmsis_os.h"
#include "Sys_Config.h"
//#include "Drv_ATT7053A.h"
//#include "BSP_Rtc.h"
#include "Drv_RN8029.h"
/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/


/******************************************************************************
* Global Variable Definitions
******************************************************************************/
static MEASURE_DATA g_Measure_Data[2];
static osThreadId Measure_Task_Id;

osThreadId Get_Measrue_Task_ID(void)
{
    return Measure_Task_Id;
}

MEASURE_DATA Get_Measrue_Data(uint8 Ch)
{
    uint8 type = Get_MeterType();
    
    if(type == TYPE_DLT_07)
    {
        return Get_Meter_Data(Ch);
    }
    //else if(type == TYPE_HARD || type == TYPE_RN8209)
    {
    
        return g_Measure_Data[Ch];
    }
//	return g_Measure_Data[Ch];
}
static uint32_t adj_buf[3] = {0};
static char adj_flag = 0;
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
void Measure_Thread(const void *p)
{
    uint8 type = Get_MeterType();
    double Energy = 0;
    uint32 iHVal = 0;
    uint32 Buf[3] = {0};
    //type = TYPE_RN8209;
    if(type == TYPE_DLT_07)
    {
        API_Meter_Init();
    }
    else if(type == TYPE_RN8209)
    {
        
        Drv_RN8209_SPI_Init();
        RN8209_Init_Para();
        
        g_Measure_Data[0].Totol_Energy = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR12);
        iHVal = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR13);
        g_Measure_Data[0].Totol_Energy += (iHVal<<16);
        Energy = (float)g_Measure_Data[0].Totol_Energy/100.0;
		
		if(Energy >= 999999.99)
		{
			Energy = 0.0;
		}
    }    
    while(1)
    {

        if(type== TYPE_DLT_07)
        {
            API_Meter_Task();
        }
        else if(type == TYPE_HARD || type == TYPE_RN8209)
        {
            float ReadEnergy = 0.0;

            {
                ReadEnergy = RN8209_Read_Val(Buf);
            }
            Energy += ReadEnergy;
			if(Energy >= 999999.99)
			{
				Energy = 0.0;
			}
            g_Measure_Data[0].VolageA = Buf[0]/10 - 55;//补偿5V
            // add V1.08 
            if(g_Measure_Data[0].VolageA > 3000)
                g_Measure_Data[0].VolageA = 2200;
            
            if(g_Measure_Data[0].VolageA < 100)
                g_Measure_Data[0].VolageA = 0;
            
            g_Measure_Data[0].CurrentA = Buf[1];
            g_Measure_Data[0].Totol_Energy = Energy * 100;
            
            HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR12,g_Measure_Data[0].Totol_Energy);
            HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR13,(g_Measure_Data[0].Totol_Energy>>16));
            
            osDelay(2000);
            
            if(adj_flag)
            {
                RN8209_Measure_Adj((uint8*)adj_buf);
                adj_flag = 0;
            }
        }
        else
        {
            osDelay(100);
        }
        
    }
}
/*  JWT  */

void SetupMeasureApp(osPriority ThreadPriority,uint8_t StackNum)
{
	osThreadDef(ModBusThread, Measure_Thread, ThreadPriority, 0, StackNum*configMINIMAL_STACK_SIZE);
	Measure_Task_Id = osThreadCreate (osThread(ModBusThread), NULL);    
}










