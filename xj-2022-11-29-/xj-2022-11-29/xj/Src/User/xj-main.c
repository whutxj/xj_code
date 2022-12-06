/******************************************************************************
*  Copyright (C), 1988-2014, Kehua Tech. Co., Ltd.
*  File name:     main.c
*  Author:        Hong Jinzhui
*  Version:       V1.00
*  Created on:    2014-12-15
*  Description:   
*  History:
*      Date        Author        Modification
*    1.2014-12-15  
*    2. ...
******************************************************************************/
#ifndef __MAIN_C
#define __MAIN_C

/******************************************************************************
* Include Files
******************************************************************************/
#include "main.h"
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "mb.h"
#include "mb_m.h"
#include "bsp_driver_uart.h"
#include "drv_usart.h"
#include "bsp_Led_Function.h"
#include "BSP_WatchDog.h"
#include "Tsk_API_Test.h"
#include "Task_SpiFlash.h"
#include "BSP_Adc.h"
#include "BSP_Rtc.h"
#include "BSP_Pwm.h"
#include "lwip.h"
#include "Sys_Config.h"
#include "MsgQueue.h"
#include "Tsk_Measure.h"
#include "Task_SpiFlash.h"
#include "Tsk_Charge_Ctrl.h"
#include "Tsk_Cloud_ComHP.h"
#include "Tsk_Cloud_Com.h"
#include "Tsk_Cloud_Com_V4.h"
#include "Tsk_Modbus.h"
#include "Tsk_TouchDW.h"
#define TAB_OFFSET  0xd000
/******************************************************************************
* Static Variable Definitions
******************************************************************************/

/******************************************************************************
* Static Function Prototype Definitions
******************************************************************************/
static void StartThread(void const * argument);

/******************************************************************************
* Function: int main(void)
* Parameter:Void 
* Return:   Void 
* Description:
******************************************************************************/
int main(void)

{
    
    
    SCB->VTOR = FLASH_BASE | TAB_OFFSET; /* Vector Table Relocation in Internal FLASH. */
    
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    SystemClock_Config();

    /* Initialize all configured peripherals */

    osThreadDef(USER_Thread, StartThread, osPriorityNormal, 0, 4*configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(USER_Thread), NULL);

    /* Start scheduler */
    osKernelStart();

    while (1)
    {
    }
}

//系统必须的初始化
static void system_init(void)
{


    BSP_LED_595_Init(); 
    
    BSP_GPIO_Init();//指示灯
    
    BSP_Rtc_Config();       //RTC 初始化     
    
    //USART_Init(COM3,STOP_1,P_EVEN,2400);
    
    USART_Init(COM1,STOP_1,P_NONE,57600);
    
    MEM_Init_Record();      //记录初始化因为后面的程序需要根据参数进行任务的初始化
    
    BSP_LED_Init(BOARD_LED);//板上的LED初始化
       
    Sys_Config_Para_Load(); //参数初始化
    
    #ifdef HP_HARD
        BSP_Led_Pwm_Init();
    #endif
    
    MX_LWIP_Init();         //网络初始化
    
    dbg_init();             //调试功能初始化

    MsgQueue_Init();        //任务间消息队列初始化

}

/******************************************************************************
* Function: static void StartThread(void const * argument)
* Parameter:* argument
* Return:   Void 
* Description:
*   
*
******************************************************************************/
static void StartThread(void const * argument)
{

    system_init();  //系统必须的初始化  
    SetupSpiFlashApp(osPriorityNormal,4);
    osDelay(5000);//等待系统的flash初始化完成
    SetupChargeCtrlApp(osPriorityNormal,4);
    SetupMeasureApp(osPriorityNormal,2);
    SetupTouchApp(osPriorityNormal,3);
    if(Get_Protocol_Type() == KH_P)
    {
        SetupCloudCommApp(osPriorityNormal,4);
        SetupModbusApp(osPriorityNormal,2);
        
    }
    else if(Get_Protocol_Type() == KH_V4)
    {
        SetupCloudCommApp_V4(osPriorityNormal,5);
    }
    osThreadTerminate(osThreadGetId());  

}

/******************************************************************************
* Function: void SystemClock_Config(void)
* Parameter:Void
* Return:   Void 
* Description:
*   System Clock Configuration
*
******************************************************************************/
void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef clkinitstruct = {0};
  RCC_OscInitTypeDef oscinitstruct = {0};
  
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  oscinitstruct.OscillatorType  = RCC_OSCILLATORTYPE_HSE;
  oscinitstruct.HSEState        = RCC_HSE_ON;
  oscinitstruct.HSEPredivValue  = RCC_HSE_PREDIV_DIV1;
  oscinitstruct.PLL.PLLState    = RCC_PLL_ON;
  oscinitstruct.PLL.PLLSource   = RCC_PLLSOURCE_HSE;
  oscinitstruct.PLL.PLLMUL      = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&oscinitstruct)!= HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  clkinitstruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  clkinitstruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  clkinitstruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  clkinitstruct.APB2CLKDivider = RCC_HCLK_DIV1;
  clkinitstruct.APB1CLKDivider = RCC_HCLK_DIV2;  
  if (HAL_RCC_ClockConfig(&clkinitstruct, FLASH_LATENCY_2)!= HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

#endif /* __MAIN_C */
/******************************************************************************
*                            end of file
******************************************************************************/

