
#ifndef __BSP_ADC_C
#define __BSP_ADC_C

/******************************************************************************
* Include Files
******************************************************************************/
#include "BSP_Adc.h"
#include "main.h"
#include "Utility.h"
#include "BSP_LED_FUNCTION.h"
#include "math.h"



volatile unsigned short FiltAdcBuf[FILT_NUM][CH_NUM];
//static unsigned short FiltAdcLeakCurrBuf[FILT_NUM][CH_NUM];

    
///******************************************************************************
//* Static Variable Definitions
//******************************************************************************/
//static void Error_Handler(void)
//{
//        
//}

//void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
//{
//	GPIO_InitTypeDef          GPIO_InitStruct;
//    RCC_PeriphCLKInitTypeDef  PeriphClkInit;

//    /* Configure ADCx clock prescaler */
//    /* Caution: On STM32F1, ADC clock frequency max is 14MHz (refer to device   */
//    /*          datasheet).                                                     */
//    /*          Therefore, ADC clock prescaler must be configured in function   */
//    /*          of ADC clock source frequency to remain below this maximum      */
//    /*          frequency.                                                      */

//	if(hadc->Instance == ADC1)
//	{
//		static DMA_HandleTypeDef  hdma_adc;
//		/*##-1- Enable peripherals and GPIO Clocks #################################*/

//        
//		/* ADC3 Periph clock enable */
//		__ADC1_CLK_ENABLE();

//        __HAL_RCC_GPIOB_CLK_ENABLE();
//        __HAL_RCC_GPIOC_CLK_ENABLE();
//        
//        PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
//        PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV8;
//        HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
//        
//		/*##-2- Configure peripheral GPIO ##########################################*/ 
//		/* ADC1 Channel8 GPIO pin configuration */
//		GPIO_InitStruct.Pin = ADC1_GPIO_PIN_CH1;
//		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
//		GPIO_InitStruct.Pull = GPIO_NOPULL;
//		HAL_GPIO_Init(ADC1_GPIO_PORT_CH1, &GPIO_InitStruct);
//        
//        
//		GPIO_InitStruct.Pin = ADC1_GPIO_PIN_CH2;
//		HAL_GPIO_Init(ADC1_GPIO_PORT_CH2, &GPIO_InitStruct);

//		GPIO_InitStruct.Pin = ADC1_GPIO_PIN_CH3;
//		HAL_GPIO_Init(ADC1_GPIO_PORT_CH3, &GPIO_InitStruct);

//		GPIO_InitStruct.Pin = ADC1_GPIO_PIN_CH4;
//		HAL_GPIO_Init(ADC1_GPIO_PORT_CH4, &GPIO_InitStruct);

////		GPIO_InitStruct.Pin = ADC1_GPIO_PIN_CH5;
////		HAL_GPIO_Init(ADC1_GPIO_PORT_CH5, &GPIO_InitStruct);        

//		/* Enable DMA2 clock */
//		__DMA1_CLK_ENABLE();
//		/*##-3- Configure the DMA streams ##########################################*/
//		/* Set the parameters to be configured */
//		hdma_adc.Instance = ADC1_DMA_STREAM;

//		hdma_adc.Init.Direction = DMA_PERIPH_TO_MEMORY;
//		hdma_adc.Init.PeriphInc = DMA_PINC_DISABLE;
//		hdma_adc.Init.MemInc = DMA_MINC_ENABLE;
//		hdma_adc.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
//		hdma_adc.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
//		hdma_adc.Init.Mode = DMA_CIRCULAR;
//		hdma_adc.Init.Priority = DMA_PRIORITY_HIGH;

//		HAL_DMA_Init(&hdma_adc);

//		/* Associate the initialized DMA handle to the the ADC handle */
//		__HAL_LINKDMA(hadc, DMA_Handle, hdma_adc);

//		/*##-4- Configure the NVIC for DMA #########################################*/
//		/* NVIC configuration for DMA transfer complete interrupt */
////		HAL_NVIC_SetPriority(ADC1_DMA_IRQn, 8, 0);

////		HAL_NVIC_EnableIRQ(ADC1_DMA_IRQn);
//	}

//	if(hadc->Instance==ADC2)
//	{
//		static DMA_HandleTypeDef  hdma_adc;
//		/*##-1- Enable peripherals and GPIO Clocks #################################*/

//		/* ADC3 Periph clock enable */
//		__ADC2_CLK_ENABLE();
//		#if 0
//		/* Enable DMA2 clock */
//		__DMA1_CLK_ENABLE();

//		GPIO_InitStruct.Pin = ADC2_GPIO_PIN_CH1;
//		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
//		GPIO_InitStruct.Pull = GPIO_NOPULL;
//		HAL_GPIO_Init(ADC2_GPIO_PORT_CH1, &GPIO_InitStruct);
//        
//        
//		GPIO_InitStruct.Pin = ADC2_GPIO_PIN_CH2;
//		HAL_GPIO_Init(ADC2_GPIO_PORT_CH2, &GPIO_InitStruct);


//		/*##-3- Configure the DMA streams ##########################################*/
//		/* Set the parameters to be configured */

//		hdma_adc.Instance = ADC2_DMA_STREAM;

//		hdma_adc.Init.Direction = DMA_PERIPH_TO_MEMORY;
//		hdma_adc.Init.PeriphInc = DMA_PINC_DISABLE;
//		hdma_adc.Init.MemInc = DMA_MINC_ENABLE;
//		hdma_adc.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
//		hdma_adc.Init.MemDataAlignment = DMA_PDATAALIGN_HALFWORD;
//		hdma_adc.Init.Mode = DMA_CIRCULAR;
//		hdma_adc.Init.Priority = DMA_PRIORITY_HIGH;

//		HAL_DMA_Init(&hdma_adc);

//		/* Associate the initialized DMA handle to the the ADC handle */
//		__HAL_LINKDMA(hadc, DMA_Handle, hdma_adc);

//		/*##-4- Configure the NVIC for DMA #########################################*/
//		/* NVIC configuration for DMA transfer complete interrupt */
////		HAL_NVIC_SetPriority(ADC2_DMA_IRQn, 0, 0);

////		HAL_NVIC_EnableIRQ(ADC2_DMA_IRQn);
//		
//   #endif
//	 		HAL_NVIC_EnableIRQ(ADC1_2_IRQn);
//	    //HAL_NVIC_EnableIRQ(TIM6_IRQn);
//	 
//	} 
//}
//  
///**
//  * @brief ADC MSP De-Initialization 
//  *        This function frees the hardware resources used in this example:
//  *          - Disable the Peripheral's clock
//  *          - Revert GPIO to their default state
//  * @param hadc: ADC handle pointer
//  * @retval None
//  */
//void HAL_ADC_MspDeInit(ADC_HandleTypeDef *hadc)
//{
//	

//	if(hadc->Instance==ADC1)
//	{
//		/*##-1- Reset peripherals ##################################################*/
//        __HAL_RCC_ADC1_FORCE_RESET();
//        __HAL_RCC_ADC1_RELEASE_RESET();
//	}
//	if(hadc->Instance==ADC2)
//	{
//		/*##-1- Reset peripherals ##################################################*/
//	}
//}



//void MX_ADC1_Init(void)
//{
//    ADC_ChannelConfTypeDef sConfig;

//    /*##-1- Configure the ADC peripheral #######################################*/
//    AdcHandle1.Instance          = ADC1;


//    AdcHandle1.Init.ScanConvMode = ENABLE;
//    AdcHandle1.Init.ContinuousConvMode = ENABLE;
//    AdcHandle1.Init.DiscontinuousConvMode = DISABLE;
//    AdcHandle1.Init.NbrOfDiscConversion = 0;

//    AdcHandle1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
//    AdcHandle1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
//    AdcHandle1.Init.NbrOfConversion = CH_NUM;//5 * NUM_OF_ADC_SAMPLE;

//      
//    if(HAL_ADC_Init(&AdcHandle1) != HAL_OK)
//    {
//        /* Initiliazation Error */
//    }

//    /*##-2- Configure ADC regular channel ######################################*/  
//    sConfig.Channel = ADC_CHANNEL_8;//
//    sConfig.Rank = 2;
//    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;;

//    if(HAL_ADC_ConfigChannel(&AdcHandle1, &sConfig) != HAL_OK)
//    {
//        /* Channel Configuration Error */
//        Error_Handler(); 
//    }
//		
//    /*##-2- Configure ADC regular channel ######################################*/  
//    sConfig.Channel = ADC_CHANNEL_9;//
//    sConfig.Rank = 3;
//    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;


//    if(HAL_ADC_ConfigChannel(&AdcHandle1, &sConfig) != HAL_OK)
//    {
//        /* Channel Configuration Error */
//        Error_Handler(); 
//    }
//    /*##-2- Configure ADC regular channel ######################################*/  
//    sConfig.Channel = ADC_CHANNEL_13; //PC3 
//    sConfig.Rank = 4;
//    sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES_5;


//    if(HAL_ADC_ConfigChannel(&AdcHandle1, &sConfig) != HAL_OK)
//    {
//        /* Channel Configuration Error */
//        Error_Handler(); 
//    }

//    /*##-2- Configure ADC regular channel ######################################*/  
//    sConfig.Channel = ADC_CHANNEL_12;    //PF10
//    sConfig.Rank = 1;//4;
//    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;


//    if(HAL_ADC_ConfigChannel(&AdcHandle1, &sConfig) != HAL_OK)
//    {
//        /* Channel Configuration Error */
//        Error_Handler(); 
//    }

////    /*##-2- Configure ADC regular channel ######################################*/  
////    sConfig.Channel = ADC_CHANNEL_14;   //PF5 µçÑ¹¼ì²â1
////    sConfig.Rank = 2;
////    sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;


////    if(HAL_ADC_ConfigChannel(&AdcHandle1, &sConfig) != HAL_OK)
////    {
////        /* Channel Configuration Error */
////        Error_Handler(); 
////    }
////    
//    if (HAL_ADCEx_Calibration_Start(&AdcHandle1) != HAL_OK)
//    {
//        /* Calibration Error */
//        Error_Handler();
//    }

//    /*##-3- Start the conversion process and enable interrupt ##################*/  
//    if(HAL_ADC_Start_DMA(&AdcHandle1, (uint32_t*)FiltAdcBuf,CH_NUM * FILT_NUM) != HAL_OK)
//    {
//        /* Start Conversation Error */
//        Error_Handler(); 
//    }
//    
//  
//}

uint16_t BSP_Get_Adc_Val(uint8_t Ch)
{
    short i = 0;
    short Ctn = 0;
    uint32_t TmpVal = 0;

    for(i = 0; i< FILT_NUM;i++)
    {
        if(Ch == CH_CP)//ÕâÀïÐèÒª×¢ÒâÕâÊÇÕë¶ÔCPÐÅºÅµÄ¸ßµçÆ½½øÐÐ»ñÈ¡
        {
            if(FiltAdcBuf[i][Ch] > 1400)//¾ØÐÎ²¨//300 4.5v
            {
                TmpVal += FiltAdcBuf[i][Ch];
                Ctn++;
            }
        }
        else
        {
            TmpVal += FiltAdcBuf[i][Ch];
            Ctn++;            
        }        
    }
    
    if(Ctn > 5)
    {
        TmpVal /= Ctn;
    }
    else
    {
        TmpVal = 0;
    }
    return TmpVal;
}


//int Get_CP_Sample(uint8_t Ch)
//{
//    uint16_t AdcVal = BSP_Get_Adc_Val(0);
//    static uint32_t Last = 2500;
//    
//    float adSamp = 0;
//    
//    if(AdcVal == 0)
//    {
//        AdcVal = Last;
//    }
//    else
//    {
//        Last = AdcVal;
//    }    
//    
//    adSamp = AdcVal * 3.3/4096;
//    adSamp += 0.2;//200mv ²¹³¥
//    adSamp /= 1.36;
//    
//    adSamp /= 8;
//    
//    adSamp  = adSamp * (105/2);
//    
//    return  adSamp * 100;
//    
//}
void BSP_Test_Fun(void)
{
    int i = 0;
//    MX_ADC1_Init();
    while(1)
    {
        for(i = 0; i < 5;i++)
        {
            dbg_printf(1,"adc ch %d =%d",i,BSP_Get_Adc_Val(i));
        }
    } 
}

#define CYCLE_CNT   100

uint32_t Conversion_Value = 0;

uint32_t  voltage = 0;//²ÉÑùµçÑ¹/
uint32_t adc_avg_sum = 0;
uint32_t adc_sum = 0; //Æ½·½ºÍ
uint32_t adc_avg = 0; //Æ½¾ù
int cnt = 0;          //²ÉÑù´ÎÊý
double sample_RMS = 0; //ÓÐÐ§Öµ
uint32_t sample_AVG = 2048; //Æ½¾ù



uint32_t get_Leakage_current(void)
{
	
	int tmp_sample = 0;//²ÉÑù-Æ½¾ù¡
	int i;
  //uint16_t 
  /* Cleat Interrupt Pending bit */
  //TIM4_ClearITPendingBit(TIM4_IT_UPDATE);
	for(i=0;i<FILT_NUM;i++)
	{
		tmp_sample = FiltAdcBuf[i][LEAK_CURRENCE_PORT] - sample_AVG;
		adc_sum += (tmp_sample * tmp_sample);
	}
  //adc_avg_sum += voltage;
  cnt++;
  if(cnt == CYCLE_CNT)
  {
      //sample_AVG = (uint32_t)(adc_avg_sum/CYCLE_CNT);
      sample_RMS = (double)sqrt(adc_sum/cnt);//ÓÐÐ§Öµ
      sample_RMS = sample_RMS*500;
      sample_RMS = (sample_RMS * 3.3) / 4096;
      
      adc_avg_sum = 0;
      cnt = 0;
      adc_sum = 0;
      adc_avg = 0;
  }
     return (uint32_t)sample_RMS; 
}

//void MX_ADC2_Init(void)
//{
//    ADC_ChannelConfTypeDef sConfig;

//    /*##-1- Configure the ADC peripheral #######################################*/
//    AdcHandle2.Instance          = ADC2;


//    AdcHandle2.Init.ScanConvMode = ENABLE;
//    AdcHandle2.Init.ContinuousConvMode = DISABLE;
//    AdcHandle2.Init.DiscontinuousConvMode = DISABLE;
//    AdcHandle2.Init.NbrOfDiscConversion = 0;

//    AdcHandle2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
//    AdcHandle2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
//    AdcHandle2.Init.NbrOfConversion = 1;

//      
//    if(HAL_ADC_Init(&AdcHandle2) != HAL_OK)
//    {
//        /* Initiliazation Error */
//    }

//    /*##-2- Configure ADC regular channel ######################################*/  
//    sConfig.Channel = ADC_CHANNEL_8;//
//    sConfig.Rank = 1;
//    sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;;

//    if(HAL_ADC_ConfigChannel(&AdcHandle2, &sConfig) != HAL_OK)
//    {
//        /* Channel Configuration Error */
//        Error_Handler(); 
//    }
//		
//	  HAL_ADC_Start_IT(&AdcHandle2);
//  
//}

//void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
//{
//	  if(hadc->Instance == ADC2)
//	  {
//	  		voltage = HAL_ADC_GetValue(hadc);
//	  }
//}
//static TIM_HandleTypeDef    TimHandle6;

//void TIM6_Config(void)
//{ 
//	  int uwPrescalerValue = 0;//20ms
//			
//	  /* Set TIMx instance */
//	  TimHandle6.Instance = TIM6;

//	  /* Initialize TIMx peripheral as follows:
//	       + Period = 10000 - 1
//	       + Prescaler = (SystemCoreClock/10000) - 1
//	       + ClockDivision = 0
//	       + Counter direction = Up
//	  */
//	  TimHandle6.Init.Period            = 3;//36000000
//	  TimHandle6.Init.Prescaler         = 3600;//uwPrescalerValue;
//	  TimHandle6.Init.ClockDivision     = 0;
//	  TimHandle6.Init.CounterMode       = TIM_COUNTERMODE_UP;
//	  TimHandle6.Init.RepetitionCounter = 0;
//	  //TimHandle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

//	  if (HAL_TIM_Base_Init(&TimHandle6) != HAL_OK)
//	  {
//	    //* Initialization Error */
//	    //Error_Handler();
//	  }

//	  /*##-2- Start the TIM Base generation in interrupt mode ####################*/
//	  /* Start Channel1 */
//	  if (HAL_TIM_Base_Start_IT(&TimHandle6) != HAL_OK)
//	  {
//	    /* Starting Error */
//	    //Error_Handler();
//	  }

//}

//void HAL_TIM6_PeriodElapsedCallback(void)
//{
//  int tmp_sample = 0;//²ÉÑù-Æ½¾ù
//  //uint16_t 
//  /* Cleat Interrupt Pending bit */
//  //TIM4_ClearITPendingBit(TIM4_IT_UPDATE);
//  tmp_sample = voltage - sample_AVG;
//  adc_sum += (tmp_sample * tmp_sample);
//  //adc_avg_sum += voltage;
//  cnt++;
//  if(cnt == CYCLE_CNT)
//  {
//      //sample_AVG = (uint32_t)(adc_avg_sum/CYCLE_CNT);
//      sample_RMS = (double)sqrt(adc_sum/cnt);//ÓÐÐ§Öµ
//      sample_RMS = sample_RMS*500;
//      sample_RMS = (sample_RMS * 3.3) / 4096;
//      
//      adc_avg_sum = 0;
//      cnt = 0;
//      adc_sum = 0;
//      adc_avg = 0;
//  }
//  HAL_ADC_Start_IT(&AdcHandle2);	
//}

//void adc2_init(void)
//{
//	MX_ADC2_Init();
//	TIM6_Config();
//	
//}

//void ADC1_2_IRQHandler(void)
//{
//	 HAL_ADC_IRQHandler(&AdcHandle2);
//}

//void TIM6_IRQHandler(void)
//{
//	 HAL_TIM_IRQHandler(&TimHandle6);
//}

//uint32_t get_Leakage_current(void)
//{
//	
//	
//     return (uint32_t)sample_RMS; 
//}

//void HAL_TIM6_Base_MspInit(void)
//{
//    /*##-1- Enable peripherals and GPIO Clocks #################################*/
//    /* TIMx Peripheral clock enable */
//    __TIM6_CLK_ENABLE();

//    /*##-2- Configure the NVIC for TIMx #########################################*/
//    /* Set the TIMx priority */
//    HAL_NVIC_SetPriority(TIM6_IRQn, 8, 0);

//    /* Enable the TIMx global Interrupt */
//    HAL_NVIC_EnableIRQ(TIM6_IRQn);    
//}

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

