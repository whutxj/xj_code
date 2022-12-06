/******************************************************************************
*  
*  File name:     LED_Function.h
*  
*  Version:       V1.00
*  Created on:    2014-12-15
*  Description:   
*  History:
*      Date        Author        Modification
*    1.2014-12-15  
*    2. ...
******************************************************************************/
#ifndef __BSP_Buzzer_H
#define __BSP_Buzzer_H

/*****************************************************************************/
#ifdef __cplusplus
 extern "C" {
#endif

/******************************************************************************
* Includes
******************************************************************************/
#include "main.h"
//#include "stm32f1xx_hal_gpio.h"

     
/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/

#define BUZZER_PIN                              GPIO_PIN_6        
#define BUZZER_PORT                             GPIOD
#define BUZZER_PORTGPIO_CLK_ENABLE()             __HAL_RCC_GPIOD_CLK_ENABLE()
#define BUZZER_PORTGPIO_CLK_DISABLE()            __HAL_RCC_GPIOD_CLK_DISABLE()     

void BSP_Buzzer_Init(void);

void BSP_Buzzer_on(void);

void BSP_Buzzer_off(void);


     #ifdef __cplusplus
	}
#endif
	
#endif
