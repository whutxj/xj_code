
#ifndef __BSP_PWM_C
#define __BSP_PWM_C

/******************************************************************************
* Include Files
******************************************************************************/
#include "main.h"
#include "BSP_Pwm.h"

/******************************************************************************
* Static Variable Definitions
******************************************************************************/
static TIM_HandleTypeDef    TimHandle;

static TIM_HandleTypeDef    TimHandleLed;
/* Timer Output Compare Configuration Structure declaration */
static TIM_OC_InitTypeDef sConfig;

static TIM_OC_InitTypeDef sConfigLed;
/******************************************************************************
* Static Function Prototype Definitions
******************************************************************************/
static void Error_Handler(void);

///**
//  * @brief TIM MSP Initialization
//  *        This function configures the hardware resources used in this example:
//  *           - Peripheral's clock enable
//  *           - Peripheral's GPIO Configuration
//  * @param htim: TIM handle pointer
//  * @retval None
//  */
//void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
//{
//    GPIO_InitTypeDef   GPIO_InitStruct;
//    /*##-1- Enable peripherals and GPIO Clocks #################################*/
//    /* TIMx Peripheral clock enable */
//    
//    if(htim->Instance == TIMx)
//    {
//        TIMx_CLK_ENABLE();

//        /* Enable all GPIO Channels Clock requested */
//        TIMx_CHANNEL_GPIO_PORT();

//				__HAL_AFIO_REMAP_TIM1_ENABLE();
//				
//        /* Configure PA.0  (On Eval Board, pin 31 on CN1  for example) (TIM2_Channel1), PA.1  (On Eval Board, pin 33 on CN1  for example) (TIM2_Channel2), PA.2  (On Eval Board, pin 34 on CN1  for example) (TIM2_Channel3),
//         PA.3  (On Eval Board, pin 58 on CN11 for example) (TIM2_Channel4) in output, push-pull, alternate function mode
//        */
//        /* Common configuration for all channels */
//        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//        GPIO_InitStruct.Pull = GPIO_PULLUP;
//        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;

//        GPIO_InitStruct.Pin = TIMx_GPIO_PIN_CHANNEL1;
//        HAL_GPIO_Init(TIMx_GPIO_PORT_CHANNEL1, &GPIO_InitStruct);

////        GPIO_InitStruct.Pin = TIMx_GPIO_PIN_CHANNEL2;
////        HAL_GPIO_Init(TIMx_GPIO_PORT_CHANNEL2, &GPIO_InitStruct);
//    }
//    if(htim->Instance == TIM4)
//    {
//        TIMy_CLK_ENABLE();

//        /* Enable all GPIO Channels Clock requested */
//        TIMy_CHANNEL_GPIO_PORT();
//        __HAL_RCC_AFIO_CLK_ENABLE();
//        __HAL_AFIO_REMAP_TIM4_ENABLE();
//        
//        // AFIO->MAPR |= AFIO_MAPR_TIM4_REMAP;
//        
//        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//        GPIO_InitStruct.Pull = GPIO_PULLUP;
//        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

//        GPIO_InitStruct.Pin = TIMy_GPIO_PIN_CHANNEL1;
//        HAL_GPIO_Init(TIMy_GPIO_PORT_CHANNEL1, &GPIO_InitStruct);
//      
//    }

//}


/* Private functions ---------------------------------------------------------*/

/**
  * @brief  BSP_Pwm_Init function.
  * @param  None
  * @retval None
  */
int BSP_Pwm_Init(void)
{
 
    TimHandle.Instance = TIMx;
  
    TimHandle.Init.Prescaler         = 1;
    TimHandle.Init.Period            = PERIOD_VALUE;
    TimHandle.Init.ClockDivision     = 0;
    TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
    TimHandle.Init.RepetitionCounter = 0;
    if (HAL_TIM_PWM_Init(&TimHandle) != HAL_OK)
    {
    /* Initialization Error */
        Error_Handler();
    }

    /*##-2- Configure the PWM channels #########################################*/
    /* Common configuration for all channels */
    sConfig.OCMode       = TIM_OCMODE_PWM1;
    sConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;
    sConfig.OCFastMode   = TIM_OCFAST_DISABLE;
    sConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
    sConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;

    sConfig.OCIdleState  = TIM_OCIDLESTATE_RESET;

    /* Set the pulse value for channel 1 */
    sConfig.Pulse = PULSE1_VALUE;
    if (HAL_TIM_PWM_ConfigChannel(&TimHandle, &sConfig, TIM_CHANNEL_3) != HAL_OK)
    {
    /* Configuration Error */
        Error_Handler();
    }

    /*##-3- Start PWM signals generation #######################################*/
    /* Start channel 1 */
    if (HAL_TIM_PWM_Start(&TimHandle, TIM_CHANNEL_3) != HAL_OK)
    {
    /* PWM Generation Error */
        Error_Handler();
    }

    BSP_Pwm_Out(100,0);

    return HAL_OK;
}

/**
  * @brief  This function is called in case of exchange Dutyfactor.
  * @param  iDutyfactor ,Port
  * @retval None
  */
void BSP_Pwm_Out(uint16_t iDutyfactor,int Port )
{

    if(TimHandle.Instance == TIMx)
    {
        if(Port == 0)
        {
            __HAL_TIM_SetCompare(&TimHandle,TIM_CHANNEL_2,(PERIOD_VALUE *(iDutyfactor)) / 100);
        }
        else
        {
            __HAL_TIM_SetCompare(&TimHandle,TIM_CHANNEL_2,(PERIOD_VALUE *(iDutyfactor)) / 100);
        }
    }
    else
    {
        //BSP_Pwm_Init();
    }
    
}


/* Private functions ---------------------------------------------------------*/

/**
  * @brief  BSP_Pwm_Init function.
  * @param  None
  * @retval None
  */
int BSP_Led_Pwm_Init(void)
{
 
//    TimHandleLed.Instance = TIM4;
//  
//    TimHandleLed.Init.Prescaler         = 1;
//    TimHandleLed.Init.Period            = PERIOD_VALUE;
//    TimHandleLed.Init.ClockDivision     = 0;
//    TimHandleLed.Init.CounterMode       = TIM_COUNTERMODE_UP;
//    TimHandleLed.Init.RepetitionCounter = 0;
//    if (HAL_TIM_PWM_Init(&TimHandleLed) != HAL_OK)
//    {
//    /* Initialization Error */
//        Error_Handler();
//    }

//    /*##-2- Configure the PWM channels #########################################*/
//    /* Common configuration for all channels */
//    sConfigLed.OCMode       = TIM_OCMODE_PWM1;
//    sConfigLed.OCPolarity   = TIM_OCPOLARITY_HIGH;
//    sConfigLed.OCFastMode   = TIM_OCFAST_DISABLE;
//    sConfigLed.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
//    sConfigLed.OCNIdleState = TIM_OCNIDLESTATE_RESET;

//    sConfigLed.OCIdleState  = TIM_OCIDLESTATE_RESET;

//    /* Set the pulse value for channel 1 */
//    sConfigLed.Pulse = PULSE1_VALUE;
//    if (HAL_TIM_PWM_ConfigChannel(&TimHandleLed, &sConfigLed, TIM_CHANNEL_2) != HAL_OK)
//    {
//    /* Configuration Error */
//        Error_Handler();
//    }
//    
//    /* Start channel 2 */
//    if (HAL_TIM_PWM_Start(&TimHandleLed, TIM_CHANNEL_2) != HAL_OK)
//    {
//    /* PWM Generation Error */
//        Error_Handler();
//    }
    
    return HAL_OK;
}

/**
  * @brief  This function is called in case of exchange Dutyfactor.
  * @param  iDutyfactor ,Port
  * @retval None
  */
void BSP_Led_Pwm_Out(uint16_t iDutyfactor)
{
   if(TimHandleLed.Instance == TIM4)
   {
       __HAL_TIM_SetCompare(&TimHandleLed,TIM_CHANNEL_2,(PERIOD_VALUE *(iDutyfactor)) / 100);
   }
   else
   {
       BSP_Led_Pwm_Init();
   }
     
}
    
/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* Turn LED3 on */

  while (1)
  {
  }
}
#endif

