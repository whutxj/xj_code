
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
#ifndef TSK_IO_H
#define TSK_IO_H

/******************************************************************************
* Includes
******************************************************************************/
#include "Global.h"
#include "cmsis_os.h"
#include "main.h"
#include "Tsk_API_LCD.h"
/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/

typedef enum
{
    ONCE_RING,                 //蜂鸣器响一次
    LAST_RING,                  //蜂鸣器持续响
    BEEP_OFF,                   //停止响
}BEEP_CTRL;


typedef enum
{
    SWITCH_DISCONNECT,       //通知断开
    SWITCH_DISCON_IM,        //立即断开
    SWITCH_CLOSE,            //闭合
}SWITCH_CTRL;


typedef enum
{
    LOCK_CTRL_NULL,
    LOCK_GUN,                 //立即锁定
    UNLOCK_GUN,               //立即断开锁定
}LOCK_CTRL;

#ifdef __cplusplus
 extern "C" {
#endif
     

/******************************************************************************
* Function Prototype Definitions
******************************************************************************/
void API_Led_Ctrl_Fun_HP(uint8 Ch);  
void Notice_Beep(uint8 BeepState,uint8 Ch);
void Notice_Switch(uint8 SwitchState,uint8 Ch);
void Notice_Ctrl_Lock(uint8 LockState,uint8 Ch);
void API_Alram_Probe(uint8 Ch);
void API_Act_Function(uint8 Ch);
void API_Led_Ctrl_Fun(uint8 Ch);
uint8 Get_Car_Link(uint8 Ch);
uint8 Get_Lock_Seat(uint8 Ch);
uint8 Notice_Lcd_State(uint8 LcdState,uint8 Ch);
   
     
#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */


