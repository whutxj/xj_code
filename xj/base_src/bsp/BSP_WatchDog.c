
#ifndef __BSP_WATCH_DOG_C
#define __BSP_WATCH_DOG_C

/******************************************************************************
* Include Files
******************************************************************************/
#include "main.h"
#include "stm32f4xx_hal.h"
#include "BSP_WatchDog.h"

/******************************************************************************
* Static Variable Definitions
******************************************************************************/
//IWDG_HandleTypeDef hIWDG;
//这里不再进行初始化，而是在主程序初始化
void BSP_WatchDog_Init(void)
{

    usr_IWDG_Init();
		//__HAL_IWDG_START(hiwdg); 
}

void BSP_WatchDog_Refresh(void)
{
    HAL_IWDG_Refresh(&hiwdg);
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */


#endif

