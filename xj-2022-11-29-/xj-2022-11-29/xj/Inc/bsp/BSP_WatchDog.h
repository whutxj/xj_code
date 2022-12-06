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
#ifndef __BSP_WATCH_DOG_H
#define __BSP_WATCH_DOG_H

/*****************************************************************************/
#ifdef __cplusplus
 extern "C" {
#endif

/******************************************************************************
* Includes
******************************************************************************/
#include "main.h"
     
#ifndef __BSP_WATCH_DOG_C
#define BSP_ADC_EXTERN  extern
#else
#define BSP_ADC_EXTERN
#endif

void BSP_WatchDog_Init(void);
void BSP_WatchDog_Refresh(void);


#ifdef __cplusplus
	}
#endif
	
#endif

