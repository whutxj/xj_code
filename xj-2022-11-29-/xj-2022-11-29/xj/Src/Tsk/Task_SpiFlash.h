/******************************************************************************
*  
*  File name:     Task_SpiFlash.h
*  
*  Version:       V1.00
*  Created on:    2017-01-03
*  Description:   
*  History:
*      Date        Author        Modification
*    1.2014-12-15  
*    2. ...
******************************************************************************/


#ifndef __TASK_SPIFLASH_H
#define __TASK_SPIFLASH_H

#ifdef __cplusplus
 extern "C" {
#endif

#ifndef __TASK_SPIFLASH_C
#define TASK_SPIFLASH_EXTERN  extern
#else
#define TASK_SPIFLASH_EXTERN
#endif
/******************************************************************************
* Includes
******************************************************************************/
//#include "bsp_driver_sd.h"
#include "cmsis_os.h"
#include "Task_API_SpiFlash.h"
#include "Update.h"
#include "Tsk_Charge_Ctrl.h"
/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/

																										
/******************************************************************************
* Data Type Definitions
******************************************************************************/

/*记录类型枚举定义*/
typedef enum{
    TYPE_SYSPARA    = (uint8_t)0,//系统参数
    TYPE_REALDATA,              //充电实时信息
    TYPE_UPDATE,                //升级信息
	TYPE_FACTORY_PARAM,
    TYPE_METERPARA,             //表参数
}Method1_Type_Enum;

typedef enum{
    RECORD_GRIDTIE    = (uint8_t)0,
    RECORD_USERLOG,
    RECORD_HISTORY,
    RECORD_INNERHISTORY,
    RECORD_POWERSCHEDULE,
}Method2_Type_Enum;

typedef enum{
    TYPE_ENERGYDAY    = (uint8_t)0,
    TYPE_ENERGYMONTH,
}Method3_Type_Enum;

typedef enum{
    TYPE_CHARGELOG    = (uint8_t)0,
    TYPE_ALARMLOG     = (uint8_t)1,
    TYPE_UN_PAY       = (uint8_t)2,
    TYPE_OTHERLOG,
}Method4_Type_Enum;


/******************************************************************************
* Global Variable Definitions
******************************************************************************/
TASK_SPIFLASH_EXTERN ChargeRecord  HistoryCharge;
TASK_SPIFLASH_EXTERN uint32  HistoryChargeIndex;
TASK_SPIFLASH_EXTERN ALARM_DATA   HistoryAlarm;
TASK_SPIFLASH_EXTERN uint32  HistoryAlarmIndex;//历史告警
TASK_SPIFLASH_EXTERN uint32  HistoryAlarmNum;//总告警数量条数
TASK_SPIFLASH_EXTERN uint32  HistoryChargeNum;//总充电记录条数

/******************************************************************************
* Function Prototype Definitions
******************************************************************************/
TASK_SPIFLASH_EXTERN void SetupSpiFlashApp(osPriority ThreadPriority,uint8_t StackNum);
TASK_SPIFLASH_EXTERN void API_Save_Sys_Param(void);
TASK_SPIFLASH_EXTERN void MEM_Init_Record(void);
TASK_SPIFLASH_EXTERN void Delet_All_Record(void);
TASK_SPIFLASH_EXTERN void Notice_Updata_Info(UpdateInfo *pUpdateInfo);
TASK_SPIFLASH_EXTERN void Save_To_Factory_Param(void);
TASK_SPIFLASH_EXTERN void Restore_Factory_Param(void);
TASK_SPIFLASH_EXTERN void Delet_All_Record(void);
TASK_SPIFLASH_EXTERN void Save_UpPay_Info(uint8 Ch);
TASK_SPIFLASH_EXTERN uint8 Del_Un_Pay_UsrInfo(ChargeRecord *pChargeRecordData);
TASK_SPIFLASH_EXTERN uint8 Get_Un_Pay_UsrInfo(ChargeRecord *pChargeRecordData,uint8 *CardNo);
TASK_SPIFLASH_EXTERN void Get_Firmware_Ver(char *p);
TASK_SPIFLASH_EXTERN void UnReport_Data_init(void);
TASK_SPIFLASH_EXTERN unsigned char Get_UnReport_Data(int type,uint8* pData,int Ch);
TASK_SPIFLASH_EXTERN void API_Save_Sys_Param_Direct(void);
TASK_SPIFLASH_EXTERN void Save_Start_Charge_Info(uint8 Ch);
TASK_SPIFLASH_EXTERN void Save_End_Charge_Opt_Info(uint8 Ch);
void Updata_Secces_Proc(void);
uint8 Get_Updata_Flag_And_Clear(void);
#ifdef __cplusplus
}
#endif
//void Notice_Updata_Info(UpdateInfo *pUpdateInfo);
#endif /* __TASK_SPIFLASH_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
