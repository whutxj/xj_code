
/******************************************************************************
*  
*  File name:     Tsk_Charge_Ctrl.h
*  Author:        
*  Version:       V1.00
*  Created on:    2017-01-05
*  Description:   
*  History:
*      Date        Author        Modification
*    1.2017-01-03  
*    2. ...
******************************************************************************/
#ifndef TSK_ADC_H
#define TSK_ADC_H

/******************************************************************************
* Includes
******************************************************************************/
#include "Global.h"
#include "cmsis_os.h"
#include "main.h"
#include "Tsk_API_IO_Ctrl.h"
/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/


#ifdef __cplusplus
 extern "C" {
#endif
/******************************************************************************
* Function Prototype Definitions
******************************************************************************/


#ifdef __cplusplus
}
#endif
int Get_CP_Sample(uint8_t Ch);

int Get_system_Temp(void);

int Get_Gun_Temp(void);
#endif /* __MAIN_H */


