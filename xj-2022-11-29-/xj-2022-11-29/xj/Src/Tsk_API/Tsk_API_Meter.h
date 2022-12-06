
/******************************************************************************
*  
*  File name:     Tsk_Meter.h
*  Author:        
*  Version:       V1.00
*  Created on:    2017-01-03
*  Description:   
*  History:
*      Date        Author        Modification
*    1.2017-01-03  
*    2. ...
******************************************************************************/
#ifndef TSK_METER_API_H
#define TSK_METER_API_H

/******************************************************************************
* Includes
******************************************************************************/
#include "main.h"
#include "cmsis_os.h"
#include "Global.h"
#include "Utility.h"
#include "Tsk_Measure.h"
/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/


/******************************************************************************
* Data Type Definitions
******************************************************************************/

/******************************************************************************
* Global Variable Definitions
******************************************************************************/

#ifdef __cplusplus
 extern "C" {
#endif
/******************************************************************************
* Function Prototype Definitions
******************************************************************************/
MEASURE_DATA Get_Meter_Data(uint8 Ch);
void API_Meter_Init(void);
void API_Meter_Task(void);

uint8  Get_Meter_Comm(uint8 Ch);
extern MEASURE_DATA g_Meate_Data[2];

#ifdef __cplusplus
}
#endif

#endif

