
#ifndef __BSP_RTC_C
#define __BSP_RTC_C

/******************************************************************************
* Include Files
******************************************************************************/
#include "BSP_Rtc.h"
#include "stm32f4xx_hal_rtc.h"

/******************************************************************************
* Static Variable Definitions
******************************************************************************/


/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void RTC_CalendarConfig(void);

/******************************************************************************
* Static Function Prototype Definitions
******************************************************************************/
/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{

  while (1)
  {
		NVIC_SystemReset();
  }
}

void HAL_RTC_MspInit_LSE(RTC_HandleTypeDef *hrtc)
{
    RCC_OscInitTypeDef        RCC_OscInitStruct;
    RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

    /*##-1- Enables the PWR Clock and Enables access to the backup domain ###################################*/
    /* To change the source clock of the RTC feature (LSE, LSI), You have to:
     - Enable the power clock using __HAL_RCC_PWR_CLK_ENABLE()
     - Enable write access using HAL_PWR_EnableBkUpAccess() function before to 
       configure the RTC clock source (to be done once after reset).
     - Reset the Back up Domain using __HAL_RCC_BACKUPRESET_FORCE() and 
       __HAL_RCC_BACKUPRESET_RELEASE().
     - Configure the needed RTc clock source */
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();

    /* Enable BKP CLK for backup registers */
    __HAL_RCC_BKP_CLK_ENABLE();

    /*##-2- Configue LSE as RTC clock soucre ###################################*/
//    RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSI;//RCC_OSCILLATORTYPE_LSE;
//    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
//    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
      RCC_OscInitStruct.OscillatorType  = RCC_OSCILLATORTYPE_HSE;
      RCC_OscInitStruct.HSEState        = RCC_HSE_ON;
      RCC_OscInitStruct.HSEPredivValue  = RCC_HSE_PREDIV_DIV5;
      RCC_OscInitStruct.PLL.PLLMUL      = RCC_PLL_MUL9; 
      RCC_OscInitStruct.Prediv1Source   = RCC_CFGR2_PREDIV1SRC_PLL2;
      RCC_OscInitStruct.PLL.PLLState    = RCC_PLL_ON;
      RCC_OscInitStruct.PLL.PLLSource   = RCC_PLLSOURCE_HSE;
      RCC_OscInitStruct.PLL2.PLL2State  = RCC_PLL2_ON;  
      RCC_OscInitStruct.PLL2.HSEPrediv2Value = RCC_HSE_PREDIV2_DIV5;
      RCC_OscInitStruct.PLL2.PLL2MUL    = RCC_PLL2_MUL8;
    if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    { 
    Error_Handler();
    }

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;//RCC_RTCCLKSOURCE_LSE;
    if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    { 
    Error_Handler();
    }

    /*##-3- Enable RTC peripheral Clocks #######################################*/
    /* Enable RTC Clock */
    __HAL_RCC_RTC_ENABLE();
}
/**
  * @brief RTC MSP Initialization 
  *        This function configures the hardware resources used in this example
  * @param hrtc: RTC handle pointer
  * 
  * @note  Care must be taken when HAL_RCCEx_PeriphCLKConfig() is used to select 
  *        the RTC clock source; in this case the Backup domain will be reset in  
  *        order to modify the RTC Clock source, as consequence RTC registers (including 
  *        the backup registers) and RCC_BDCR register are set to their reset values.
  *             
  * @retval None
  */
void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc)
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;
  
  /*##-1- Configue LSI as RTC clock soucre ###################################*/ 
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();

    /* Enable BKP CLK for backup registers */
    __HAL_RCC_BKP_CLK_ENABLE();
  HAL_RCC_GetOscConfig(&RCC_OscInitStruct);
  
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.PLL2.PLL2State = RCC_PLL2_NONE;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  { 
    Error_Handler();
  }
  
  /*##-2- Enable RTC peripheral Clocks #######################################*/ 
  /* Enable RTC Clock */ 
  __HAL_RCC_RTC_ENABLE(); 
}



/**
  * @brief RTC MSP De-Initialization
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  * @param hrtc: RTC handle pointer
  * @retval None
  */
void HAL_RTC_MspDeInit(RTC_HandleTypeDef *hrtc)
{
  /*##-1- Reset peripherals ##################################################*/
  __HAL_RCC_RTC_DISABLE();

  /*##-2- Disables the PWR Clock and Disables access to the backup domain ###################################*/
  HAL_PWR_DisableBkUpAccess();
  __HAL_RCC_PWR_CLK_DISABLE();
  
  /* Disable BKP CLK for backup registers */
  __HAL_RCC_BKP_CLK_DISABLE();
}


/* Private functions ---------------------------------------------------------*/

/**
  * @brief  BSP_Rtc_Config program
  * @param  None
  * @retval None
  */
void BSP_Rtc_Config(void)
{
  /*##-1- Configure the RTC peripheral #######################################*/
  /* Configure RTC prescaler and RTC data registers */
  /* RTC configured as follow:
      - Asynch Prediv  = Calculated automatically by HAL */
  RtcHandle.Instance = RTC; 
  RtcHandle.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  RtcHandle.Init.OutPut = RTC_OUTPUTSOURCE_NONE;
  if (HAL_RTC_Init(&RtcHandle) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /*##-2- Check if Data stored in BackUp register1: No Need to reconfigure RTC#*/
  /* Read the Back Up Register 1 Data */
  if (HAL_RTCEx_BKUPRead(&RtcHandle, RTC_BKP_DR1) != 0x32F2)
  {
    /* Configure RTC Calendar */
    RTC_CalendarConfig();

    RTC_WriteTimeCounter(&RtcHandle, 1556640000);//初始化时间
  }
  else
  {
    /* Clear source Reset Flag */
    __HAL_RCC_CLEAR_RESET_FLAGS();
  }

}

/**
  * @brief  Configure the current time and date.
  * @param  None
  * @retval None
  */
static void RTC_CalendarConfig(void)
{
  RTC_DateTypeDef sdatestructure;
  RTC_TimeTypeDef stimestructure;

  /*##-1- Configure the Date #################################################*/
  /* Set Date: Tuesday February 18th 2014 */
  sdatestructure.Year = 0x14;
  sdatestructure.Month = RTC_MONTH_FEBRUARY;
  sdatestructure.Date = 0x18;
  sdatestructure.WeekDay = RTC_WEEKDAY_TUESDAY;
  
  if(HAL_RTC_SetDate(&RtcHandle,&sdatestructure,RTC_FORMAT_BCD) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /*##-2- Configure the Time #################################################*/
  /* Set Time: 02:00:00 */
  stimestructure.Hours = 0x02;
  stimestructure.Minutes = 0x00;
  stimestructure.Seconds = 0x00;

  if (HAL_RTC_SetTime(&RtcHandle, &stimestructure, RTC_FORMAT_BCD) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /*##-3- Writes a data in a RTC Backup data Register1 #######################*/
  HAL_RTCEx_BKUPWrite(&RtcHandle, RTC_BKP_DR1, 0x32F2);
}


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

