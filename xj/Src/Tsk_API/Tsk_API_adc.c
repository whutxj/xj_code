
#ifndef TSK_ADC_C
#define TSK_ADC_C

/******************************************************************************
* Includes
******************************************************************************/

#include "MsgQueue.h"
#include "string.h"
#include "cmsis_os.h"
#include "time.h"
#include "Sys_Config.h"
#include "Global.h"
#include "Tsk_API_Meter.h"
#include "Utility.h"
#include "Bsp_ADC.h"
#include "Bsp_GPIO.h"
#include "bsp_Led_Function.h"
#include "Tsk_API_ADC.h"
#include "Tsk_Charge_Ctrl.h"
#include "Drv_MMA8652FC.h"




////#define CH_CP  0
////#define CH_TEMP 1
////#define CH_GUN  2
////0 = 继电器粘连测试端口1  
////1 = 继电器粘连测试端口2   
////2 = 继电器温度  
////3=环境温度  
////4=枪头温度   
////5 = 计量芯片过零检测  
////6= CP-VS 
////7= 漏电检测端口


//#define SWITCH_CONNNECT_NEG	0//0 = 继电器粘连测试端口1  
//#define SWITCH_CONNNECT_POS	1//1 = 继电器粘连测试端口2  
//#define SWITCH_TEMP	2//2 = 继电器温度  
//#define CH_TEMP 3//=环境温度  
//#define CH_GUN  4//4=枪头温度 
//#define MEASURE_ZERO  5//5 = 计量芯片过零检测 
//#define CH_CP  6  // CP的电压
//#define LEAK_CURRENCE_PORT	7//7= 漏电检测端口


#define   OFFSET_1  0 //补偿

// -15 ~ +15  0~ 3V
#define   OFFSET  0 //补偿


#define X1 1.875 
#define X2 2.581

#define Y1 -25
#define Y2 160

const uint32_t Temp_Tbl[]=
{
    369,
    455,
    553,
    666,
    793,
    935,
    1090,
    1257,
    1434,
    1620,
    1812,
    2007,
    2202,
    2396,
    2586,
    2700,
    2945,
};


int Get_CP_Sample(uint8_t Ch)
{
    uint16_t AdcVal = BSP_Get_Adc_Val(CH_CP);
    static uint32_t Last = 3373;
    
    float adSamp = 0;
    
    if(AdcVal == 0)
    {
        AdcVal = Last;
    }
    else
    {
        Last = AdcVal;
    }    
    
    adSamp = AdcVal * 3.3/4096;
    adSamp += OFFSET;//

    adSamp *= 4;
    
    adSamp += 0.7;

    
    return  adSamp * 100;
    
}

int Get_CP_Sample_2(uint8_t Ch)
{
    uint16_t AdcVal = BSP_Get_Adc_Val(CH_CP);
    
    
    float adSamp = 0;
    
    
    adSamp = AdcVal * 3.3/4096;
    adSamp += OFFSET;//

    adSamp *= 4;
    
    adSamp += 0.7;
    
    return  adSamp * 100;
    
}

int Get_system_Temp(void)
{
    uint16_t AdcVal = BSP_Get_Adc_Val(CH_TEMP); 
    float adSamp = 0;  
    int i = 0;
    adSamp = AdcVal * 3.3/4096;
    adSamp += OFFSET_1;//

    //
    
    adSamp *= 1000;
    
    for( i = 0;i < sizeof(Temp_Tbl)/sizeof(Temp_Tbl[0]);i++)
    {
        if(adSamp <= Temp_Tbl[i])
        {
            break;
        }
    }
    
    return 20 + i * 5;
    
}




int Get_Gun_Temp(void)
{
    uint16_t AdcVal = BSP_Get_Adc_Val(CH_TEMP); 
    float adSamp = 0;  
    //int i = 0;
    float k = 0;
    float b = 0;
    adSamp = AdcVal * 3.3/4096;
    adSamp += OFFSET_1;//

    if(adSamp <= 1.875)
    {
        return -25;
    }
    else if(adSamp >= 2.581)
    {
        return 160;
    }
    else
    {
        k = (X2 - X1)/(Y2 - Y1);
        b = Y1 - k*X1;

        return  adSamp*k + b;
    }
}


#endif
