/******************************************************************************
*  Copyright (C), 1988-2014, Kehua Tech. Co., Ltd.
*  File name:     Tsk_API_Card.c
*  Author:        lx
*  Version:       V1.00
*  Created on:    2016-12-22
*  Description:   
*  History:
*      Date        Author        Modification
*    1.2014-12-15  
*    2. ...
******************************************************************************/
#ifndef _TSK_API_CARD628_H
#define _TSK_API_CARD628_H

/******************************************************************************
* Include Files
******************************************************************************/
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "Global.h"
#include "Drv_MT_Card.h"

#include "Tsk_API_Card.h"
#define MIN_ALLOW_MONEY    200 //最小允许充电值 

/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/


/******************************************************************************
* Static Function Prototype Definitions
******************************************************************************/
START_CRG_OPT_CARD MT628_Start_Charge_Card(CARD_DATA *PCardData,uint8 Ch);
END_OPT_CARD MT628_End_Charge_Card(CARD_RECORD Record,int *Balance,int Ch);
END_OPT_CARD MT628_Add_Charge_Card(CARD_RECORD Record,int Ch);
uint8 Get_CardCommStatu_MT628(void);
uint8 Set_Charge_Card_No_MT628(uint8 *CardNo,uint8 Ch);
uint8 MT628_Search_Card(CARD_DATA *PCardData);
#endif
