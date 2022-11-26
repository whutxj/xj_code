
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
//���ﲻ�ٽ��г�ʼ�����������������ʼ��
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

