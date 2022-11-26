/******************************************************************************
*  
*  File name:     LED_Function.h
*  Author:        
*  Version:       V1.00
*  Created on:    2016-12-15
*  Description:  <time.h> 里的函数重定义
*  History:
*      Date        Author        Modification
*    1.2014-12-15  
*    2. ...
******************************************************************************/
#ifndef __DRV_TIME_H
#define __DRV_TIME_H


#ifdef __cplusplus
 extern "C" {
#endif

/******************************************************************************
* Includes
******************************************************************************/

#include "main.h"
#include <time.h>
//#include "BSP_RTC.h"   

/*****************************************************************************/     
int drv_set_sys_time(struct tm tm_val);

#ifdef __cplusplus
            }
#endif
	 
#endif
