
#ifndef __BSP_LED_FUNCTION_C
#define __BSP_LED_FUNCTION_C

/******************************************************************************
* Include Files
******************************************************************************/

#include "BSP_LED_Function.h"

/******************************************************************************
* Static Variable Definitions
******************************************************************************/
static GPIO_TypeDef* GPIO_PORT[] = 
{
    LED1_GPIO_PORT, 
    LED2_GPIO_PORT, 
    LED3_GPIO_PORT,
    LED4_GPIO_PORT,
	LED5_GPIO_PORT,
};
const  uint16_t GPIO_PIN[LEDn] = 
{
    LED1_PIN, 
    LED2_PIN, 
    LED3_PIN, 
    LED4_PIN,
    LED5_PIN,
};


#define LED_CLK_LOW()       HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET)
/* Deselect sFLASH: Chip Select pin high */
#define LED_CLK_HIGH()      HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET)


#define LED_STCLK_LOW()       HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET)
/* Deselect sFLASH: Chip Select pin high */
#define LED_STCLK_HIGH()      HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET)

#define LED_DATA_LOW()      HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET)
/* Deselect sFLASH: Chip Select pin high */
#define LED_DATA_HIGH()     HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET)


#define CLK_LOW()       HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET)
/* Deselect sFLASH: Chip Select pin high */
#define CLK_HIGH()      HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET)


#define STCLK_LOW()       HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET)
/* Deselect sFLASH: Chip Select pin high */
#define STCLK_HIGH()      HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET)

#define DATA_LOW()      HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET)
/* Deselect sFLASH: Chip Select pin high */
#define DATA_HIGH()     HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET)

#define MR_HIGH()       HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET)


/******************************************************************************
* Static Function Prototype Definitions
******************************************************************************/


/******************************************************************************
* Function: void BSP_LED_Init(LED_TypDef Led)
* Parameter:
*   LED_TypDef Led
		@arg LED1-LED4
* Return:   Void 
* Description:
*   This function is used for LED initialize.
*
******************************************************************************/
void BSP_LED_Init(LED_TypDef Led)
{
    GPIO_InitTypeDef  GPIO_InitStruct;

    /* Enable the GPIO_LED Clock */
    switch(Led)
    {
        case GRERN_LED1: 
            LED1_GPIO_CLK_ENABLE();
            break;
        
        case RED_LED1: 
            LED2_GPIO_CLK_ENABLE();
            break;
        
        case YALLOW_LED1: 
            LED3_GPIO_CLK_ENABLE();
            break;

        case BOARD_LED:
            LED4_GPIO_CLK_ENABLE();
            break;
     
				case NET_LED:
					LED5_GPIO_CLK_ENABLE();
            break;
				
        default:  
            break;
    }
    /* Configure the GPIO_LED pin */
    GPIO_InitStruct.Pin = GPIO_PIN[Led];
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIO_PORT[Led], &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIO_PORT[Led], GPIO_PIN[Led], GPIO_PIN_RESET); 
}

/******************************************************************************
* Function: void LED_Function(LED_TypDef Led,LED_ACTDef Led_ACT)
* Parameter:
*   1. Led: Specifies the Led to be set.
			@arg LED1-LED4
		2. Led_ACT: Specifies the action.
			@arg LED_On¡¢LED_Off¡¢LED_Toggle
* Return:   Void
* Description:
*   Turns selected LED in selected station.
*
******************************************************************************/
void LED_Function(LED_TypDef Led,LED_ACTDef Led_ACT)
{
	if(Led_ACT==LED_On)
	{
		HAL_GPIO_WritePin(GPIO_PORT[Led], GPIO_PIN[Led], GPIO_PIN_SET); 
	}
	
	if(Led_ACT==LED_Off)
	{
		HAL_GPIO_WritePin(GPIO_PORT[Led], GPIO_PIN[Led], GPIO_PIN_RESET);
	}
	
    if(Led_ACT==LED_Toggle)
	{
		HAL_GPIO_TogglePin(GPIO_PORT[Led], GPIO_PIN[Led]);
	}
}

void BSP_LED_595_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStruct;

    int Led;
    
    for( Led = (0);Led < 8;Led++)
    {     
        /* Enable the GPIO_LED Clock */
        switch(Led)
        {
            case 0: 
                LED1_GPIO_CLK_ENABLE();
                break;
            
            case 1: 
                LED2_GPIO_CLK_ENABLE();
                break;
            
            case 2: 
                LED3_GPIO_CLK_ENABLE();
                break;
            
            case 3: 
                LED4_GPIO_CLK_ENABLE();
                break;
            case 4: 
                LED5_GPIO_CLK_ENABLE();
                break;
            
            case 5: 
                LED6_GPIO_CLK_ENABLE();
                break;
            case 6: 
                LED7_GPIO_CLK_ENABLE();
                break;            
            default:  
                break;
        }
        /* Configure the GPIO_LED pin */
        GPIO_InitStruct.Pin = GPIO_PIN[Led];
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
        HAL_GPIO_Init(GPIO_PORT[Led], &GPIO_InitStruct);
        HAL_GPIO_WritePin(GPIO_PORT[Led], GPIO_PIN[Led], GPIO_PIN_RESET);
    }    
}



#if 0
static void delay(void)
{	
	int i = 2000;
	while(i) 
	{ 
		i--; 
	} 
}
#endif

//// HP
//void BSP_Led_SendData(uint8_t Data)
//{
//	int i = 0;
//	for(i = 0; i < 8;i++)
//	{
//		LED_CLK_LOW();
//		//osDelay(2);
//		if(Data & 0x80)
//		{
//			LED_DATA_HIGH();
//		}
//		else
//		{
//			LED_DATA_LOW();
//		}
//        
//		Data<<=1;
//		
//		LED_CLK_HIGH();
//        
//	}
//    LED_STCLK_LOW();
//    //osDelay(5);
//    LED_STCLK_HIGH();  
//    //osDelay(1);
//    LED_STCLK_LOW();    
//}

void BSP_Led_SendData(uint16_t iData)
{
	int i = 0;
    uint16_t Data = iData;
	for(i = 0; i < 16;i++)
	{
		LED_CLK_LOW();
		//osDelay(2);
		if(Data & 0x8000)
		{
			LED_DATA_HIGH();
		}
		else
		{
			LED_DATA_LOW();
		}
        
		Data<<=1;
		
		LED_CLK_HIGH();
        
	}
    LED_STCLK_LOW();
//    osDelay(2);
    LED_STCLK_HIGH();  
    //osDelay(2);
    LED_STCLK_LOW();    
}
//V4
void BSP_Led_Data(uint8_t Data)
{
	int i = 0;
    
    static uint8_t oldData = 0x99;
    
    if(oldData != Data)
    {
       oldData = Data; 
    }
    else
    {
        return;
    }
    
    MR_HIGH();
	for(i = 0; i < 8;i++)
	{
		CLK_LOW();
        //osDelay(2); 
		if(Data & 0x80)
		{
			DATA_HIGH();
		}
		else
		{
			DATA_LOW();
		}
        
		Data<<=1;
		
		CLK_HIGH();
        
	}
    STCLK_LOW();

    STCLK_HIGH();  
    //osDelay(5);
    STCLK_LOW();    
}

#endif /* __LED_FUNCTION_C */
/******************************************************************************
*                            end of file
******************************************************************************/
