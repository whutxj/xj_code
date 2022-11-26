
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
#ifndef TSK_ALARM_H
#define TSK_ALARM_H

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
typedef enum
{
    MSG_CREAT_START_ALRM_INFO,  //创建开始告警
    MSG_UPDATA_END_ALRM_INFO,   //更新结束告警
    MSG_UPDATA_REPORT_ALRM_INFO,   //上报标记

    MSG_CREAT_START_CHARGE_LOG,  //创建开始充电
    MSG_UPDATA_END_CHARGE_LOG,   //更新结束充电
    MSG_UPDATA_REPORT_CHARGE_LOG,   //上报标记
    MSG_UPDATA_REPOR_CHARGE_LOG_HP,//更新上报标记
    MSG_SAVE_SYS_PARAM,
    MSG_SAVE_FACTOR_PARAM,//保存出厂参数
    MSG_RESTORE_FACTORY_PARAM,//恢复出厂参数
    MSG_SAVE_UNPAY_CHARGE_LOG,//
    MSG_READ_ALARM_LOG,
    MSG_READ_CHARGE_LOG,
    MSG_SAVE_SYS_PARAM_DRIECT,
}MSG_ALARM_OPT;

/******************************************************************************
* Data Type Definitions
******************************************************************************/

typedef enum
{
	//ALARM_REASON_NONE, 告警编码 与显示屏故障字符串 AlarmText 对应
	ALARM_REASON_BMS_COMM,      //bms 通讯
	ALARM_REASON_EMERGENCY,     //急停
	ALARM_REASON_OUTPUT_OVER_V, //输出过压
	ALARM_REASON_OUTPUT_OVER_I, //输出过流
	ALARM_REASON_BAT_CONVERSE_CONNECT,  //电池反接
	ALARM_REASON_BAT_CELL_OVER_V,       //单体电压过高
	ALARM_REASON_BAT_OVER_TEMP,         //单体温度过高
	ALARM_REASON_CONNECT,               //车连接故障
	ALARM_REASON_SISTEM_TEMP,           //系统温度
	ALARM_REASON_AC_INPUT_OVER_V,       //交流输入过压
	ALARM_REASON_AC_INPUT_LOW_V,        //交流输入欠压
    ALARM_REASON_AC_OUT_PUT_ONET_I,     //交流输出过流
	ALARM_REASON_ISOLATION,             //绝缘故障
	ALARM_REASON_ISOLATION_N,           //负绝缘故障
	ALARM_REASON_OUTPUT_CONNECT_DEVICE_OVER_TEMP,//输出连接器过温
	ALARM_REASON_BMS_PART_OVER_TEMP,        //bms元件过温
	ALARM_REASON_OUTPUT_CONNECT_DEVICE_FAIL,//输出连接器故障
	ALARM_REASON_BAT_GROUP_OVER_TEMP,       //电池组温度过高
	ALARM_REASON_OUTPUT_OVER_NEED_I,        //电流超过需求值
	ALARM_REASON_I_NO_NORMAL,               //电流异常
	ALARM_REASON_PARAM_OVER_TIME,//
    ALARM_REASON_METER,         //表通讯
	ALARM_REASON_CARD,          //读卡器通讯
	ALARM_REASON_OUTPUT_CONTACT,//接触器
 	ALARM_REASON_OUTPUT_FUSE,   //熔断器断开
    ALARM_REASON_THUNDER,       //防雷器故障
    ALARM_REASON_TOUCH,         //触摸屏
    ALARM_REASON_TEMP_SENSOR,   //系统温度传感器
    ALARM_REASON_CRG_MOD_COMM,  //充电模块通讯告警
    ALARM_REASON_GUN_TEMP_ALRM, //充电枪温度
    ALARM_REASON_DOOR_OPEN,     //维修门开
    ALARM_REASON_OUT_SHORT_OPEN,//输出短路
    ALARM_REASON_PE_CONNECT,   //地线未接告警
    ALARM_REASON_ELC_LOCK,      //电子锁异常
    ALARM_REASON_BAIS,
    ALARM_REASON_GUN_OVER_TEMP, //充电枪过温
    ALARM_REASON_LOSE_PHASE,	//缺相
    ALARM_REASON_LEAKAGE_CURRENT,//漏电流
    ALARM_REASON_MAX ,//
    
}ALARM_REASON;
#pragma pack(1)
typedef struct
{
    uint8 AlarmCode;        //告警编码
    uint32 StartTime;       //告警开始时间
    uint32 EndTime;         //告警结束时间
    uint8 Recover;          //恢复标记 0 恢复 非0 没有恢复
    uint8 ReportF;          //上报标识   
    uint8 REVERSE0;        //预留空间
}ALARM_DATA;
#pragma pack()

#ifdef __cplusplus
 extern "C" {
#endif
/******************************************************************************
* Function Prototype Definitions
******************************************************************************/
void SetupAlarmProbeActApp(osPriority ThreadPriority,uint8_t StackNum);
uint32 Get_Current_Alarm_Time(uint8 alarmID,uint8 Ch);

uint64 Get_FaultBit(uint8 Ch);
uint8 Get_Alarm_State(uint8 Ch);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */


