
#ifndef __BSP_BUZZER_C
#define __BSP_BUZZER_C

/******************************************************************************
* Include Files
******************************************************************************/
#include "BSP_Buzzer.h"
/******************************************************************************
* Static Variable Definitions
******************************************************************************/

/* Private function prototypes -----------------------------------------------*/
static  uint8_t g_Buzzer_On;
/******************************************************************************
* Static Function Prototype Definitions
******************************************************************************/

    
void BSP_Buzzer_Init(void)
{
    GPIO_InitTypeDef GPIO_Buzzer_Init_Structure;

    BUZZER_PORTGPIO_CLK_ENABLE();
    GPIO_Buzzer_Init_Structure.Pin       = BUZZER_PIN; 
    GPIO_Buzzer_Init_Structure.Mode      = GPIO_MODE_OUTPUT_PP;
    GPIO_Buzzer_Init_Structure.Pull      = GPIO_PULLUP;
    GPIO_Buzzer_Init_Structure.Speed     = GPIO_SPEED_LOW; 
    HAL_GPIO_Init(BUZZER_PORT, &GPIO_Buzzer_Init_Structure);
    HAL_GPIO_WritePin(BUZZER_PORT,BUZZER_PIN,GPIO_PIN_RESET);
}

void BSP_Buzzer_on(void)
{
    g_Buzzer_On = 1;
 //   HAL_GPIO_WritePin(BUZZER_PORT,BUZZER_PIN,GPIO_PIN_SET);
}

void BSP_Buzzer_off(void)
{
    g_Buzzer_On = 0;
    g_Buzzer_On = g_Buzzer_On;
 //   HAL_GPIO_WritePin(BUZZER_PORT,BUZZER_PIN,GPIO_PIN_RESET);
}

void BSP_Buzzer_Test(void)
{
     BSP_Buzzer_Init();
     uint32_t Tick = 0;
     uint8_t Xor = 0;
     
     while(1)
     {
        if(HAL_GetTick() - Tick >= 500)
        {
            Tick = HAL_GetTick();
            Xor ^= 1;
        }
        if(Xor)
        {
            BSP_Buzzer_off();
        }
        else
        {
            BSP_Buzzer_on();
        }    
     }
}

#endif
