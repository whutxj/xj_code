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
#ifndef __BSP_ADC_H
#define __BSP_ADC_H

/*****************************************************************************/
#ifdef __cplusplus
 extern "C" {
#endif

/******************************************************************************
* Includes
******************************************************************************/
#include "main.h"
     
#ifndef __BSP_ADC_C
#define BSP_ADC_EXTERN  extern
#else
#define BSP_ADC_EXTERN
#endif


#define  NUM_OF_ADC_SAMPLE   10 


typedef enum
{
    VOLTAGE_12V,    //12V 采样
    VOLTAGE_5V,     //5V  采样
    VOLTAGE_3_3V,   //3.3V 采样
    PWM1,           //pwm 1 采样
    PWM2,           //pwm 2 采样
}ADC_CH_DEF;

//BSP_ADC_EXTERN ADC_HandleTypeDef    AdcHandle1;
//BSP_ADC_EXTERN ADC_HandleTypeDef    AdcHandle2;

///******************************************************************************
//* Macros Definitions (constant/Macros)
//******************************************************************************/     
//#define ADC1_GPIO_PIN_CH1       GPIO_PIN_2
//#define ADC1_GPIO_PIN_CH2       GPIO_PIN_3
//#define ADC1_GPIO_PIN_CH3       GPIO_PIN_0
//#define ADC1_GPIO_PIN_CH4       GPIO_PIN_1
//#define ADC1_GPIO_PIN_CH5       GPIO_PIN_4
//    
//#define ADC1_GPIO_PORT_CH1      GPIOC
//#define ADC1_GPIO_PORT_CH2      GPIOC
//#define ADC1_GPIO_PORT_CH3      GPIOB
//#define ADC1_GPIO_PORT_CH4      GPIOB
//#define ADC1_GPIO_PORT_CH5      GPIOC
//    
//#define ADC2_GPIO_PIN_CH1       GPIO_PIN_0
//#define ADC2_GPIO_PIN_CH2       GPIO_PIN_1
//    
//#define ADC2_GPIO_PORT_CH1      GPIOB    
//#define ADC2_GPIO_PORT_CH2      GPIOB
//    
//#define ADC1_DMA_STREAM         DMA1_Channel1
//#define ADC2_DMA_STREAM         DMA1_Channel2
//    
//    
//#define ADC1_DMA_CLK_ENABLE()          __HAL_RCC_DMA1_CLK_ENABLE()
//#define ADC1_DMA_IRQn                DMA1_Channel1_IRQn
//    
//#define ADC2_DMA_CLK_ENABLE()          __HAL_RCC_DMA1_CLK_ENABLE()
//#define ADC2_DMA_IRQn                DMA1_Channel7_IRQn


//BSP_ADC_EXTERN  __IO uint32_t uhADCxConvertedValue[5];

//void MX_ADC1_Init(void);

uint16_t BSP_Get_Adc_Val(uint8_t Ch);


int Get_CP_Sample(uint8_t Ch);

#ifdef __cplusplus
	}
#endif
	
#endif

