/******************************************************************************
*
*  File name:     TSK_JWT.h
*  Author:
*  Version:       V1.00
*  Created on:    2018-12-19
*  Description:
*  History:
*      Date        Author        Modification
*    1.2018-12-19
*    2. ...
******************************************************************************/
#ifndef TSK_JWT_H
#define TSK_JWT_H

#ifdef __cplusplus
 extern "C" {
#endif
/******************************************************************************
* Includes
******************************************************************************/
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "Global.h"
#include "Utility.h"
#include <time.h>
#include "Tsk_Api_Card.h"
#include "Tsk_API_Alarm.h"
#include "Tsk_API_LCD.h"
#include "CRC16.h"
#include "DataBase.h"
#include "Tsk_API_Meter.h"
#include "Tsk_API_adc.h"

#define Max_fee_num  12


typedef struct
{
  uint8  card_start_rx_flag;
  uint8  card_stop_rx_flag;
  uint8  heart_beat_time[6];
  uint8  billing_rule_id[4];
  uint8  billing_rule_version[4];
  uint8  billing_effect_time[6];
  uint16 login_NO;
	uint16 bill_serial_NO;
	uint16 heart_beat_NO;
	uint16 progress_NO;
	uint16 progress_data_NO;
	uint8_t       order_NO[8];
  uint32_t      sharp_E;
  uint32_t      peak_E;
  uint32_t      flat_E;
  uint32_t      valley_E;
  uint32_t      eletricity_fee;
  uint32_t      sharp_money;
  uint32_t      peak_money;
  uint32_t      flat_money;
  uint32_t      valley_money;
}
JWT_info_t;

typedef struct TMemBufferEvent
{
  uint8                      *ptr;

  uint16                      length;
  uint8                       data[300];
}
membuffer_event_t;

typedef enum
{
  JWT_card_start,
  JWT_card_end,
  JWT_APP_start_success,
  JWT_APP_start_fail,
  JWT_charge_record,
  JWT_charge_progress,
  JWT_charge_progress_data,
  JWT_warning,
  JWT_QR_code,
  JWT_gun_idle,
  JWT_gun_plug_in,
  JWT_gun_charging,
  JWT_gun_finish,
  JWT_gun_book,
  JWT_gun_fault,
  JWT_gun_status_change,
}JWT_cmd;

typedef struct
{
  uint8  feenum;
  uint8  fee_stype[Max_fee_num];
  uint8  fee_start_time[Max_fee_num][6];
  uint8  fee_end_time[Max_fee_num][6];
  uint16 elefee[Max_fee_num];
  uint16 serfee[Max_fee_num];
  uint32 stage_energy[Max_fee_num];
  uint32 stage_elefee[Max_fee_num];
  uint32 stage_serfee[Max_fee_num];
}
JWT_fee_info;

typedef struct
{
  uint8  feenum;
  uint8  fee_start_time[Max_fee_num][2];
  uint8  fee_end_time[Max_fee_num][2];
  uint8  fee_time_value[48];
}
JWT_rx_fee_info;

typedef enum
{
    CustomCloud_CMD_LOGIN                         =   0x01,       // 登陆
    CustomCloud_CMD_LOGIN_RET                     =   0x81,       // 登陆回复

    CustomCloud_CMD_CARD_START_CRG                =   0x04,       // 开始充电
    CustomCloud_CMD_CARD_START_CRG_RET            =   0x84,       // 开始充电回复

    CustomCloud_CMD_CARD_STOP_CRG                 =   0x05,       //
    CustomCloud_CMD_CARD_STOP_CRG_RET             =   0x85,       //

    CustomCloud_CMD_CTRL_START_CRG                =   0x06,       // 后台启动充电
    CustomCloud_CMD_CTRL_START_CRG_RET            =   0x86,       // 回复启动充电结果

    CustomCloud_CMD_CTRL_STOP_CRG                 =   0x07,       // 后台结束充电
    CustomCloud_CMD_CTRL_STOP_CRG_RET             =   0x87,       // 回复结束充电结果


    CustomCloud_CMD_CRGRECORD_REPORT              =   0x08,       // 充电记录上传
    CustomCloud_CMD_CRGRECORD_REPORT_RET          =   0x88,       // 充电记录上传回复


    CustomCloud_CMD_SEND_FAULT                    =   0x0d,       // 桩主动故障信息
    CustomCloud_CMD_SEND_FAULT_RET                =   0x8d,       // 告警回复

    CustomCloud_CMD_CRG_REALDATA                  =   0x09,       //充电进度

    CustomCloud_CMD_MONIROT_DATA                  =   0x0A,       //监控数据
    CustomCloud_CMD_HEART                         =   0X0C,       // 心跳
    CustomCloud_CMD_HEART_RET                     =   0X8C,       // 心跳回复

    CustomCloud_CMD_SEND_GUN_STATUS   		      =   0x1F,
    CustomCloud_CMD_SEND_VIN           		      =   0x20,       //
    CustomCloud_CMD_RECV_VIN           		      =   0xA0,       //

    CustomCloud_CMD_SET_QRCode           		  =   0x1C,       // 桩请求设置二维码
    CustomCloud_CMD_SET_QRCode_RET          	  =   0x9C,       // 后台回复二维码

    CustomCloud_CMD_CTRL_REEBOOT,
    CustomCloud_CMD_SET_FEE_0B                    =   0x0b,       // 计费规则
    CustomCloud_CMD_SET_FEE_RET_8B                =   0x8b,       // 计费规则

    CustomCloud_CMD_SET_FEE_1B                    =   0x1b,       // 分段计费规则
    CustomCloud_CMD_SET_FEE_RET_9B                =   0x9b,       // 分段计费规则

    CustomCloud_CMD_SET_TIME                      =   0x0e,       // 后台预约充电
    CustomCloud_CMD_SET_TIME_RET                  =   0x8e,       // 后台预约充电回复
    CustomCloud_CMD_UPDATA_SOFT                   =   0x1E,       // 后台升级请求
    CustomCloud_CMD_UPDATA_SOFT_RET               =   0x9e,       // 后台请求返回升级
    CustomCloud_CMD_SET_BLACILIST_RET             =   0x00,

    CustomCloud_CMD_REBOOT                        =   0x1d,       // 后台重启请求
    CustomCloud_CMD_REBOOT_RET                    =   0x9d,       // 后台请求返回重启

    CustomCloud_CMD_updata_balance                =   0x21,       // 余额更新
    CustomCloud_CMD_updata_balance_RET            =   0xA1,       // 余额更新回复

    CustomCloud_CMD_UPDATA_SOFT_TCP				  = 	0xA2, 	  // 后台TCP升级请求
    CustomCloud_CMD_UPDATA_SOFT_TCP_RET			  = 	0x22, 	  // 充电桩应答升级请求
    CustomCloud_CMD_SEND_UPDATA_FILE			  = 	0xA3, 	  // 后台TCP下发升级文件
    CustomCloud_CMD_SEND_UPDATA_FILE_RET		  = 	0x23, 	  // 充电桩应答接收升级文件
    CustomCloud_CMD_SEND_UPDATA_RESULT_TCP 		  = 	0x24, 	  // 充电桩上传升级结果
    CustomCloud_CMD_SEND_UPDATA_RESULT_TCP_RET 	  = 	0xA4, 	  // 后台TCP应答升级结果

    CustomCloud_CMD_UPDATA_SOFT_FTP				  = 	0xA5, 	  // 后台FTP升级请求
    CustomCloud_CMD_UPDATA_SOFT_FTP_RET			  = 	0x25, 	  // 充电桩应答升级请求
    CustomCloud_CMD_SEND_UPDATA_RESULT_FTP 		  = 	0x26, 	  // 充电桩上传升级结果
    CustomCloud_CMD_SEND_UPDATA_RESULT_FTP_RET 	  = 	0xA6, 	  // 后台FTP应答升级结果

}CustomCloud_RPO_CMD;



uint8* JWT_tx_0x01_login(uint8* data);
uint8* JWT_tx_0x0C_heart_beat(uint8 ch, uint8* data);
uint8* JWT_tx_0x04_card_charge_start(uint8 ch, uint8* data);
void   JWT_rx_0x06_app_start_process(uint8 *data);
uint8  Ctrl_send_charger_record(uint8 Ch);

uint8* JWT_tx_0x86_app_charge_start_response(uint8 ch, uint8 result, uint16 serial_NO, uint8* data);
uint8* JWT_tx_0x87_app_charge_stop_response(uint8 ch, uint8 result, uint16 serial_NO, uint8* data);
uint8* JWT_tx_setting_response(uint8 cmd, uint8 result, uint8* data);
void   JWT_rx_0x0B_set_billing_rule_process(uint8 *data);
uint8* JWT_tx_0x08_charge_record(uint8 ch, uint8* data);
uint8* JWT_tx_0x8B_billing_rule_set_response(uint8* data);
uint8* JWT_tx_0x0B_billing_rule_query_response(uint8* data);

uint16 JWT_daemon_rx_message_query(uint8 *data,int RcvLen);
uint8  JWT_rx_handler(uint8 *data);
void   uart_net_daemon(void);
void   Ctrl_card_charge(uint8 Ch, uint8 control, uint8 *card_NO);
void   Ctrl_send_warning_message(uint8 Ch, uint16 warning_code, uint8 warning_status, uint32 warning_value);

void 	 MT_4G_module_tx_0x2B_0x2B_0x2B(void);
void 	 MT_4G_module_tx_a(void);
void 	 MT_4G_module_tx_AT_Z_reset(void);
void 	 MT_4G_module_setting_IP_port_NO(void);
void Cloud_Task(const void * p);


extern JWT_info_t	g_JWT_info_t;
extern RecordData g_RecordData;

#ifdef __cplusplus
}
#endif

#endif
