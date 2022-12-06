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
#ifndef __BSP_RTC_H
#define __BSP_RTC_H

/*****************************************************************************/
#ifdef __cplusplus
 extern "C" {
#endif

/******************************************************************************
* Includes
******************************************************************************/
#include "main.h"
     
#ifndef __BSP_RTC_C
#define BSP_RTC_EXTERN  extern
#else
#define BSP_RTC_EXTERN
#endif

BSP_RTC_EXTERN RTC_HandleTypeDef RtcHandle;

/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/
BSP_RTC_EXTERN void BSP_Rtc_Config(void);  


#ifdef __cplusplus
	}
#endif
	
#endif

