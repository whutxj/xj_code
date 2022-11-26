

/******************************************************************************
*  Copyright (C), 1988-2014, Kehua Tech. Co., Ltd.
*  File name:     Tsk_Can_Com.h
*  Author:        lx
*  Version:       V1.00
*  Created on:    2017-01-03
*  Description:   
*  History:
*      Date        Author        Modification
*    1.2014-12-15  
*    2. ...
******************************************************************************/
#ifndef TSK_CAN_COM_H
#define TSK_CAN_COM_H

/******************************************************************************
* Includes
******************************************************************************/
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "Global.h"
#include "mbdatatype.h"
#include "Sys_Config.h"
#include "Utility.h"
#include "drv_time.h"
#include "Tsk_API_Card.h"

#define ADDR_COLLECT        0x8A  



#define ID_START_CHARGE     0x01    //
#define ID_START_CHARGE_RES 0x02    //
#define ID_STOP_CHARGE      0x03    //
#define ID_STOP_RES         0x04    //
#define ID_SET_TIME         0x05    //
#define ID_SET_TIME_RES     0x06    //
#define ID_CHECK_VER        0x07    //
#define ID_VERION_RES       0x08    //
#define ID_SET_CHARGE_PARAM 0x09    //
#define ID_SET_PARAM_RES    0x0a    //
#define ID_SET_SERVER       0x0b    //
#define ID_SET_SERVER_RES   0x0c    //
#define ID_CTRL_E_LOCK      0x0d    //
#define ID_CTRL_E_LOCK_RES  0x0e    //
#define ID_ADJUST_POWER     0X1f    //
#define ID_ADJUST_POEWE_RES 0x10    //

//状态帧

#define ID_START_OVER       0x11    //
#define ID_START_OVER_RES   0x12    //
#define ID_STOP_OVER        0x13    //
#define ID_STOP_OVER_RES    0x14    //


//交流遥测
#define ID_TELEET1          0x30    //
#define ID_TELEET2          0x31    //
//交流遥信
#define ID_TEL_SIGN1            0x32    //
#define ID_TEL_SIGN2            0x33    //

#define ID_HEAT_BEAT            0x40    //
#define ID_HEAT_BEAT1           0x41    //

#define ID_ERR                  0x50    //
#define ID_ERR1                 0x51    //



#ifdef __cplusplus
     extern "C" {


#endif
#ifdef __cplusplus
}
#endif

#endif

