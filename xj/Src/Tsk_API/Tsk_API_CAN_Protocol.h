
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
#ifndef TSK_API_CAN_H
#define TSK_API_CAN_H

/******************************************************************************
* Includes
******************************************************************************/
#include "main.h"
#include "cmsis_os.h"
#include "Global.h"
#include "Utility.h"
#include "bsp_can.h"


/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/

#define  PORT_NUM  1


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
uint8 API_Send_Single(uint8 P,uint8 PF,uint8 PS,uint8 SA,uint8 *Data,uint8 Len);
void API_Start_Send_Multi(uint8 P,uint8 PF,uint8 PS,uint8 SA,int Ch);
uint8 API_Parse_Multi(uint8 *RcvBuf,uint32 ID,uint8 *Data,uint16 Len,int Ch);
/*------------------------------------------------------------
操作目的：   发送 多包 需要周期调用
作    者：      
操作结果：   
函数参数：Data 数据 ，Len长度，Ch通道
            
返回值：
            STEP_START 开始发送第一包
            STEP_ING 正在发送
            STEP_END 发送结束 可以发送下一包
版   本:    V1.0

记   录:

------------------------------------------------------------*/
uint8 API_Send_Multi(uint8 *Data,uint16 Len,int Ch);
#ifdef __cplusplus
}
#endif

#endif

