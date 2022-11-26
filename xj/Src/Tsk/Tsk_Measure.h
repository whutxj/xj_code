
/******************************************************************************
*  
*  File name:     Tsk_Measure.h
*  Author:        
*  Version:       V1.00
*  Created on:    2017-01-03
*  Description:   
*  History:
*      Date        Author        Modification
*    1.2017-01-03  
*    2. ...
******************************************************************************/
#ifndef TSK_METER_H
#define TSK_METER_H

/******************************************************************************
* Includes
******************************************************************************/
#include "main.h"
#include "cmsis_os.h"
#include "Global.h"
#include "Utility.h"

/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/


/******************************************************************************
* Data Type Definitions
******************************************************************************/

typedef struct
{
    
    U32 Totol_Energy;   //有功总电能    放大 100倍
    I16  VolageA;       //A相电压       放大 10倍
    I16  VolageB;       //B相电压
    I16  VolageC;       //C相电压
    I16  CurrentA;      //A相电流      
    I16  CurrentB;      //B相电流      放大 100倍
    I16  CurrentC;      //C相电流       放大 100倍
}MEASURE_DATA;

/******************************************************************************
* Global Variable Definitions
******************************************************************************/

#ifdef __cplusplus
 extern "C" {
#endif
/******************************************************************************
* Function Prototype Definitions
******************************************************************************/
void SetupMeasureApp(osPriority ThreadPriority,uint8_t StackNum);
MEASURE_DATA Get_Measrue_Data(uint8 Ch);
osThreadId Get_Measrue_Task_ID(void);
#ifdef __cplusplus
}
#endif

#endif

