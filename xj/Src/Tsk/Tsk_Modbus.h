/******************************************************************************
*  
*  File name:     Tsk_Modbus.h
*  
*  Version:       V1.00
*  Created on:    2017-01-03
*  Description:   
*  History:
*      Date        Author        Modification
*    1.2014-12-15  
*    2. ...
******************************************************************************/


#ifndef __TASK_MODBUS_H
#define __TASK_MODBUS_H

#ifdef __cplusplus
 extern "C" {
#endif

#ifndef __TASK_MODBUS_C
#define TASK_MODBUS_EXTERN  extern
#else
#define TASK_MODBUS_EXTERN
#endif
/******************************************************************************
* Includes
******************************************************************************/

#include "cmsis_os.h"

#include "Tsk_Charge_Ctrl.h"


#define RET_HISTORY 0x11
#define RET_ALARM 0x21
#define RET_REAL  0x31
#define RET_CARD  0x41


/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/
typedef enum
{
    MSG_MAST_CLOUD_SEND_HISTORY,
    MSG_MAST_CLOUD_SEND_ALARM,
    MSG_MAST_CLOUD_SEND_REAL,
    MSG_MAST_CLOUD_AUTH_CARD,
    
}MSG_MAST_TYPE;

typedef enum
{
    MSG_MAST_CLOUD_RET_HISTORY,
    MSG_MAST_CLOUD_RET_ALARM,
    MSG_MAST_CLOUD_RET_REAL,
    MSG_MAST_CLOUD_RET_CARD,
    MSG_MAST_CLOUD_ADJ_TIME,
    MSG_MAST_CLOUD_SET_FEE,
    MSG_MAST_CLOUD_CTRL_ON,
    MSG_MAST_CLOUD_CTRL_OFF,
    MSG_MAST_CLOUD_BOOK,
    
}MSG_CLOUD_TYPE;

typedef enum
{
    CTRL_ON,//开机
    CTRL_OFF,//关机
    BOOK_CHARGE,//预约
    BOOK_CANLE_CHARGE,//取消预约
    AUTH_CARD_OK,//认证卡成功
    AUTH_CARD_ERR,//认证卡成功
}REMOTE_CTRL_TYPE;
																										
/******************************************************************************
* Data Type Definitions
******************************************************************************/


/******************************************************************************
* Global Variable Definitions
******************************************************************************/

/******************************************************************************
* Function Prototype Definitions
******************************************************************************/
TASK_MODBUS_EXTERN void SetupModbusApp(osPriority ThreadPriority,uint8_t StackNum);
TASK_MODBUS_EXTERN osThreadId Get_Modbus_Task_Id(void);

//TASK_MODBUS_EXTERN UsrAccount st_UsrAccount;
TASK_MODBUS_EXTERN uint8 g_ModbusCom;
TASK_MODBUS_EXTERN uint32 CommTick;
#ifdef __cplusplus
}
#endif

#endif /* __TASK_SPIFLASH_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

