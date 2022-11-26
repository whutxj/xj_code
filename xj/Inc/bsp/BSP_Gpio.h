/******************************************************************************
*  
*  File name:     LED_Function.h
*  Author:        
*  Version:       V1.00
*  Created on:    2014-12-15
*  Description:   
*  History:
*      Date        Author        Modification
*    1.2014-12-15  
*    2. ...
******************************************************************************/
#ifndef __BSP_GPIO_FUNCTION_H
#define __BSP_GPIO_FUNCTION_H

/*****************************************************************************/

	 

/******************************************************************************
* Includes
******************************************************************************/
#include "main.h"


/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/ 
#define INPUT1_PIN                                GPIO_PIN_5
#define INPUT1_GPIO_PORT                          GPIOE
#define INPUT1_GPIO_CLK_ENABLE()                  __GPIOE_CLK_ENABLE()  
#define INPUT1_GPIO_CLK_DISABLE()                 __GPIOE_CLK_DISABLE()  

#define INPUT2_PIN                                GPIO_PIN_6
#define INPUT2_GPIO_PORT                          GPIOE
#define INPUT2_GPIO_CLK_ENABLE()                  __GPIOE_CLK_ENABLE()  
#define INPUT2_GPIO_CLK_DISABLE()                 __GPIOE_CLK_DISABLE()

#define INPUT3_PIN                                GPIO_PIN_7
#define INPUT3_GPIO_PORT                          GPIOE
#define INPUT3_GPIO_CLK_ENABLE()                  __GPIOE_CLK_ENABLE()  
#define INPUT3_GPIO_CLK_DISABLE()                 __GPIOE_CLK_DISABLE()  

#define INPUT4_PIN                                GPIO_PIN_11
#define INPUT4_GPIO_PORT                          GPIOE
#define INPUT4_GPIO_CLK_ENABLE()                  __GPIOE_CLK_ENABLE()  
#define INPUT4_GPIO_CLK_DISABLE()                 __GPIOE_CLK_DISABLE()

#define INPUT5_PIN                                GPIO_PIN_4
#define INPUT5_GPIO_PORT                          GPIOE
#define INPUT5_GPIO_CLK_ENABLE()                  __GPIOE_CLK_ENABLE()  
#define INPUT5_GPIO_CLK_DISABLE()                 __GPIOE_CLK_DISABLE()  

#define INPUT6_PIN                                GPIO_PIN_13            // REV1
#define INPUT6_GPIO_PORT                          GPIOC
#define INPUT6_GPIO_CLK_ENABLE()                  __GPIOC_CLK_ENABLE()  
#define INPUT6_GPIO_CLK_DISABLE()                 __GPIOC_CLK_DISABLE()

#define INPUT7_PIN                                GPIO_PIN_5            // REV2
#define INPUT7_GPIO_PORT                          GPIOD
#define INPUT7_GPIO_CLK_ENABLE()                  __GPIOD_CLK_ENABLE()  
#define INPUT7_GPIO_CLK_DISABLE()                 __GPIOD_CLK_DISABLE()

#define INPUT8_PIN                                GPIO_PIN_0            // REV3
#define INPUT8_GPIO_PORT                          GPIOD
#define INPUT8_GPIO_CLK_ENABLE()                  __GPIOD_CLK_ENABLE()  
#define INPUT8_GPIO_CLK_DISABLE()                 __GPIOD_CLK_DISABLE()

#define INPUT9_PIN                                GPIO_PIN_0            // REV4
#define INPUT9_GPIO_PORT                          GPIOC
#define INPUT9_GPIO_CLK_ENABLE()                  __GPIOC_CLK_ENABLE()  
#define INPUT9_GPIO_CLK_DISABLE()                 __GPIOC_CLK_DISABLE()

#define INPUT10_PIN                                GPIO_PIN_8            // 
#define INPUT10_GPIO_PORT                          GPIOB
#define INPUT10_GPIO_CLK_ENABLE()                  __GPIOC_CLK_ENABLE()  
#define INPUT10_GPIO_CLK_DISABLE()                 __GPIOC_CLK_DISABLE()

#define INPUT11_PIN                                GPIO_PIN_5            // 
#define INPUT11_GPIO_PORT                          GPIOC
#define INPUT11_GPIO_CLK_ENABLE()                  __GPIOC_CLK_ENABLE()  
#define INPUT11_GPIO_CLK_DISABLE()                 __GPIOC_CLK_DISABLE()

#define INPUT12_PIN                                GPIO_PIN_9            //
#define INPUT12_GPIO_PORT                          GPIOC
#define INPUT12_GPIO_CLK_ENABLE()                  __GPIOC_CLK_ENABLE()  
#define INPUT12_GPIO_CLK_DISABLE()                 __GPIOC_CLK_DISABLE()

#define INPUT13_PIN                                GPIO_PIN_10            //
#define INPUT13_GPIO_PORT                          GPIOB
#define INPUT13_GPIO_CLK_ENABLE()                  __GPIOB_CLK_ENABLE()  
#define INPUT13_GPIO_CLK_DISABLE()                 __GPIOB_CLK_DISABLE()

#define OUTPUT1_PIN                                GPIO_PIN_14//急停           
#define OUTPUT1_GPIO_PORT                          GPIOE
#define OUTPUT1_GPIO_CLK_ENABLE()                  __GPIOE_CLK_ENABLE()  
#define OUTPUT1_GPIO_CLK_DISABLE()                 __GPIOE_CLK_DISABLE()  

#define OUTPUT2_PIN                                GPIO_PIN_8
#define OUTPUT2_GPIO_PORT                          GPIOE
#define OUTPUT2_GPIO_CLK_ENABLE()                  __GPIOE_CLK_ENABLE()  
#define OUTPUT2_GPIO_CLK_DISABLE()                 __GPIOE_CLK_DISABLE()

#define OUTPUT3_PIN                                GPIO_PIN_9
#define OUTPUT3_GPIO_PORT                          GPIOE
#define OUTPUT3_GPIO_CLK_ENABLE()                  __GPIOE_CLK_ENABLE()  
#define OUTPUT3_GPIO_CLK_DISABLE()                 __GPIOE_CLK_DISABLE()  

#define OUTPUT4_PIN                                GPIO_PIN_9			//4G电源
#define OUTPUT4_GPIO_PORT                          GPIOB
#define OUTPUT4_GPIO_CLK_ENABLE()                  __GPIOE_CLK_ENABLE()  
#define OUTPUT4_GPIO_CLK_DISABLE()                 __GPIOE_CLK_DISABLE()

#define OUTPUT5_PIN                                GPIO_PIN_11
#define OUTPUT5_GPIO_PORT                          GPIOE
#define OUTPUT5_GPIO_CLK_ENABLE()                  __GPIOE_CLK_ENABLE()  
#define OUTPUT5_GPIO_CLK_DISABLE()                 __GPIOE_CLK_DISABLE()

#define OUTPUT6_PIN                                GPIO_PIN_12
#define OUTPUT6_GPIO_PORT                          GPIOE
#define OUTPUT6_GPIO_CLK_ENABLE()                  __GPIOE_CLK_ENABLE()  
#define OUTPUT6_GPIO_CLK_DISABLE()                 __GPIOE_CLK_DISABLE()



#define GPIO_INT_ZCD_PIN                GPIO_PIN_4
#define GPIO_INT_ZCD_PORT               GPIOD

/******************************************************************************
* Data Type Definitions
******************************************************************************/
typedef enum
{
    STOP_DETECT,    //急停检测
    SPD_DETECT1,    //防雷检测
    //GUN_LOCK_DETECT1,//枪在位检测
    KM1_DETECT,      //接触器状态检测
    
    ELECTR_DETECT1,  //电磁锁检测
    
//    SPD_DETECT2,
//    GUN_LOCK_DETECT2,
//    ELECTR_DETECT2,    
//    KM2_DETECT,
//    DOOR_DETECT,				// 门禁 9
//    VER_1,
//    VER_2,
//    VER_3,
    Input,
}Input_TypDef;
     
typedef enum
{
    MK1_OUT,      //接触器输出
    ELECTR1_LOCK1,//电磁锁输出1
    ELECTR1_LOCK2,//电磁锁输出2
//    MK2_OUT,      //2接触器输出
//    ELECTR2_LOCK1,//2电磁锁输出1
//    ELECTR2_LOCK2,//2电磁锁输出2
	POWER_4G,

    Output,
}Output_TypDef;

typedef enum
{
    OUT_LOW,
    OUT_HIGHE,
}OutPutDef;

typedef enum
{
    IN_LOW,
    IN_HIGHE,
}InPutDef;

/******************************************************************************
* Global Variable Definitions
******************************************************************************/
#ifdef __cplusplus
 extern "C" {
#endif


/******************************************************************************
* Function Prototype Definitions
******************************************************************************/
void BSP_GPIO_Init(void);
void BSP_GPIO_OutPut(Output_TypDef Index,OutPutDef Ctrl);
InPutDef BSP_Read_InPut(Input_TypDef Index);
void EXTI_ZCD_Config(int timer);
InPutDef Read_KM_Input(void);
#ifdef __cplusplus
}
#endif

#endif /* __LED_Function_H */

/******************************************************************************
*                            end of file
******************************************************************************/
