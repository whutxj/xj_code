
#ifndef __BSP_GPIO_C
#define __BSP_GPIO_C

/******************************************************************************
* Include Files
******************************************************************************/
#include "main.h"
#include "BSP_Gpio.h"

#include <math.h>
#include <stdlib.h>
static int init_flag = 0;
static uint8_t Act_Flg = 0;
static uint32_t Lowtick = 0;
/******************************************************************************
* Static Variable Definitions
******************************************************************************/
static GPIO_TypeDef* INPUT_GPIO_PORT[] = 
{
    INPUT1_GPIO_PORT,
    INPUT2_GPIO_PORT,
    INPUT3_GPIO_PORT,
    INPUT4_GPIO_PORT,
//    INPUT5_GPIO_PORT,
//    INPUT6_GPIO_PORT,
//    INPUT7_GPIO_PORT,
//    INPUT8_GPIO_PORT,
//    INPUT9_GPIO_PORT,
//    INPUT10_GPIO_PORT,
//    INPUT11_GPIO_PORT,   
//    INPUT12_GPIO_PORT,
//    INPUT13_GPIO_PORT,    
};
static const uint16_t INPUT_GPIO_PIN[] = 
{
    INPUT1_PIN,
    INPUT2_PIN,
    INPUT3_PIN,
    INPUT4_PIN,
    INPUT5_PIN,
//    INPUT6_PIN,
//    INPUT7_PIN,
//    INPUT8_PIN,
//    INPUT9_PIN,
//    INPUT10_PIN,
//    INPUT11_PIN,
//    INPUT12_PIN,
//    INPUT13_PIN,    
};

static GPIO_TypeDef* OUTPUT_GPIO_PORT[Output] =
{
    OUTPUT1_GPIO_PORT,
    OUTPUT2_GPIO_PORT,
    OUTPUT3_GPIO_PORT,
    OUTPUT4_GPIO_PORT,
//    OUTPUT5_GPIO_PORT,
//    OUTPUT6_GPIO_PORT,
};
static const uint16_t OUTPUT_GPIO_PIN[Output] = 
{
    OUTPUT1_PIN,
    OUTPUT2_PIN,
    OUTPUT3_PIN,
    OUTPUT4_PIN,
//    OUTPUT5_PIN,
//    OUTPUT6_PIN, 
};

TIM_HandleTypeDef  ZCD_TimeOutTimHandle;

/******************************************************************************
* Static Function Prototype Definitions
******************************************************************************/

/******************************************************************************
* Function: void BSP_GPIO_Init(LED_TypDef Led)
* Parameter:
*   Null
* Return:   Void 
* Description:
*   This function is used for LED initialize.
*
******************************************************************************/
void BSP_GPIO_Init(void)
{
	usr_GPIO_Init();
//    GPIO_InitTypeDef  GPIO_InitStruct;
//    Output_TypDef i = (Output_TypDef)0;

//    OUTPUT1_GPIO_CLK_ENABLE();
//    OUTPUT2_GPIO_CLK_ENABLE();
//    OUTPUT3_GPIO_CLK_ENABLE();
//    OUTPUT4_GPIO_CLK_ENABLE();
//    OUTPUT5_GPIO_CLK_ENABLE();
//    OUTPUT6_GPIO_CLK_ENABLE();

//    for(i = (Output_TypDef)0; i < Output;i++)
//    {
//        /* Enable the GPIO_LED Clock */
//        
//        /* Configure the GPIO_LED pin */
//        GPIO_InitStruct.Pin = OUTPUT_GPIO_PIN[i];
//        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//        GPIO_InitStruct.Pull = GPIO_PULLUP;
//        GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
//        HAL_GPIO_Init(OUTPUT_GPIO_PORT[i], &GPIO_InitStruct);
//        HAL_GPIO_WritePin(INPUT_GPIO_PORT[i], INPUT_GPIO_PIN[i], GPIO_PIN_RESET);         
//    }
//    INPUT1_GPIO_CLK_ENABLE();
//    INPUT2_GPIO_CLK_ENABLE();
//    INPUT3_GPIO_CLK_ENABLE();
//    INPUT4_GPIO_CLK_ENABLE();
//    INPUT5_GPIO_CLK_ENABLE();
//    INPUT6_GPIO_CLK_ENABLE();
//    INPUT7_GPIO_CLK_ENABLE();
//    INPUT8_GPIO_CLK_ENABLE();
//    INPUT9_GPIO_CLK_ENABLE();
//    INPUT10_GPIO_CLK_ENABLE();
//    INPUT11_GPIO_CLK_ENABLE();
//    INPUT12_GPIO_CLK_ENABLE();
//    INPUT13_GPIO_CLK_ENABLE();
//    for(Input_TypDef i = (Input_TypDef)0; i < Input;i++)
//    {
//        /* Enable the GPIO Clock */
//        
//        /* Configure the GPIO pin */
//        GPIO_InitStruct.Pin = INPUT_GPIO_PIN[i];
//        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//        GPIO_InitStruct.Pull = GPIO_PULLUP;
//        GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
//        HAL_GPIO_Init(INPUT_GPIO_PORT[i], &GPIO_InitStruct);
//        HAL_GPIO_WritePin(INPUT_GPIO_PORT[i], INPUT_GPIO_PIN[i], GPIO_PIN_RESET);         
//    }    
}

/******************************************************************************
* Function: BSP_GPIO_OutPut(Output_TypDef Index,OutPutDef Ctrl)
* Parameter:
*   1. Led: Specifies the Led to be set.
			@arg Index
		2. OutPutDef: Specifies the action.
			@arg OUT_HIGHE¡¢OUT_LOW,
* Return:   Void
* Description:
*   Turns selected OUT in selected station.
*
******************************************************************************/
void BSP_GPIO_OutPut(Output_TypDef Index,OutPutDef Ctrl)
{
    if(Ctrl == OUT_HIGHE)
    {
        HAL_GPIO_WritePin(OUTPUT_GPIO_PORT[Index], OUTPUT_GPIO_PIN[Index], GPIO_PIN_SET); 
    }

    if(Ctrl==OUT_LOW)
    {
        HAL_GPIO_WritePin(OUTPUT_GPIO_PORT[Index], OUTPUT_GPIO_PIN[Index], GPIO_PIN_RESET);
    }
}

InPutDef BSP_Read_InPut(Input_TypDef Index)
{
    GPIO_PinState  State = HAL_GPIO_ReadPin(INPUT_GPIO_PORT[Index], INPUT_GPIO_PIN[Index]);
    InPutDef In;
    if(State == GPIO_PIN_RESET)
    {
        In = IN_LOW;
    }
    else
    {
        In = IN_HIGHE;
    }
    
    return In;
}
/*  JWT  */


void EXTI_ZCD_Config(int timer)
{
    GPIO_InitTypeDef   GPIO_InitStructure;

    /* Enable GPIOA clock */
    __GPIOD_CLK_ENABLE();

    /* Configure PA0 pin as input floating */
    GPIO_InitStructure.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    GPIO_InitStructure.Pin = GPIO_INT_ZCD_PIN;
    HAL_GPIO_Init(GPIO_INT_ZCD_PORT,&GPIO_InitStructure);

    /* Enable and set EXTI Line0 Interrupt to the lowest priority */
    HAL_NVIC_SetPriority(EXTI4_IRQn, 8, 0);
    HAL_NVIC_EnableIRQ(EXTI4_IRQn);
    
    //timer
    
    uint32_t uwPrescalerValue = 0;
    /* -----------------------------------------------------------------------
    TIM4 input clock (TIM4CLK) is set to 2 * APB1 clock (PCLK1), 
    since APB1 prescaler is different from 1.   
    TIM4CLK = 2 * PCLK1  
    PCLK1 = HCLK / 4 
    => TIM4CLK = HCLK / 2 = SystemCoreClock /2
    To get TIM4 counter clock at 20 KHz, the Prescaler is computed as following:
    Prescaler = (TIM4CLK / TIM4 counter clock) - 1
    Prescaler = ((SystemCoreClock /2) /20 KHz) - 1
    ----------------------------------------------------------------------- */  
    /* Compute the prescaler value to have TIM3 counter clock equal to 10 KHz */
    uwPrescalerValue = (uint32_t)((SystemCoreClock ) / 10000) - 1;//

    /* Set TIMx instance */
    ZCD_TimeOutTimHandle.Instance = TIM4;
    ZCD_TimeOutTimHandle.Init.Period = timer - 1;						
    ZCD_TimeOutTimHandle.Init.Prescaler = uwPrescalerValue;
    ZCD_TimeOutTimHandle.Init.ClockDivision = 0;
    ZCD_TimeOutTimHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
    HAL_TIM_Base_Init(&ZCD_TimeOutTimHandle); 
}

#include "BSP_LED_Function.h"
void TIM4_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&ZCD_TimeOutTimHandle);
    
    HAL_TIM_Base_Stop(&ZCD_TimeOutTimHandle);
    
    Act_Flg = 0;
}

void HAL_TIM4_Base_MspInit(void)
{
    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* TIMx Peripheral clock enable */
    __TIM4_CLK_ENABLE();

    /*##-2- Configure the NVIC for TIMx #########################################*/
    /* Set the TIMx priority */
    HAL_NVIC_SetPriority(TIM4_IRQn, 8, 0);

    /* Enable the TIMx global Interrupt */
    HAL_NVIC_EnableIRQ(TIM4_IRQn);    
}



void EXTI4_IRQHandler(void)
{
  /* EXTI line interrupt detected */
  if(__HAL_GPIO_EXTI_GET_IT(GPIO_INT_ZCD_PIN) != RESET)
  {
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_INT_ZCD_PIN);
    //Æô¶¯¶¨Ê±Æ÷
    if(Act_Flg == 0)
    {
        if(init_flag == 1)
        {
            __HAL_TIM_SetCounter(&ZCD_TimeOutTimHandle,0);
            
            HAL_TIM_Base_Start(&ZCD_TimeOutTimHandle);       
        }
        else if(init_flag == 0)
        {
            HAL_TIM_Base_Start_IT(&ZCD_TimeOutTimHandle);
            init_flag = 1;        
        }
    }   
  }    
}

uint8_t Sample_Input(void)
{ 
    if(BSP_Read_InPut(KM1_DETECT) == IN_LOW)
    {
        Lowtick = HAL_GetTick();
    }
    return 0;
}

InPutDef Read_KM_Input(void)
{
    uint32_t Tick = HAL_GetTick();
    if(abs(Tick - Lowtick) < 100)
    {
        return IN_LOW;
    }
    return IN_HIGHE;
}
#endif


