
/******************************************************************************
* Include Files
******************************************************************************/
#include "main.h"
#include "drv_time.h"

/******************************************************************************
* Static Variable Definitions
******************************************************************************/



/**
  * @brief  Read the time counter available in RTC_CNT registers.
  * @param  hrtc   pointer to a RTC_HandleTypeDef structure that contains
  *                the configuration information for RTC.
  * @retval Time counter
  */
uint32_t RTC_ReadTimeCounter(RTC_HandleTypeDef* hrtc)
{
  uint16_t high1 = 0, high2 = 0, low = 0;
  uint32_t timecounter = 0;

//  high1 = READ_REG(hrtc->Instance->CNTH & RTC_CNTH_RTC_CNT);
//  low   = READ_REG(hrtc->Instance->CNTL & RTC_CNTL_RTC_CNT);
//  high2 = READ_REG(hrtc->Instance->CNTH & RTC_CNTH_RTC_CNT);

//  if (high1 != high2)
//  { /* In this case the counter roll over during reading of CNTL and CNTH registers, 
//       read again CNTL register then return the counter value */
//    timecounter = (((uint32_t) high2 << 16 ) | READ_REG(hrtc->Instance->CNTL & RTC_CNTL_RTC_CNT));
//  }
//  else
//  { /* No counter roll over during reading of CNTL and CNTH registers, counter 
//       value is equal to first value of CNTL and CNTH */
//    timecounter = (((uint32_t) high1 << 16 ) | low);
//  }

  return timecounter;
}

/**
  * @brief  Write the time counter in RTC_CNT registers.
  * @param  hrtc   pointer to a RTC_HandleTypeDef structure that contains
  *                the configuration information for RTC.
  * @param  TimeCounter: Counter to write in RTC_CNT registers
  * @retval HAL status
  */
HAL_StatusTypeDef RTC_WriteTimeCounter(RTC_HandleTypeDef* hrtc, uint32_t TimeCounter)
{
  HAL_StatusTypeDef status = HAL_OK;
//  
//  /* Set Initialization mode */
//  if(RTC_EnterInitMode(hrtc) != HAL_OK)
//  {
//    status = HAL_ERROR;
//  } 
//  else
//  {
//    /* Set RTC COUNTER MSB word */
//    WRITE_REG(hrtc->Instance->CNTH, (TimeCounter >> 16));
//    /* Set RTC COUNTER LSB word */
//    WRITE_REG(hrtc->Instance->CNTL, (TimeCounter & RTC_CNTL_RTC_CNT));
//    
//    /* Wait for synchro */
//    if(RTC_ExitInitMode(hrtc) != HAL_OK)
//    {       
//      status = HAL_ERROR;
//    }
//  }

  return status;
}

/******************************************************************************
* Static Function Prototype Definitions
******************************************************************************/

time_t time(time_t *tm)
{

  
    RTC_HandleTypeDef *hrtc1 = &hrtc;

    
    if(tm)
    {
        *tm = RTC_ReadTimeCounter(hrtc1);
    }
    
    return RTC_ReadTimeCounter(hrtc1);
}

int drv_set_sys_time(struct tm tm_val)
{
 
    RTC_HandleTypeDef *hrtc1 = &hrtc;
    time_t time = mktime(&tm_val);


    RTC_WriteTimeCounter(hrtc1, time);
    
    return 0;
}

struct tm *gmtime(const time_t * sec)
{
    time_t newTime = 0;
    
    newTime  = (*sec) + 8 * 3600;
    
    return localtime(&newTime);
}
