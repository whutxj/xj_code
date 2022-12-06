/******************************************************************************
*  
*  File name:     LED_Function.h
*  
*  Version:       V1.00
*  Created on:    2014-12-15
*  Description:   
*  History:
*      Date        Author        Modification
*    1.2016-12-15  
*    2. ...
******************************************************************************/
#ifndef __BSP_PWM_H
#define __BSP_PWM_H

/*****************************************************************************/
#ifdef __cplusplus
 extern "C" {
#endif

/******************************************************************************
* Includes
******************************************************************************/
#include "main.h"

     
/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/

     /* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* User can use this section to tailor TIMx instance used and associated
   resources */
/* Definition for TIMx clock resources */
#define TIMx                           TIM1
#define TIMx_CLK_ENABLE()              __HAL_RCC_TIM1_CLK_ENABLE()

/* Definition for TIMx Channel Pins */
#define TIMx_CHANNEL_GPIO_PORT()       __HAL_RCC_GPIOE_CLK_ENABLE()
#define TIMx_GPIO_PORT_CHANNEL1        GPIOE
#define TIMx_GPIO_PORT_CHANNEL2        GPIOE

#define TIMx_GPIO_PIN_CHANNEL1         GPIO_PIN_13
#define TIMx_GPIO_PIN_CHANNEL2         GPIO_PIN_6



#define TIMy                           TIM4
#define TIMy_CLK_ENABLE()              __HAL_RCC_TIM4_CLK_ENABLE()

/* Definition for TIMx Channel Pins */
#define TIMy_CHANNEL_GPIO_PORT()       __HAL_RCC_GPIOD_CLK_ENABLE()
#define TIMy_GPIO_PORT_CHANNEL1        GPIOD

#define TIMy_GPIO_PIN_CHANNEL1         GPIO_PIN_13


/* Private typedef -----------------------------------------------------------*/
#define  PERIOD_VALUE       (uint32_t)(36000)                 /* Period Value  */
#define  PULSE1_VALUE       (uint32_t)(PERIOD_VALUE/2)        /* Capture Compare 1 Value  */
#define  PULSE2_VALUE       (uint32_t)(PERIOD_VALUE/2)        /* Capture Compare 2 Value  */


int BSP_Pwm_Init(void);

void BSP_Pwm_Out(uint16_t iDutyfactor,int Port );


int BSP_Led_Pwm_Init(void);

void BSP_Led_Pwm_Out(uint16_t iDutyfactor);

#ifdef __cplusplus
	}
#endif
	

#endif

