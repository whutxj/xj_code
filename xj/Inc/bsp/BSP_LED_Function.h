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
#ifndef __BSP_LED_FUNCTION_H
#define __BSP_LED_FUNCTION_H

/*****************************************************************************/
#ifdef __cplusplus
 extern "C" {
#endif
	 
#ifndef __BSP_LED_FUNCTION_C
#define BSP_LED_FUNCTION_EXTERN  extern
#else
#define BSP_LED_FUNCTION_EXTERN
#endif

/******************************************************************************
* Includes
******************************************************************************/
#include "main.h"

/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/
#define LEDn                                    5           //
#define LED1_PIN                                GPIO_PIN_0
#define LED1_GPIO_PORT                          GPIOE
#define LED1_GPIO_CLK_ENABLE()                  __GPIOE_CLK_ENABLE()  
#define LED1_GPIO_CLK_DISABLE()                 __GPIOE_CLK_DISABLE()  

#define LED2_PIN                                GPIO_PIN_1  //
#define LED2_GPIO_PORT                          GPIOE
#define LED2_GPIO_CLK_ENABLE()                  __GPIOE_CLK_ENABLE()  
#define LED2_GPIO_CLK_DISABLE()                 __GPIOE_CLK_DISABLE()

#define LED3_PIN                                GPIO_PIN_2//  
#define LED3_GPIO_PORT                          GPIOE
#define LED3_GPIO_CLK_ENABLE()                  __GPIOE_CLK_ENABLE()  
#define LED3_GPIO_CLK_DISABLE()                 __GPIOE_CLK_DISABLE()  

#define LED4_PIN                                GPIO_PIN_0//BOARD
#define LED4_GPIO_PORT                          GPIOC
#define LED4_GPIO_CLK_ENABLE()                  __GPIOD_CLK_ENABLE()
#define LED4_GPIO_CLK_DISABLE()                 __GPIOD_CLK_DISABLE()

#if 0
#define LED4_PIN                                GPIO_PIN_7//BOARD
#define LED4_GPIO_PORT                          GPIOD
#define LED4_GPIO_CLK_ENABLE()                  __GPIOD_CLK_ENABLE()  
#define LED4_GPIO_CLK_DISABLE()                 __GPIOD_CLK_DISABLE()
#endif

#define LED5_PIN                                GPIO_PIN_3//
#define LED5_GPIO_PORT                          GPIOE
#define LED5_GPIO_CLK_ENABLE()                  __GPIOE_CLK_ENABLE()  
#define LED5_GPIO_CLK_DISABLE()                 __GPIOE_CLK_DISABLE()  

#define LED6_PIN                                GPIO_PIN_9
#define LED6_GPIO_PORT                          GPIOC
#define LED6_GPIO_CLK_ENABLE()                  __GPIOC_CLK_ENABLE()  
#define LED6_GPIO_CLK_DISABLE()                 __GPIOC_CLK_DISABLE()

#define LED7_PIN                                GPIO_PIN_11//
#define LED7_GPIO_PORT                          GPIOD
#define LED7_GPIO_CLK_ENABLE()                  __GPIOD_CLK_ENABLE()  
#define LED7_GPIO_CLK_DISABLE()                 __GPIODCLK_DISABLE()

#define LED8_PIN                                GPIO_PIN_12        
#define LED8_GPIO_PORT                          GPIOD
#define LED8_GPIO_CLK_ENABLE()                  __GPIOD_CLK_ENABLE()  
#define LED8_GPIO_CLK_DISABLE()                 __GPIODCLK_DISABLE()



/******************************************************************************
* Data Type Definitions
******************************************************************************/
typedef enum
{
	LED_On = 0,
	LED_Off = 1,
	LED_Toggle = 2,
}LED_ACTDef;

typedef enum
{
    RED_LED1  = 0,
    GRERN_LED1 = 1,
    YALLOW_LED1 = 2,
    BOARD_LED = 3,
    NET_LED = 4,
//    YALLOW_LED2 = 4,
//	GRERN_LED2 = 6,
//    RED_LED2 = 7,
}LED_TypDef;



/******************************************************************************
* Global Variable Definitions
******************************************************************************/



/******************************************************************************
* Function Prototype Definitions
******************************************************************************/
BSP_LED_FUNCTION_EXTERN void BSP_LED_Init(LED_TypDef Led);
BSP_LED_FUNCTION_EXTERN void LED_Function(LED_TypDef Led,LED_ACTDef Led_ACT);
BSP_LED_FUNCTION_EXTERN void BSP_LED_595_Init(void);
BSP_LED_FUNCTION_EXTERN void BSP_Led_SendData(uint16_t Data);
BSP_LED_FUNCTION_EXTERN void BSP_Led_Data(uint8_t Data);
#ifdef __cplusplus
}
#endif

#endif /* __LED_Function_H */

/******************************************************************************
*                            end of file
******************************************************************************/
