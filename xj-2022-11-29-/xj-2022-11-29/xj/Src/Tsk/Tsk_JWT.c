/******************************************************************************
* Include Files
******************************************************************************/
#include "Tsk_JWT.h"
#include "Sys_Config.h"
#include <string.h>
#include <stdio.h>
#include "Tsk_Charge_Ctrl.h"
#include "MsgQueue.h"
#include "lwip/sockets.h"
#include "eth_comm.h"
#include "string.h"
#include "lwip.h"
#include "lwip/sockets.h"
#include <stdio.h>
#include "lwip/netdb.h"
#include <string.h>
#include "lwip/api.h"
#include "Utility.h"
#include "Tsk_Charge_Ctrl.h"
#include "Tsk_Cloud_Com.h"
#include "Task_SpiFlash.h"
#include "stdlib.h"
/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/
#define JWT_STX           0x68
#define protocol_version  100
#define net_uart_port     COM4
//#define net_uart_port     COM2

#define rx_buff_length    300
#define net_uart_port_2G     COM2
#define JWT_link_eth 0
#define JWT_link_2G  1
#define JWT_link_4G  2

#define JWT_link_way JWT_link_eth

//32、33报文是否启用宏
#define JWT_32_33  0



typedef struct
{
    uint8_t *SendBuf;
    uint8_t RcvBuf[300];
    uint16_t RcvLen;
    uint16_t SendLen;

    uint8_t Port;
    uint8_t connectType;
    uint32_t heartTick;
    int (*Read)(uint8_t Port,uint8_t *buf,uint16_t len,int timeout);
    int (*Send)(uint8_t Port,uint8_t *buf,uint16_t len,int timeout);

}Cloud_Comm_Info;

static Cloud_Comm_Info comm;
u8_t loginStatus = 0;

//#define UART2_rx_enable()   (GPIOD->CRL &= 0xFFCFFFFF)
//#define UART2_tx_enable()   (GPIOD->CRL |= 0x00300000)
/******************************************************************************
* Global Variable Definitions
******************************************************************************/
JWT_info_t 				 g_JWT_info_t;
DeviceInfo 				 g_DeviceInfo;
RecordData         g_RecordData;
ChargeRecord st_Record;//正在上传的

sysRunningDataBase g_sysRunningDataBase;
UpdateInfo JWT_UpdateInfo;
static TCP_UpdateInfo TCP_Info;
static uint8_t QR_flag = 0;
static JWT_fee_info stage_fee;
static JWT_rx_fee_info rx_stage_fee;
static uint8 first_conect_byLAN =0;

//for test by LLZ-22-04-15
static uint32 login_num = 0;//用来记录登录次数
static uint32 reset_num = 0;//用来记录重启次数



uint16 JWT_daemon_rx_message_query(uint8 *data,int RcvLen);

uint8* JWT_tx_0x01_login(uint8* data);
uint8* JWT_tx_0x04_card_charge_start(uint8 ch, uint8* data);
void   JWT_rx_0x06_app_start_process(uint8 *data);
uint8* JWT_tx_0x86_app_charge_start_response(uint8 ch, uint8 result, uint16 serial_NO, uint8* data);
uint8* JWT_tx_0x87_app_charge_stop_response(uint8 ch, uint8 result, uint16 serial_NO, uint8* data);
uint8* JWT_tx_setting_response(uint8 cmd, uint8 result, uint8* data);
void   JWT_rx_0x0B_set_billing_rule_process(uint8 *data);
uint8* JWT_tx_0x08_charge_record(uint8 ch, uint8*);
uint8* JWT_tx_0x8B_billing_rule_set_response(uint8* data);
uint8* JWT_tx_0x0B_billing_rule_query_response(uint8* data);
void   JWT_rx_0xA5_Remote_Update(uint8 *data);
uint8* JWT_tx_0x25_Remote_Update_response(uint8 *data,uint8 result);
uint8* JWT_tx_0x26_Remote_Update_result(uint8 *data ,uint8 result);
void   JWT_rx_0xA2_TCP_Update(uint8 *data);
uint8* JWT_tx_0x22_TCP_Update_response(uint8 *data, uint8 result);
int    JWT_rx_0xA3_TCP_Update_Data(uint8 *data);
uint8* JWT_tx_0x23_TCP_Update_Data_response(uint8 *data, uint8 result);
uint8* JWT_tx_0x24_TCP_Update_Data_result_response(uint8 *data, uint8 result);

uint8 JWT_rx_0xA5_HTTP_Update(uint8 *data);
uint8* JWT_tx_0xA6_HTTP_Update_result(uint8 *data ,uint8 result);

void set_test_num(uint8 NUM);
uint32 get_test_num(uint8 NUM);

/******************************************************************************
* Function Prototype Definitions
******************************************************************************/

#define testcode_llz 0
/*下面两个函数用来测试桩的*/

void set_test_num(uint8 NUM)
{
    #if testcode_llz
        if(NUM == 1)
            login_num++;
        else if(NUM == 2)
            reset_num++;
    #else
        return;
    #endif
}
uint32 get_test_num(uint8 NUM)
{
    #if testcode_llz
        if(NUM == 1)
            return login_num;
        else if(NUM == 2)
            return reset_num;
        return 0;

    #else

        return 0;

    #endif
}


void ceshilianwang(void)
{

    uint8 index = 13;
    uint8 ks[32] = "ftp://114.55.114.174";
    uint8 zh[16] = "sr";
    uint8 mm[16] = "sr123";
    uint16 port = 21;
    uint8 file[32] = "/1641435075595/AC_STATION.bin";

    memset(JWT_UpdateInfo.Update_url,0,64);
    memcpy(JWT_UpdateInfo.Update_url,ks,20);

    JWT_UpdateInfo.Update_port = port;
    memcpy(JWT_UpdateInfo.Update_Usr,zh,16);

    memcpy(JWT_UpdateInfo.Update_Pass,mm,16);

    memcpy(JWT_UpdateInfo.Update_File,file,32);


    JWT_UpdateInfo.UpdateMode = FTP_MODE;

    dbg_printf(1,"YUN_FTP.Update_Usr=%s ",JWT_UpdateInfo.Update_Usr);
    dbg_printf(1,"YUN_FTP.Update_Pass=%s ",JWT_UpdateInfo.Update_Pass);
    dbg_printf(1,"YUN_FTP.Update_File=%s ",JWT_UpdateInfo.Update_File);
}



uint8 JWT_rx_handler(uint8 *data)
{
	uint8  login = 0;
  uint16 i = 0;
  uint16 STATE = 0x00;
  uint16 CRC_value;

  uint8 rx_byte[rx_buff_length];

	co_dword dword;
  co_dword dword_dataLen;

  memcpy(&rx_byte[0], &data[0], rx_buff_length);
  do
  {
    if (rx_byte[i] == JWT_STX)
    {
      dword_dataLen.b[0] = rx_byte[i + 5];
      dword_dataLen.b[1] = rx_byte[i + 4];

			if ((i + 7 + dword_dataLen.w[0]) > (rx_buff_length - 1))
			{
				break;
			}

			if (rx_byte[i + 1] == STATE)
			{
				break;
			}
			dword.b[1] = rx_byte[i + 6 + dword_dataLen.w[0]];
			dword.b[0] = rx_byte[i + 7 + dword_dataLen.w[0]];
			CRC_value = dword.w[0];
      if (crc16_xmodem(&rx_byte[i + 1], &rx_byte[i + 6 + dword_dataLen.w[0]]) == CRC_value)
      {
        memcpy(&data[0], &rx_byte[i], 6 + dword_dataLen.w[0] + 1 + 1);

        STATE = JWT_daemon_rx_message_query(data,8 + dword_dataLen.l);
        switch (STATE)
        {
        case 0x07:
          {
          	if (JWT_link_way == JWT_link_eth)
            {

						}
						else
						{
            	data = JWT_tx_0x87_app_charge_stop_response(0, 0, 0, data);
            	Drv_Uart_Write(net_uart_port, data, 25, 0);
						}
					}
          break;

        case 0x0B:
          {
          	if (JWT_link_way == JWT_link_eth)
            {

						}
						else
						{
            	data = JWT_tx_0x0B_billing_rule_query_response(data);
            	Drv_Uart_Write(net_uart_port, data, 75, 0);
						}
					}
          break;

        case 0x0E:
          {
          	if (JWT_link_way == JWT_link_eth)
            {

						}
						else
						{
            	data = JWT_tx_setting_response(0x8E, 0x00, data);
            	Drv_Uart_Write(net_uart_port, data, 17, 0);
					  }
					}
          break;

				case 0x81:
					{
						login = 0x81;
					}
					break;

        case 0x8B:
          {
          	if (JWT_link_way == JWT_link_eth)
            {

						}
						else
						{
            	data = JWT_tx_0x0B_billing_rule_query_response(data);
            	Drv_Uart_Write(net_uart_port, data, 75, 0);
						}
					}
          break;

        default:
          break;
       }
      }
			else
			{
				dbg_printf(1, " background recv buf CRC error!");
			}
    }
    i += 6 + dword_dataLen.w[0] + 1 + 1;
  }while (i < (rx_buff_length - 150));

  return login;
}

void JWT_set_sys_time(uint8_t *RcvBuf)
{
    struct tm  SystemTimer;
    //uint8_t index =0;
    struct tm *SetingTime;


    {
        SystemTimer.tm_year =BcdToHex8(RcvBuf[0]) +100;
        SystemTimer.tm_mon = BcdToHex8(RcvBuf[1]) - 1;
        SystemTimer.tm_mday = BcdToHex8(RcvBuf[2]);


        SystemTimer.tm_hour = BcdToHex8(RcvBuf[3]);
        SystemTimer.tm_min = BcdToHex8(RcvBuf[4]);
        SystemTimer.tm_sec = BcdToHex8(RcvBuf[5]);

		time_t time = mktime(&SystemTimer);
		time -= 8*3600;
		SetingTime = localtime(&time);

        drv_set_sys_time(*SetingTime);
    }

}

/*------------------------------------------------------------
操作目的： 获取对应时间段的时间和时段值
作    者：
函数参数：
返回值：
            无
版   本:    V1.0
记   录:
------------------------------------------------------------*/
void JWT_first_fill_feenum()
{
  uint8 ret_feenum,i,k,num,value   = 0;
  memset(&rx_stage_fee,0,sizeof(rx_stage_fee));
  dbg_printf(1,"in JWT_first_fill_feenum",ret_feenum);

  ret_feenum = 0;
  for(i=0;i<48;i++)
  {
      rx_stage_fee.fee_time_value[i] = ret_feenum;
      if(g_Sys_Config_Param.FeeTime[i] != g_Sys_Config_Param.FeeTime[i+1] )
      {
        k = i+1;
        ret_feenum ++;
        if(ret_feenum == 1)
        {
          rx_stage_fee.fee_start_time[ret_feenum -1][0] = 0;
          rx_stage_fee.fee_start_time[ret_feenum -1][1] = 0;

          if(k/2 < 9)
          {
            rx_stage_fee.fee_end_time[ret_feenum -1][0] = k/2;
          }
          else
          {
            rx_stage_fee.fee_end_time[ret_feenum -1][0] = ((k/2)/10)*16 + ((k/2)%10);
          }

          if( k%2 == 1)
            rx_stage_fee.fee_end_time[ret_feenum -1][1] = 0x30;
          else
            rx_stage_fee.fee_end_time[ret_feenum -1][1] = 0;
        }
        else
        {
          rx_stage_fee.fee_start_time[ret_feenum -1][0] = rx_stage_fee.fee_end_time[ret_feenum-2][0];
          rx_stage_fee.fee_start_time[ret_feenum -1][1] = rx_stage_fee.fee_end_time[ret_feenum-2][1];

          if(k/2 < 9)
          {
            rx_stage_fee.fee_end_time[ret_feenum -1][0] = k/2;
          }
          else
          {
            rx_stage_fee.fee_end_time[ret_feenum -1][0] = ((k/2)/10)*16 + ((k/2)%10);
          }

          if( k%2 == 1)
            rx_stage_fee.fee_end_time[ret_feenum -1][1] = 0x30;
          else
            rx_stage_fee.fee_end_time[ret_feenum -1][1] = 0;
        }

        dbg_printf(1,"fee_start_time[%d]=%x %x \n fee_end_time[%d]=%x %x"
        ,ret_feenum-1,rx_stage_fee.fee_start_time[ret_feenum -1][0]
        ,rx_stage_fee.fee_start_time[ret_feenum -1][1]
        ,ret_feenum-1,rx_stage_fee.fee_end_time[ret_feenum -1][0]
        ,rx_stage_fee.fee_end_time[ret_feenum -1][1]);

      }

  }
  if(g_Sys_Config_Param.FeeTime[47] == g_Sys_Config_Param.FeeTime[46])
      rx_stage_fee.fee_time_value[47] = rx_stage_fee.fee_time_value[46];

  dbg_printf(1,"ret_feenum = %d",ret_feenum);
  for(i=0;i<48;i++)
    dbg_printf(1,"fee_time_value[%d] = %d",i,rx_stage_fee.fee_time_value[i]);

  rx_stage_fee.feenum = ret_feenum;
  return ;
}

/*------------------------------------------------------------
操作目的： 充电中返回实时时段费率明细
作    者：
函数参数：
返回值：
            无
版   本:    V1.0
记   录:
------------------------------------------------------------*/

void JWT_feenum_callback()
{
    uint8 ret_feenum,i,k,num,value   = 0;
    uint8 CurIndex     = 0;
    uint8 CurTimeHour  = 0;
    uint8 StaIndex     = 0;
    uint8 StaTimeHour  = 0;
    uint8 NextIndex     = 0;
    struct tm *curTime = {0};
    time_t time_val = time(NULL);
    struct tm *st_time;
    uint8 bcd_time[6] = {0};
    uint8 bcd_time_end[6] = {0};

    memset(&stage_fee,0,sizeof(stage_fee));

    curTime = gmtime(&time_val);
    bcd_time_end[0] = HexToBcd(curTime->tm_year - 100);
    bcd_time_end[1] = HexToBcd(curTime->tm_mon + 1);
    bcd_time_end[2] = HexToBcd(curTime->tm_mday);
    bcd_time_end[3] = HexToBcd(curTime->tm_hour);
    bcd_time_end[4] = HexToBcd(curTime->tm_min);
    bcd_time_end[5] = HexToBcd(curTime->tm_sec);
    CurTimeHour = curTime->tm_hour;
    CurIndex = CurTimeHour * 2;
    if (curTime->tm_min > 30)
    {
        CurIndex += 1;
    }
    if ((CurIndex < 0) || (CurIndex > 47))
    {
        CurIndex = 0;
    }


    dbg_printf(1,"StartTime=%d",gChargeRecord[0].StartTime);
    st_time = gmtime((const time_t *)&gChargeRecord[0].StartTime);
    bcd_time[0] = HexToBcd(st_time->tm_year - 100);
    bcd_time[1] = HexToBcd(st_time->tm_mon + 1);
    bcd_time[2] = HexToBcd(st_time->tm_mday);
    bcd_time[3] = HexToBcd(st_time->tm_hour);
    bcd_time[4] = HexToBcd(st_time->tm_min);
    bcd_time[5] = HexToBcd(st_time->tm_sec);
    StaTimeHour = st_time->tm_hour;
    if(st_time->tm_min> 30)
        StaIndex = StaTimeHour*2 +1;
    else
        StaIndex = StaTimeHour*2;

//    dbg_printf(1,"crgStartTime = %x %x %x %x %x %x"
//    ,g_RecordData.crgStartTime[0],g_RecordData.crgStartTime[1],g_RecordData.crgStartTime[2]
//    ,g_RecordData.crgStartTime[3],g_RecordData.crgStartTime[4],g_RecordData.crgStartTime[5]);
//    StaTimeHour = (g_RecordData.crgStartTime[3] >> 4)*10 + (g_RecordData.crgStartTime[3] & 0xf);
//    StaTimeMin  = (g_RecordData.crgStartTime[4] >> 4)*10 + (g_RecordData.crgStartTime[4] & 0xf);
//    if(StaTimeMin > 30)
//        StaIndex = StaTimeHour*2 +1;
//    else
//        StaIndex = StaTimeHour*2;



    num = CurIndex - StaIndex;
    ret_feenum = 0;
    for(i=0;i<num;i++)
    {
        if(g_Sys_Config_Param.FeeTime[StaIndex + i +1] != g_Sys_Config_Param.FeeTime[StaIndex + i])
        {
          ret_feenum ++;
        }
    }
    dbg_printf(1,"ret_feenum = %d",ret_feenum);
    dbg_printf(1,"CurIndex = %d StaIndex=%d num=%d",CurIndex,StaIndex,num);
    stage_fee.feenum = ret_feenum+1;

    if(ret_feenum < 1)
    {
        stage_fee.fee_stype[0] = g_Sys_Config_Param.FeeTime[CurIndex];
        //memcpy(stage_fee.fee_start_time[0],g_RecordData.crgStartTime,6);
        memcpy(stage_fee.fee_start_time[0],bcd_time,6);
//        stage_fee.fee_end_time[0][0] = HexToBcd(curTime->tm_year - 100);
//        stage_fee.fee_end_time[0][1] = HexToBcd(curTime->tm_mon + 1);
//        stage_fee.fee_end_time[0][2] = HexToBcd(curTime->tm_mday);
//        stage_fee.fee_end_time[0][3] = HexToBcd(curTime->tm_hour);
//        stage_fee.fee_end_time[0][4] = HexToBcd(curTime->tm_min);
//        stage_fee.fee_end_time[0][5] = HexToBcd(curTime->tm_sec);
        memcpy(stage_fee.fee_end_time[0],bcd_time_end,6);

        stage_fee.elefee[0] = g_Sys_Config_Param.Price[CurIndex];
        stage_fee.serfee[0] = g_Sys_Config_Param.CrgServFee;

        stage_fee.stage_energy[0] = gChargeRecord[0].EndEnerge - gChargeRecord[0].StartEnerge;
        stage_fee.stage_elefee[0] = (g_Sys_Config_Param.Price[CurIndex] * stage_fee.stage_energy[0])/100 ;
        stage_fee.stage_serfee[0] = (g_Sys_Config_Param.CrgServFee* stage_fee.stage_energy[0])/100;

        dbg_printf(1,"fee_stype %d \n fee_start_time %x %x %x %x %x %x \n fee_end_time %x %x %x %x %x %x \n"
          ,stage_fee.fee_stype[0]
          ,stage_fee.fee_start_time[0][0],stage_fee.fee_start_time[0][1],stage_fee.fee_start_time[0][2]
          ,stage_fee.fee_start_time[0][3],stage_fee.fee_start_time[0][4],stage_fee.fee_start_time[0][5]
          ,stage_fee.fee_end_time[0][0],stage_fee.fee_end_time[0][1],stage_fee.fee_end_time[0][2]
          ,stage_fee.fee_end_time[0][3],stage_fee.fee_end_time[0][4],stage_fee.fee_end_time[0][5]);
        dbg_printf(1,"elefee %d \n serfee %d \n stage_energy %d \n stage_elefee %d \n stage_serfee %d \n"
          ,stage_fee.elefee[0],stage_fee.serfee[0]
          ,stage_fee.stage_energy[0],stage_fee.stage_elefee[0],stage_fee.stage_serfee[0]);
    }
    else
    {
        for(i=0;i<ret_feenum+1;i++)
        {
            if(i == 0)
            {
                stage_fee.fee_stype[i] = g_Sys_Config_Param.FeeTime[StaIndex];
                value = rx_stage_fee.fee_time_value[StaIndex];
                dbg_printf(1,"value = %d",value);
                //memcpy(stage_fee.fee_start_time[i],g_RecordData.crgStartTime,6);
                memcpy(stage_fee.fee_start_time[i],bcd_time,6);
                stage_fee.fee_end_time[i][0] = HexToBcd(curTime->tm_year - 100);
                stage_fee.fee_end_time[i][1] = HexToBcd(curTime->tm_mon + 1);
                stage_fee.fee_end_time[i][2] = HexToBcd(curTime->tm_mday);
                stage_fee.fee_end_time[i][3] = rx_stage_fee.fee_end_time[value][0];
                stage_fee.fee_end_time[i][4] = rx_stage_fee.fee_end_time[value][1];
                //stage_fee.fee_end_time[i][5] = HexToBcd(curTime->tm_sec);
                stage_fee.fee_end_time[i][5] = 0;

                stage_fee.elefee[i] = g_Sys_Config_Param.Price[StaIndex];
                stage_fee.serfee[i] = g_Sys_Config_Param.CrgServFee;

                for(k=0;k<num;k++)
                {
                    if(g_Sys_Config_Param.FeeTime[StaIndex+k] == g_Sys_Config_Param.FeeTime[StaIndex+k+1])
                    {
                      stage_fee.stage_energy[i] += gChargeRecord[0].SectionEnerge[StaIndex+k];
                    }
                    else
                    {
                       //加上最后一个区间
                       stage_fee.stage_energy[i] += gChargeRecord[0].SectionEnerge[StaIndex+k];
                       NextIndex = StaIndex + k + 1;
                       break;
                    }
                }
                stage_fee.stage_elefee[i] = (stage_fee.elefee[i] * stage_fee.stage_energy[i])/100 ;
                stage_fee.stage_serfee[i] = (g_Sys_Config_Param.CrgServFee* stage_fee.stage_energy[i])/100;

                dbg_printf(1,"i=%d fee_stype %d \n fee_start_time %x %x %x %x %x %x \n fee_end_time %x %x %x %x %x %x \n"
          ,i,stage_fee.fee_stype[i]
          ,stage_fee.fee_start_time[i][0],stage_fee.fee_start_time[i][1],stage_fee.fee_start_time[i][2]
          ,stage_fee.fee_start_time[i][3],stage_fee.fee_start_time[i][4],stage_fee.fee_start_time[i][5]
          ,stage_fee.fee_end_time[i][0],stage_fee.fee_end_time[i][1],stage_fee.fee_end_time[i][2]
          ,stage_fee.fee_end_time[i][3],stage_fee.fee_end_time[i][4],stage_fee.fee_end_time[i][5]);
        dbg_printf(1,"elefee %d \n serfee %d \n stage_energy %d \n stage_elefee %d \n stage_serfee %d \n"
          ,stage_fee.elefee[i],stage_fee.serfee[i]
          ,stage_fee.stage_energy[i],stage_fee.stage_elefee[i],stage_fee.stage_serfee[i]);
            }
            else if(i == ret_feenum)
            {
                stage_fee.fee_stype[i] = g_Sys_Config_Param.FeeTime[NextIndex];
                value = rx_stage_fee.fee_time_value[NextIndex];
                dbg_printf(1,"value = %d",value);
                //memcpy(stage_fee.fee_end_time[i],g_RecordData.endStartTime,6);
                memcpy(stage_fee.fee_end_time[i],bcd_time_end,6);
                stage_fee.fee_start_time[i][0] = HexToBcd(curTime->tm_year - 100);
                stage_fee.fee_start_time[i][1] = HexToBcd(curTime->tm_mon + 1);
                stage_fee.fee_start_time[i][2] = HexToBcd(curTime->tm_mday);
                stage_fee.fee_start_time[i][3] = rx_stage_fee.fee_start_time[value][0];
                stage_fee.fee_start_time[i][4] = rx_stage_fee.fee_start_time[value][1];
                //stage_fee.fee_start_time[i][5] = HexToBcd(curTime->tm_sec);
                stage_fee.fee_start_time[i][5] = 0;

                stage_fee.elefee[i] = g_Sys_Config_Param.Price[NextIndex];
                stage_fee.serfee[i] = g_Sys_Config_Param.CrgServFee;

                for(k=0;k<num;k++)
                {
                    if(g_Sys_Config_Param.FeeTime[NextIndex+k] == g_Sys_Config_Param.FeeTime[NextIndex+k+1])
                    {
                      stage_fee.stage_energy[i] += gChargeRecord[0].SectionEnerge[NextIndex+k];
                    }
                    else
                    {
                       stage_fee.stage_energy[i] += gChargeRecord[0].SectionEnerge[NextIndex+k];
                       break;
                    }
                }
                stage_fee.stage_elefee[i] = (stage_fee.elefee[i]* stage_fee.stage_energy[i])/100 ;
                stage_fee.stage_serfee[i] = (g_Sys_Config_Param.CrgServFee* stage_fee.stage_energy[i])/100;

                dbg_printf(1,"i=%d fee_stype %d \n fee_start_time %x %x %x %x %x %x \n fee_end_time %x %x %x %x %x %x \n"
          ,i,stage_fee.fee_stype[i]
          ,stage_fee.fee_start_time[i][0],stage_fee.fee_start_time[i][1],stage_fee.fee_start_time[i][2]
          ,stage_fee.fee_start_time[i][3],stage_fee.fee_start_time[i][4],stage_fee.fee_start_time[i][5]
          ,stage_fee.fee_end_time[i][0],stage_fee.fee_end_time[i][1],stage_fee.fee_end_time[i][2]
          ,stage_fee.fee_end_time[i][3],stage_fee.fee_end_time[i][4],stage_fee.fee_end_time[i][5]);
        dbg_printf(1,"elefee %d \n serfee %d \n stage_energy %d \n stage_elefee %d \n stage_serfee %d \n"
          ,stage_fee.elefee[i],stage_fee.serfee[i]
          ,stage_fee.stage_energy[i],stage_fee.stage_elefee[i],stage_fee.stage_serfee[i]);
            }
            else
            {
                stage_fee.fee_stype[i] = g_Sys_Config_Param.FeeTime[NextIndex];
                value = rx_stage_fee.fee_time_value[NextIndex];
                dbg_printf(1,"value = %d",value);
                stage_fee.fee_start_time[i][0] = HexToBcd(curTime->tm_year - 100);
                stage_fee.fee_start_time[i][1] = HexToBcd(curTime->tm_mon + 1);
                stage_fee.fee_start_time[i][2] = HexToBcd(curTime->tm_mday);
                stage_fee.fee_start_time[i][3] = rx_stage_fee.fee_start_time[value][0];
                stage_fee.fee_start_time[i][4] = rx_stage_fee.fee_start_time[value][1];
                //stage_fee.fee_start_time[i][5] = HexToBcd(curTime->tm_sec);
                stage_fee.fee_start_time[i][5] = 0;
                stage_fee.fee_end_time[i][0] = HexToBcd(curTime->tm_year - 100);
                stage_fee.fee_end_time[i][1] = HexToBcd(curTime->tm_mon + 1);
                stage_fee.fee_end_time[i][2] = HexToBcd(curTime->tm_mday);
                stage_fee.fee_end_time[i][3] = rx_stage_fee.fee_end_time[value][0];
                stage_fee.fee_end_time[i][4] = rx_stage_fee.fee_end_time[value][1];
                //stage_fee.fee_end_time[i][5] = HexToBcd(curTime->tm_sec);
                stage_fee.fee_end_time[i][5] = 0;

                stage_fee.elefee[i] = g_Sys_Config_Param.Price[NextIndex];
                stage_fee.serfee[i] = g_Sys_Config_Param.CrgServFee;

                for(k=0;k<num;k++)
                {
                    if(g_Sys_Config_Param.FeeTime[NextIndex+k] == g_Sys_Config_Param.FeeTime[NextIndex+k+1])
                    {
                      stage_fee.stage_energy[i] += gChargeRecord[0].SectionEnerge[NextIndex+k];
                    }
                    else
                    {
                       stage_fee.stage_energy[i] += gChargeRecord[0].SectionEnerge[NextIndex+k];
                       NextIndex = NextIndex + k + 1;
                       break;
                    }
                }
                stage_fee.stage_elefee[i] = (stage_fee.elefee[i] * stage_fee.stage_energy[i])/100 ;
                stage_fee.stage_serfee[i] = (g_Sys_Config_Param.CrgServFee* stage_fee.stage_energy[i])/100;

                dbg_printf(1,"i=%d fee_stype %d \n fee_start_time %x %x %x %x %x %x \n fee_end_time %x %x %x %x %x %x \n"
          ,i,stage_fee.fee_stype[i]
          ,stage_fee.fee_start_time[i][0],stage_fee.fee_start_time[i][1],stage_fee.fee_start_time[i][2]
          ,stage_fee.fee_start_time[i][3],stage_fee.fee_start_time[i][4],stage_fee.fee_start_time[i][5]
          ,stage_fee.fee_end_time[i][0],stage_fee.fee_end_time[i][1],stage_fee.fee_end_time[i][2]
          ,stage_fee.fee_end_time[i][3],stage_fee.fee_end_time[i][4],stage_fee.fee_end_time[i][5]);
        dbg_printf(1,"elefee %d \n serfee %d \n stage_energy %d \n stage_elefee %d \n stage_serfee %d \n"
          ,stage_fee.elefee[i],stage_fee.serfee[i]
          ,stage_fee.stage_energy[i],stage_fee.stage_elefee[i],stage_fee.stage_serfee[i]);
            }
        }
    }



    return ;
}

/*------------------------------------------------------------
操作目的： 充电订单返回实时时段费率明细
作    者：
函数参数：
  reportRecord：订单数据
返回值：
            无
版   本:    V1.0
记   录:
------------------------------------------------------------*/

void JWT_record_feenum_callback(ChargeRecord reportRecord)
{
    uint8 ret_feenum,i,k,num,value   = 0;
    uint8 EndIndex     = 0;
    uint8 EndTimeHour  = 0;
    uint8 EndTimeMin   = 0;
    uint8 StaIndex     = 0;
    uint8 StaTimeHour  = 0;
    uint8 StaTimeMin   = 0;
    uint8 NextIndex     = 0;
    struct tm *curTime = {0};
    time_t time_val = time(NULL);
    memset(&stage_fee,0,sizeof(stage_fee));

    struct tm *st_time;
    uint8 bcd_time[6] = {0};
    uint8 bcd_time_end[6] = {0};

    dbg_printf(1,"StartTime=%d EndTime=%d",reportRecord.StartTime,reportRecord.EndTime);
    st_time = gmtime((const time_t *)&reportRecord.StartTime);
    bcd_time[0] = HexToBcd(st_time->tm_year - 100);
    bcd_time[1] = HexToBcd(st_time->tm_mon + 1);
    bcd_time[2] = HexToBcd(st_time->tm_mday);
    bcd_time[3] = HexToBcd(st_time->tm_hour);
    bcd_time[4] = HexToBcd(st_time->tm_min);
    bcd_time[5] = HexToBcd(st_time->tm_sec);

    st_time = gmtime((const time_t *)&reportRecord.EndTime);
    bcd_time_end[0] = HexToBcd(st_time->tm_year - 100);
    bcd_time_end[1] = HexToBcd(st_time->tm_mon + 1);
    bcd_time_end[2] = HexToBcd(st_time->tm_mday);
    bcd_time_end[3] = HexToBcd(st_time->tm_hour);
    bcd_time_end[4] = HexToBcd(st_time->tm_min);
    bcd_time_end[5] = HexToBcd(st_time->tm_sec);

    dbg_printf(1,"bcd_time = %x %x %x %x %x %x",bcd_time[0],bcd_time[1],bcd_time[2]
      ,bcd_time[3],bcd_time[4],bcd_time[5]);
    dbg_printf(1,"bcd_time_end = %x %x %x %x %x %x",bcd_time_end[0],bcd_time_end[1],bcd_time_end[2]
      ,bcd_time_end[3],bcd_time_end[4],bcd_time_end[5]);

    StaTimeHour = (bcd_time[3] >> 4)*10 + (bcd_time[3] & 0xf);
    StaTimeMin  = (bcd_time[4] >> 4)*10 + (bcd_time[4] & 0xf);
    if(StaTimeMin > 30)
        StaIndex = StaTimeHour*2 +1;
    else
        StaIndex = StaTimeHour*2;
    dbg_printf(1,"record StaTimeHour = %d StaTimeMin=%d",StaTimeHour,StaTimeMin);
    EndTimeHour = (bcd_time_end[3] >> 4)*10 + (bcd_time_end[3] & 0xf);
    EndTimeMin  = (bcd_time_end[4] >> 4)*10 + (bcd_time_end[4] & 0xf);
    if(EndTimeMin > 30)
        EndIndex = EndTimeHour*2 +1;
    else
        EndIndex = EndTimeHour*2;
    dbg_printf(1,"record  EndTimeHour = %d EndTimeMin=%d",EndTimeHour,EndTimeMin);
    num = EndIndex - StaIndex;
    dbg_printf(1,"EndIndex = %d StaIndex=%d num=%d",EndIndex,StaIndex,num);

    //num = 1;
    ret_feenum = 0;
#if 1
    for(i=0;i<num;i++)
    {
        if(g_Sys_Config_Param.FeeTime[StaIndex + i +1] != g_Sys_Config_Param.FeeTime[StaIndex + i])
        {
          ret_feenum ++;
        }
    }
    dbg_printf(1,"ret_feenum = %d",ret_feenum);

    stage_fee.feenum = ret_feenum +1;

    if(ret_feenum < 1)
    {
        stage_fee.fee_stype[0] = g_Sys_Config_Param.FeeTime[StaIndex];
        memcpy(stage_fee.fee_start_time[0],bcd_time,6);
        memcpy(stage_fee.fee_end_time[0],bcd_time_end,6);

        stage_fee.elefee[0] = g_Sys_Config_Param.Price[StaIndex];
        stage_fee.serfee[0] = g_Sys_Config_Param.CrgServFee;

        stage_fee.stage_energy[0] = reportRecord.EndEnerge - reportRecord.StartEnerge;
        stage_fee.stage_elefee[0] = (g_Sys_Config_Param.Price[StaIndex] * stage_fee.stage_energy[0])/100 ;
        stage_fee.stage_serfee[0] = (g_Sys_Config_Param.CrgServFee* stage_fee.stage_energy[0])/100;
        dbg_printf(1,"fee_stype %d \n fee_start_time %x %x %x %x %x %x \n fee_end_time %x %x %x %x %x %x \n"
          ,stage_fee.fee_stype[0]
          ,stage_fee.fee_start_time[0][0],stage_fee.fee_start_time[0][1],stage_fee.fee_start_time[0][2]
          ,stage_fee.fee_start_time[0][3],stage_fee.fee_start_time[0][4],stage_fee.fee_start_time[0][5]
          ,stage_fee.fee_end_time[0][0],stage_fee.fee_end_time[0][1],stage_fee.fee_end_time[0][2]
          ,stage_fee.fee_end_time[0][3],stage_fee.fee_end_time[0][4],stage_fee.fee_end_time[0][5]);
        dbg_printf(1,"elefee %d \n serfee %d \n stage_energy %d \n stage_elefee %d \n stage_serfee %d \n"
          ,stage_fee.elefee[0],stage_fee.serfee[0]
          ,stage_fee.stage_energy[0],stage_fee.stage_elefee[0],stage_fee.stage_serfee[0]);
    }
    else
    {
        for(i=0;i<ret_feenum+1;i++)
        {
            if(i == 0)
            {
                stage_fee.fee_stype[i] = g_Sys_Config_Param.FeeTime[StaIndex];
                memcpy(stage_fee.fee_start_time[i],bcd_time,6);
                memcpy(stage_fee.fee_end_time[i],bcd_time_end,6);
                value = rx_stage_fee.fee_time_value[StaIndex];
                dbg_printf(1,"value = %d",value);
                stage_fee.fee_end_time[i][3] = rx_stage_fee.fee_end_time[value][0];
                stage_fee.fee_end_time[i][4] = rx_stage_fee.fee_end_time[value][1];
                stage_fee.fee_end_time[i][5] = 0;

                stage_fee.elefee[i] = g_Sys_Config_Param.Price[StaIndex];
                stage_fee.serfee[i] = g_Sys_Config_Param.CrgServFee;

                for(k=0;k<num;k++)
                {
                    if(g_Sys_Config_Param.FeeTime[StaIndex+k] == g_Sys_Config_Param.FeeTime[StaIndex+k+1])
                    {
                      stage_fee.stage_energy[i] += reportRecord.SectionEnerge[StaIndex+k];
                    }
                    else
                    {
                       //加上最后一个区间
                       stage_fee.stage_energy[i] += reportRecord.SectionEnerge[StaIndex+k];
                       NextIndex = StaIndex + k + 1;
                       break;
                    }
                }
                stage_fee.stage_elefee[i] = (stage_fee.elefee[i] * stage_fee.stage_energy[i])/100 ;
                stage_fee.stage_serfee[i] = (g_Sys_Config_Param.CrgServFee* stage_fee.stage_energy[i])/100;

               dbg_printf(1,"i=%d fee_stype %d \n fee_start_time %x %x %x %x %x %x \n fee_end_time %x %x %x %x %x %x \n"
          ,i,stage_fee.fee_stype[i]
          ,stage_fee.fee_start_time[i][0],stage_fee.fee_start_time[i][1],stage_fee.fee_start_time[i][2]
          ,stage_fee.fee_start_time[i][3],stage_fee.fee_start_time[i][4],stage_fee.fee_start_time[i][5]
          ,stage_fee.fee_end_time[i][0],stage_fee.fee_end_time[i][1],stage_fee.fee_end_time[i][2]
          ,stage_fee.fee_end_time[i][3],stage_fee.fee_end_time[i][4],stage_fee.fee_end_time[i][5]);
        dbg_printf(1,"elefee %d \n serfee %d \n stage_energy %d \n stage_elefee %d \n stage_serfee %d \n"
          ,stage_fee.elefee[i],stage_fee.serfee[i]
          ,stage_fee.stage_energy[i],stage_fee.stage_elefee[i],stage_fee.stage_serfee[i]);
            }
            else if(i == ret_feenum)
            {
                stage_fee.fee_stype[i] = g_Sys_Config_Param.FeeTime[NextIndex];
                memcpy(stage_fee.fee_end_time[i],bcd_time_end,6);
                memcpy(stage_fee.fee_start_time[i],bcd_time,6);
                value = rx_stage_fee.fee_time_value[NextIndex];
                dbg_printf(1,"value = %d",value);
                stage_fee.fee_start_time[i][3] = rx_stage_fee.fee_start_time[value][0];
                stage_fee.fee_start_time[i][4] = rx_stage_fee.fee_start_time[value][1];
                stage_fee.fee_start_time[i][5] = 0;

                stage_fee.elefee[i] = g_Sys_Config_Param.Price[NextIndex];
                stage_fee.serfee[i] = g_Sys_Config_Param.CrgServFee;

                for(k=0;k<num;k++)
                {
                    if(g_Sys_Config_Param.FeeTime[NextIndex+k] == g_Sys_Config_Param.FeeTime[NextIndex+k+1])
                    {
                      stage_fee.stage_energy[i] += reportRecord.SectionEnerge[NextIndex+k];
                    }
                    else
                    {
                       stage_fee.stage_energy[i] += reportRecord.SectionEnerge[NextIndex+k];
                       break;
                    }
                }
                stage_fee.stage_elefee[i] = (stage_fee.elefee[i]* stage_fee.stage_energy[i])/100 ;
                stage_fee.stage_serfee[i] = (g_Sys_Config_Param.CrgServFee* stage_fee.stage_energy[i])/100;

                dbg_printf(1,"i=%d fee_stype %d \n fee_start_time %x %x %x %x %x %x \n fee_end_time %x %x %x %x %x %x \n"
          ,i,stage_fee.fee_stype[i]
          ,stage_fee.fee_start_time[i][0],stage_fee.fee_start_time[i][1],stage_fee.fee_start_time[i][2]
          ,stage_fee.fee_start_time[i][3],stage_fee.fee_start_time[i][4],stage_fee.fee_start_time[i][5]
          ,stage_fee.fee_end_time[i][0],stage_fee.fee_end_time[i][1],stage_fee.fee_end_time[i][2]
          ,stage_fee.fee_end_time[i][3],stage_fee.fee_end_time[i][4],stage_fee.fee_end_time[i][5]);
        dbg_printf(1,"elefee %d \n serfee %d \n stage_energy %d \n stage_elefee %d \n stage_serfee %d \n"
          ,stage_fee.elefee[i],stage_fee.serfee[i]
          ,stage_fee.stage_energy[i],stage_fee.stage_elefee[i],stage_fee.stage_serfee[i]);
            }
            else
            {
                stage_fee.fee_stype[i] = g_Sys_Config_Param.FeeTime[NextIndex];
                memcpy(stage_fee.fee_start_time[i],bcd_time,6);
                memcpy(stage_fee.fee_end_time[i],bcd_time_end,6);
                value = rx_stage_fee.fee_time_value[NextIndex];
                dbg_printf(1,"value = %d",value);
                stage_fee.fee_start_time[i][3] = rx_stage_fee.fee_start_time[value][0];
                stage_fee.fee_start_time[i][4] = rx_stage_fee.fee_start_time[value][1];
                stage_fee.fee_start_time[i][5] = 0;
                stage_fee.fee_end_time[i][3] = rx_stage_fee.fee_end_time[value][0];
                stage_fee.fee_end_time[i][4] = rx_stage_fee.fee_end_time[value][1];
                stage_fee.fee_end_time[i][5] = 0;

                stage_fee.elefee[i] = g_Sys_Config_Param.Price[NextIndex];
                stage_fee.serfee[i] = g_Sys_Config_Param.CrgServFee;

                for(k=0;k<num;k++)
                {
                    if(g_Sys_Config_Param.FeeTime[NextIndex+k] == g_Sys_Config_Param.FeeTime[NextIndex+k+1])
                    {
                      stage_fee.stage_energy[i] += reportRecord.SectionEnerge[NextIndex+k];
                    }
                    else
                    {
                       stage_fee.stage_energy[i] += reportRecord.SectionEnerge[NextIndex+k];
                       NextIndex = NextIndex + k + 1;
                       break;
                    }
                }
                stage_fee.stage_elefee[i] = (stage_fee.elefee[i] * stage_fee.stage_energy[i])/100 ;
                stage_fee.stage_serfee[i] = (g_Sys_Config_Param.CrgServFee* stage_fee.stage_energy[i])/100;

                dbg_printf(1,"i=%d fee_stype %d \n fee_start_time %x %x %x %x %x %x \n fee_end_time %x %x %x %x %x %x \n"
          ,i,stage_fee.fee_stype[i]
          ,stage_fee.fee_start_time[i][0],stage_fee.fee_start_time[i][1],stage_fee.fee_start_time[i][2]
          ,stage_fee.fee_start_time[i][3],stage_fee.fee_start_time[i][4],stage_fee.fee_start_time[i][5]
          ,stage_fee.fee_end_time[i][0],stage_fee.fee_end_time[i][1],stage_fee.fee_end_time[i][2]
          ,stage_fee.fee_end_time[i][3],stage_fee.fee_end_time[i][4],stage_fee.fee_end_time[i][5]);
        dbg_printf(1,"elefee %d \n serfee %d \n stage_energy %d \n stage_elefee %d \n stage_serfee %d \n"
          ,stage_fee.elefee[i],stage_fee.serfee[i]
          ,stage_fee.stage_energy[i],stage_fee.stage_elefee[i],stage_fee.stage_serfee[i]);
            }
        }
    }

#endif

    return ;
}



uint16 JWT_daemon_rx_message_query(uint8 *data,int RcvLen)
{
  uint8  *buff = data;
  int ret = 0;
  int cnt = 0;
  uint16 rx_cmd = 0x00;
  //memcpy(buff, data, RcvLen);

  switch (buff[1])
  {
    case CustomCloud_CMD_LOGIN_RET:
      {
        if (buff[14] == 0u)
        {
          rx_cmd = CustomCloud_CMD_LOGIN_RET;
          dbg_printf(1, " log in succefully");
        }
      }
      break;

    case CustomCloud_CMD_CARD_START_CRG_RET:
      {
        if (buff[18] == 0)
        {
          dbg_printf(1, " card_start_charging_result:successfully");
        }
        else if (buff[18] == 1)
        {
          dbg_printf(1," card_start_charging_result:card is invalid");
        }
        else if (buff[18] == 2)
        {
          dbg_printf(1," card_start_charging_result:there is an unsettled order");
        }
        else if (buff[18] == 3)
        {
          dbg_printf(1," card_start_charging_result:system error");
        }
        g_JWT_info_t.card_start_rx_flag = 1;
      }
      break;

    case CustomCloud_CMD_CARD_STOP_CRG_RET:
      {
        if (buff[14])
        {
          dbg_printf(1," card_stop_charging_result:failure");
        }
        else
        {
          dbg_printf(1," card_stop_charging_result:successful");
        }
        g_JWT_info_t.card_stop_rx_flag = 1;
      }
      break;

    case CustomCloud_CMD_CTRL_START_CRG:
      {
        JWT_rx_0x06_app_start_process(buff);
        rx_cmd = CustomCloud_CMD_CTRL_START_CRG;
      }
      break;

    case CustomCloud_CMD_CTRL_STOP_CRG:
      {
        uint8 Ch;

        Ch = (buff[6] == 2u) ? 1u : 0;
        dbg_printf(1," gun_NO:%d, APP resquest stopping", Ch);

        Ctrl_power_off(STOP_REASON_CAR_STOP, Ch);
        rx_cmd = CustomCloud_CMD_CTRL_STOP_CRG;
      }
      break;

    case CustomCloud_CMD_CRGRECORD_REPORT_RET:
      {
        dbg_printf(1," rx_cmd_0x88 charger record ack response");
        g_RecordData.payMoney = 6u;
        Updata_UnReport_Data(TYPE_CHARGELOG,(uint8_t *)&st_Record,0);
      }
      break;

    case CustomCloud_CMD_SET_FEE_0B:
      {
				dbg_printf(1," rx_cmd_0x0B set_billing_rule_process");
				//print_hex_data(buff, 100);
        JWT_rx_0x0B_set_billing_rule_process(buff);
				rx_cmd = CustomCloud_CMD_SET_FEE_0B;
      }
      break;

    case CustomCloud_CMD_HEART_RET:
      {
        dbg_printf(1," rx_cmd_0x8C heart_beat response");
        rx_cmd = CustomCloud_CMD_HEART_RET;
        memcpy(&g_JWT_info_t.heart_beat_time[0], &buff[6], 6);
        JWT_set_sys_time(&buff[6]);
        comm.heartTick = time(NULL);
        if(abs(time(NULL) - gChargeRecord[0].StartTime) > 48 * 3600)
        {
            gChargeRecord[0].StartTime = time(NULL);
        }




      }
      break;

    case CustomCloud_CMD_SEND_FAULT_RET:
      {
        dbg_printf(1," rx_cmd_8D warning_response");
        rx_cmd = CustomCloud_CMD_SEND_FAULT_RET;
      }
      break;

		case CustomCloud_CMD_SET_FEE_RET_8B:
			{
				dbg_printf(1, " rx_cmd_8B query billing rule ");
        //comm.SendBuf = JWT_tx_0x0B_billing_rule_query_response(comm.SendBuf );
			  //print_hex_data(comm.SendBuf,comm.SendLen);

			}
			break;

    case CustomCloud_CMD_SET_TIME:
      {

        rx_cmd = CustomCloud_CMD_SET_TIME;
        JWT_set_sys_time(&buff[6]);
        comm.heartTick = time(NULL);
        if(abs(time(NULL) - gChargeRecord[0].StartTime) > 48 * 3600)
        {
            gChargeRecord[0].StartTime = time(NULL);
        }
      }
      break;

    case CustomCloud_CMD_SET_QRCode_RET:
      {
      	//print_hex_data(buff,100);
      	uint8 i=0;
        uint16 QR_length;
				i += 7;
		    QR_length = (buff[7] << 8) + buff[8];
				i+=2;
				if(QR_length != 0)
				{
					memset(g_Sys_Config_Param.QRCode,0,sizeof(g_Sys_Config_Param.QRCode));
	        memcpy(&g_Sys_Config_Param.QRCode,buff+i,QR_length);
					osDelay(500);
					API_Save_Sys_Param_Direct();
				}

				dbg_printf(1," QR=%s ",g_Sys_Config_Param.QRCode);
				dbg_printf(1," len=%d",QR_length);

				rx_cmd = CustomCloud_CMD_SET_QRCode_RET;
      }
      break;
	case CustomCloud_CMD_UPDATA_SOFT_FTP:
      {
		//print_hex_data(buff,155);

        rx_cmd = 0x25;
        dbg_printf(1," rx_cmd_0xA5 Remote Update ");
		if(buff[6] == 0)//整机程序
		{
			//rx_0xA5
			JWT_rx_0xA5_Remote_Update(buff);
			osDelay(500);
			dbg_printf(1,"send tx_cmd_0x25 Remote_Update_response");
			comm.SendBuf = JWT_tx_0x25_Remote_Update_response(buff,0);
			comm.Send(comm.Port,comm.SendBuf,20,0);
			osDelay(1500);

			/**************************** 程序升级 ******************************/
			char Update_File_buff[32];
			sprintf(Update_File_buff,"./%s",JWT_UpdateInfo.Update_File);
			memcpy(JWT_UpdateInfo.Update_File,Update_File_buff,sizeof(Update_File_buff));
			dbg_printf(1,"start updata file %s",JWT_UpdateInfo.Update_File);
			//JWT_UpdateInfo.UpdateMode = FTP_MODE;

			//升级前，先擦除flash
			Erase_spi_flash_app_data();

			ret=Ftp_Updata(&JWT_UpdateInfo);

			if(ret ==  1)
            {
                dbg_printf(1,"update ok\r");
                JWT_UpdateInfo.UpdateMode = FTP_IAP_MODE;
                BSP_SPIFLASH_SectorErase(UPDATA_INFO_ADDR);
                BSP_SPIFLASH_BufferWrite((uint8_t*)&JWT_UpdateInfo,UPDATA_INFO_ADDR,sizeof(JWT_UpdateInfo));
				osDelay(1000);


				comm.SendBuf = JWT_tx_0x26_Remote_Update_result(buff,1);
				comm.Send(comm.Port,comm.SendBuf,20,0);
				dbg_printf(1," tx_cmd_0x26 Remote Update result 1 ");
				osDelay(1000);

				//重启
				dbg_printf(1,"reboot .....\n");
				osDelay(1000);
				set_test_num(2);
				NVIC_SystemReset();

            }
            else
            {
                JWT_UpdateInfo.UpdateMode = UPDATEMODE_NONE;//不要再继续升级了

                dbg_printf(1,"update err");


				if(ret == 2)
				{
					comm.SendBuf = JWT_tx_0x26_Remote_Update_result(buff,2);
					comm.Send(comm.Port,comm.SendBuf,20,0);
					dbg_printf(1," tx_cmd_0x26 Remote Update result download fail ");
					osDelay(1000);
				}
				else if(ret == 0)
				{
					comm.SendBuf = JWT_tx_0x26_Remote_Update_result(buff,0);
					comm.Send(comm.Port,comm.SendBuf,20,0);
					dbg_printf(1," tx_cmd_0x26 Remote Update result update fail ");
					osDelay(1000);
				}


                cnt++;

                if(cnt >= 3)
                {
                    set_test_num(2);

                    NVIC_SystemReset();

                }
            }
		}
		else
		{
			JWT_tx_0x25_Remote_Update_response(buff,1);
		}
	}
      break;

	case 0xA6:
      {
        if(buff[6] == 1)
        {
			dbg_printf(1, " rx_cmd_A6 Remote Update result response success ");
		}
		else if(buff[6] == 2)
        {
			dbg_printf(1, " rx_cmd_A6 Remote Update result response fail");
		}
		else
		{
			dbg_printf(1, " rx_cmd_A6 Remote Update result response %d ",buff[6]);
		}
      }
      break;
	case 0xA2:
      {
        //print_hex_data(buff,53);

        rx_cmd = 0xA2;
        dbg_printf(1," rx_cmd_0xA2 Remote Update ");
		//收到升级指令重置参数
		g_Sys_Config_Param.TCP_ud_flag = 0;
		g_Sys_Config_Param.TCP_ud_stop_adr = 0;
		g_Sys_Config_Param.TCP_ud_stop_serial_number = 0;


		//保存参数
		//API_Save_Sys_Param_Direct();

		if(buff[6] == 0)//整机程序
		{
			JWT_rx_0xA2_TCP_Update(buff);

			if(g_Charge_Ctrl_Manager[0].CurState == STATE_IDLE)
			{
				comm.SendBuf = JWT_tx_0x22_TCP_Update_response(buff,0);
				comm.Send(comm.Port,comm.SendBuf,23,0);
				dbg_printf(1," tx_cmd_0x22 TCP Update response 0 ");

				//升级前，先擦除flash
				Erase_spi_flash_app_data();

				g_Sys_Config_Param.TCP_ud_flag = 1;//开始升级
			}
			else
			{
				comm.SendBuf = JWT_tx_0x22_TCP_Update_response(buff,1);
				comm.Send(comm.Port,comm.SendBuf,23,0);
				dbg_printf(1," tx_cmd_0x22 Update fail  Charging  1 ");

			}
		}
		else
		{
				comm.SendBuf = JWT_tx_0x22_TCP_Update_response(buff,2);
				comm.Send(comm.Port,comm.SendBuf,23,0);
				dbg_printf(1," tx_cmd_0x22 Update fail  file err  2 ");

		}
      }
      break;

	case 0xA3:
      {
      //超时便不会接收数据包
      if(g_Sys_Config_Param.TCP_ud_flag != 1)
	  	break;

       //print_hex_data(buff,274);
	   //osDelay(100);
       ret = JWT_rx_0xA3_TCP_Update_Data(buff);

	   if(ret == 1 )
	   	{
		    //dbg_printf(1,"write adr : %d ",(TCP_Info.TCP_File_bags_serial_number - 1) * LEN_READ);

			//写入数据到flash
			BSP_SPIFLASH_BufferWrite(TCP_Info.TCP_File_data,(TCP_Info.TCP_File_bags_serial_number - 1) * LEN_READ,LEN_READ);
			//print_hex_data(TCP_Info.TCP_File_data,256);
			//保存地址、序列号
			g_Sys_Config_Param.TCP_ud_stop_adr = TCP_Info.TCP_File_bags_serial_number * LEN_READ;
			g_Sys_Config_Param.TCP_ud_stop_serial_number = TCP_Info.TCP_File_bags_serial_number;

			//保存参数
			API_Save_Sys_Param_Direct();
			//dbg_printf(1,"g_Sys_serial_number : %d ",g_Sys_Config_Param.TCP_ud_stop_serial_number);
			//dbg_printf(1,"g_Sys_adr : %d ",g_Sys_Config_Param.TCP_ud_stop_adr);
			osDelay(100);

			//send 0x23
			comm.SendBuf = JWT_tx_0x23_TCP_Update_Data_response(buff,0);
			comm.Send(comm.Port,comm.SendBuf,19,0);
			dbg_printf(1," tx_cmd_0x23");

			//显示进度
	        dbg_printf(1,"ftp complete %d\n ",(g_Sys_Config_Param.TCP_ud_stop_adr* 100 / APP_SIZE));

			if(g_Sys_Config_Param.TCP_ud_stop_serial_number == TCP_Update_File_bags_number)
			{
				dbg_printf(1,"g_serial_num : %d ",g_Sys_Config_Param.TCP_ud_stop_serial_number);

				ret = check_file();
				if( ret == 1)//&& g_Sys_Config_Param.TCP_ud_stop_adr == APP_SIZE
				{
					dbg_printf(1,"update ok\r");
	                JWT_UpdateInfo.UpdateMode = FTP_IAP_MODE;
	                BSP_SPIFLASH_SectorErase(UPDATA_INFO_ADDR);
	                BSP_SPIFLASH_BufferWrite((uint8_t*)&JWT_UpdateInfo,UPDATA_INFO_ADDR,sizeof(JWT_UpdateInfo));
	                dbg_printf(1,"please reboot system\n");
					osDelay(5000);


				    //send 0x24
				    comm.SendBuf = JWT_tx_0x24_TCP_Update_Data_result_response(buff,1);
				    comm.Send(comm.Port,comm.SendBuf,20,0);
				    dbg_printf(1," tx_cmd_0x24 TCP Update result  success !");
				    //osDelay(1000);
				    g_Sys_Config_Param.TCP_ud_flag = 2;

				}
				else
				{
					dbg_printf(1,"check_file fail ");
					//send 0x24
				    comm.SendBuf = JWT_tx_0x24_TCP_Update_Data_result_response(buff,0);
				    comm.Send(comm.Port,comm.SendBuf,20,0);
				    dbg_printf(1," tx_cmd_0x24 TCP Update result fail !");
				    osDelay(1000);
				}

			}

		}
	   else if(ret == 2)
	   	{
			comm.SendBuf = JWT_tx_0x23_TCP_Update_Data_response(buff,2);
			comm.Send(comm.Port,comm.SendBuf,19,0);
			dbg_printf(1," tx_cmd_023   serial_num err   2 ");
			osDelay(1000);
	    }
	   else
	   	{
			comm.SendBuf = JWT_tx_0x23_TCP_Update_Data_response(buff,1);
			comm.Send(comm.Port,comm.SendBuf,19,0);
			dbg_printf(1," tx_cmd_023   bags_lenth err  0   upd_flag:%d ",g_Sys_Config_Param.TCP_ud_flag);
			osDelay(1000);
	    }
      }
      break;

	case 0xA4:
      {
      	print_hex_data(buff,15);

        //重置
        //g_Sys_Config_Param.TCP_ud_flag = 0;
		//g_Sys_Config_Param.TCP_ud_stop_adr = 0;
		//g_Sys_Config_Param.TCP_ud_stop_serial_number = 0;
		//保存参数
		//API_Save_Sys_Param_Direct();
      }
      break;

  case 0x12:
    {
      ret = JWT_rx_0xA5_HTTP_Update(buff);
      JWT_tx_0xA6_HTTP_Update_result(buff,ret);
      if(ret == 2)
      {
        start_http_download(comm.connectType);
        http_download_process();
      }
    }
    break;

    default:
      break;
  }
	return rx_cmd;
}

uint8* JWT_membuffer_alloc(uint16 serialNumber, uint16 cmd, uint8 *data, uint16 length)
{
  static uint8  SendBuf[400];
  uint16 index;
  uint16 i;
  uint16 CRC_value;

	index = 0;
  SendBuf[index++] = JWT_STX;
  SendBuf[index++] = cmd;
  SendBuf[index++] = serialNumber >> 8;
  SendBuf[index++] = serialNumber;
  SendBuf[index++] = length >> 8;
  SendBuf[index++] = length;

  if ((NULL != data) && (length != 0))
  {
    for (i=0; i < length; i++)
    {
      SendBuf[index++] = *data++;
    }
  }

  CRC_value = crc16_xmodem(&SendBuf[1], &SendBuf[index]);
  SendBuf[index++] = CRC_value >> 8;
  SendBuf[index++] = CRC_value;

  return SendBuf;
}

uint8* JWT_tx_0x01_login(uint8* data)
{
  uint8 i;
  uint8 DataBuf[120];
  set_test_num(1);

  DataBuf[0] = ((g_Sys_Config_Param.Serial[0] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[1] & 0x0F);
  DataBuf[1] = ((g_Sys_Config_Param.Serial[2] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[3] & 0x0F);
  DataBuf[2] = ((g_Sys_Config_Param.Serial[4] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[5] & 0x0F);
  DataBuf[3] = ((g_Sys_Config_Param.Serial[6] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[7] & 0x0F);
  DataBuf[4] = ((g_Sys_Config_Param.Serial[8] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[9] & 0x0F);
  DataBuf[5] = ((g_Sys_Config_Param.Serial[10] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[11] & 0x0F);
  DataBuf[6] = ((g_Sys_Config_Param.Serial[12] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[13] & 0x0F);
  DataBuf[7] = ((g_Sys_Config_Param.Serial[14] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[15] & 0x0F);

	for (i = 0; i < 8; i++)
  {
    g_DeviceInfo.serNo[i] = DataBuf[i];
  }

  DataBuf[8] = 0x01;//g_DeviceInfo.deviceType;
  DataBuf[9] = 0x01;

  DataBuf[10] = 0x00;
  DataBuf[11] = 0x00;
  DataBuf[12] = 0x00;
  //DataBuf[13] = 0x00;
  DataBuf[13] = g_Sys_Config_Param.restart_num;

  DataBuf[14] = 0x00;
  DataBuf[15] = 0x00;
  DataBuf[16] = 0x00;

  //DataBuf[17] = SoftVer[1]*10 + SoftVer[2];
  //DataBuf[18] = SoftVer[4]*10 + SoftVer[5];

  DataBuf[17] = 2;//SoftVer[1]- '0';
  DataBuf[18] = ((SoftVer[3] -'0') *10)+ (SoftVer[4]-'0');

  DataBuf[19] = protocol_version / 100;
  DataBuf[20] = protocol_version % 100;

  DataBuf[21] = g_DeviceInfo.feeRuleVer[0];
  DataBuf[22] = g_DeviceInfo.feeRuleVer[1];
  DataBuf[23] = g_DeviceInfo.feeRuleVer[2];
  DataBuf[24] = g_DeviceInfo.feeRuleVer[3];

  DataBuf[25] = g_DeviceInfo.feeRuleVer[4];
  DataBuf[26] = g_DeviceInfo.feeRuleVer[5];
  DataBuf[27] = g_DeviceInfo.feeRuleVer[6];
  DataBuf[28] = g_DeviceInfo.feeRuleVer[7];

  data = JWT_membuffer_alloc((g_JWT_info_t.login_NO < 65533) ? g_JWT_info_t.login_NO++ : 0, CustomCloud_CMD_LOGIN, DataBuf, 29);
  print_hex_data(data,37);
  return data;
}

uint8* JWT_tx_0x04_card_charge_start(uint8 ch, uint8* data)
{
  uint8 i;
  uint8 DataBuf[30];
  uint64 ordeer_stamp;

  g_JWT_info_t.card_start_rx_flag = 0;
  g_RecordData.crgType = 0x01;
  g_RecordData.payMoney = 1;
  memcpy(&g_RecordData.crgStartTime[0], &g_JWT_info_t.heart_beat_time[0], 6);

  DataBuf[0] = ch + 1;
  DataBuf[1] = g_RecordData.crgMode;
  DataBuf[2] = g_sysRunningDataBase.crgModeData >> 24;
  DataBuf[3] = g_sysRunningDataBase.crgModeData >> 16;
  DataBuf[4] = g_sysRunningDataBase.crgModeData >> 8;
  DataBuf[5] = g_sysRunningDataBase.crgModeData;

  for (i = 0; i < 8; i++)
  {
    DataBuf[i + 6] = g_RecordData.cardNO[i];
  }

  ordeer_stamp = time(NULL);
  DataBuf[14] = g_RecordData.cardNO[5];
  DataBuf[15] = g_RecordData.cardNO[6];
  DataBuf[16] = g_RecordData.cardNO[7];

  DataBuf[17] = ordeer_stamp / 100000000;
  DataBuf[18] = ordeer_stamp / 1000000 % 100;
  DataBuf[19] = ordeer_stamp / 10000 % 100;
  DataBuf[20] = ordeer_stamp / 100 % 100;
  DataBuf[21] = ordeer_stamp % 100;

  for (i = 0; i < 8; i++)
  {
    g_JWT_info_t.order_NO[i] = DataBuf[i + 14];
    gChargeRecord[0].Reverse[i] = DataBuf[i + 14];
  }

  for (i = 0; i < 8; i++)
  {
    DataBuf[i + 22] = g_DeviceInfo.serNo[i];
  }

  data = JWT_membuffer_alloc(0x04, CustomCloud_CMD_CARD_START_CRG, DataBuf, 30);
  return data;
}

uint8* JWT_tx_0x05_card_charge_stop(uint8 ch, uint8* data)
{
  uint8 i;
  uint8 DataBuf[25];

  g_JWT_info_t.card_stop_rx_flag = 0;

  DataBuf[0] = ch + 1;

  for (i = 0; i < 8; i++)
  {
    DataBuf[i + 1] = g_RecordData.cardNO[i];
  }

  for (i = 0; i < 8; i++)
  {
    DataBuf[i + 9] = g_JWT_info_t.order_NO[i];
  }

  for (i = 0; i < 8; i++)
  {
    DataBuf[i + 17] = g_DeviceInfo.serNo[i];
  }

  return JWT_membuffer_alloc(0x05, CustomCloud_CMD_CARD_STOP_CRG, DataBuf, 25);
}

void JWT_rx_0x06_app_start_process(uint8 *data)
{
  uint8 i;
  uint8 Ch;
  uint8 buff[50];
  co_dword dword;
  UsrAccount Account = {0} ;

  memcpy(&g_RecordData.crgStartTime[0], &g_JWT_info_t.heart_beat_time[0], 6);

  memcpy(&buff[0], data, 40);
  Ch = (buff[6] == 2u) ? 1u : 0;
  dbg_printf(1," gun_NO:%d, APP resquest starting", Ch);

  Account.CrgMode = buff[7];

  dword.b[3] = buff[8];
  dword.b[2] = buff[9];
  dword.b[1] = buff[10];
  dword.b[0] = buff[11];
  Account.CrgModeVal = dword.v / 10;

  dword.b[3] = buff[12];
  dword.b[2] = buff[13];
  dword.b[1] = buff[14];
  dword.b[0] = buff[15];
  Account.Money = dword.v;

  for (i = 0; i < 8; i++)
  {
    Account.AccountNo[i] = buff[i + 18];
  }

  Ctrl_power_on(Ch, &Account);

  g_RecordData.crgType = 0x02;

  for (i = 0; i < 8; i++)
  {
    g_RecordData.cardNO[i] = 0u;
  }

  for (i = 0; i < 8; i++)
  {
    g_JWT_info_t.order_NO[i] = buff[i + 18];
  }
}

uint8* JWT_tx_0x86_app_charge_start_response(uint8 ch, uint8 result, uint16 serial_NO, uint8* data)
{
  uint8 i;
  uint8 DataBuf[60];

	if (result == 0)
  {
		g_RecordData.payMoney = 1;
	}

  for (i = 0; i < 8; i++)
  {
    DataBuf[i] = g_JWT_info_t.order_NO[i];
  }

  DataBuf[8] = result;

  for (i = 0; i < 8; i++)
  {
    DataBuf[i + 9] = g_DeviceInfo.serNo[i];
  }
  data = JWT_membuffer_alloc(serial_NO, CustomCloud_CMD_CTRL_START_CRG_RET, DataBuf, 17);
  return data;
}

uint8* JWT_tx_0x87_app_charge_stop_response(uint8 ch, uint8 result, uint16 serial_NO, uint8* data)
{
  uint8 i;
  uint8 DataBuf[17];

  for (i = 0; i < 8; i++)
  {
    DataBuf[i] = g_JWT_info_t.order_NO[i];
  }

  DataBuf[8] = result;

  for (i = 0; i < 8; i++)
  {
    DataBuf[i + 9] = g_DeviceInfo.serNo[i];
  }


  data = JWT_membuffer_alloc(serial_NO, CustomCloud_CMD_CTRL_STOP_CRG_RET, DataBuf, 17);
  return data;
}

uint8* JWT_tx_0x08_charge_record(uint8 ch, uint8* data)
{
  uint8 i;
  uint8 DataBuf[117];
  co_dword dword;

  DataBuf[0] = ch + 1;

  for (i = 0; i < 8; i++)
  {
    DataBuf[1 + i] = g_JWT_info_t.order_NO[i];
  }

  DataBuf[9] = g_RecordData.crgType;

  for (i = 0; i < 8; i++)
  {
    DataBuf[10 + i] = g_RecordData.cardNO[i];
  }

  for (i = 0; i < 17; i++)
  {
    DataBuf[18 + i] = 0x01;
  }

  DataBuf[35] = g_RecordData.currentSOC;

  DataBuf[36] = gChargeRecord[ch].StopReason;

  for (i = 0; i < 6; i++)
  {
    DataBuf[37 + i] = g_RecordData.crgStartTime[i];
  }

  for (i = 0; i < 6; i++)
  {
    DataBuf[43 + i] = g_RecordData.endStartTime[i];
  }

  dword.v = gChargeRecord[ch].StartEnerge*10;
  DataBuf[49] = dword.b[3];
  DataBuf[50] = dword.b[2];
  DataBuf[51] = dword.b[1];
  DataBuf[52] = dword.b[0];

  dword.v = gChargeRecord[ch].EndEnerge*10;
  DataBuf[53] = dword.b[3];
  DataBuf[54] = dword.b[2];
  DataBuf[55] = dword.b[1];
  DataBuf[56] = dword.b[0];

  dword.v = (gChargeRecord[ch].EndEnerge - gChargeRecord[ch].StartEnerge)*10;
  DataBuf[57] = dword.b[3];
  DataBuf[58] = dword.b[2];
  DataBuf[59] = dword.b[1];
  DataBuf[60] = dword.b[0];

  DataBuf[61] = g_JWT_info_t.sharp_E >> 24;
  DataBuf[62] = g_JWT_info_t.sharp_E >> 16;
  DataBuf[63] = g_JWT_info_t.sharp_E >> 8;
  DataBuf[64] = g_JWT_info_t.sharp_E;

  DataBuf[65] = g_JWT_info_t.peak_E >> 24;
  DataBuf[66] = g_JWT_info_t.peak_E >> 16;
  DataBuf[67] = g_JWT_info_t.peak_E >> 8;
  DataBuf[68] = g_JWT_info_t.peak_E;

  DataBuf[69] = g_JWT_info_t.flat_E >> 24;
  DataBuf[70] = g_JWT_info_t.flat_E >> 16;
  DataBuf[71] = g_JWT_info_t.flat_E >> 8;
  DataBuf[72] = g_JWT_info_t.flat_E;

  DataBuf[73] = g_JWT_info_t.valley_E >> 24;
  DataBuf[74] = g_JWT_info_t.valley_E >> 16;
  DataBuf[75] = g_JWT_info_t.valley_E >> 8;
  DataBuf[76] = g_JWT_info_t.valley_E;

  dword.v = gChargeRecord[ch].ChargeMoney*10;
  DataBuf[77] = dword.b[3];
  DataBuf[78] = dword.b[2];
  DataBuf[79] = dword.b[1];
  DataBuf[80] = dword.b[0];

  DataBuf[81] = g_JWT_info_t.sharp_money >> 24;
  DataBuf[82] = g_JWT_info_t.sharp_money >> 16;
  DataBuf[83] = g_JWT_info_t.sharp_money >> 8;
  DataBuf[84] = g_JWT_info_t.sharp_money;

  DataBuf[85] = g_JWT_info_t.peak_money >> 24;
  DataBuf[86] = g_JWT_info_t.peak_money >> 16;
  DataBuf[87] = g_JWT_info_t.peak_money >> 8;
  DataBuf[88] = g_JWT_info_t.peak_money;

  DataBuf[89] = g_JWT_info_t.flat_money >> 24;
  DataBuf[90] = g_JWT_info_t.flat_money >> 16;
  DataBuf[91] = g_JWT_info_t.flat_money >> 8;
  DataBuf[92] = g_JWT_info_t.flat_money;

  DataBuf[93] = g_JWT_info_t.valley_money >> 24;
  DataBuf[94] = g_JWT_info_t.valley_money >> 16;
  DataBuf[95] = g_JWT_info_t.valley_money >> 8;
  DataBuf[96] = g_JWT_info_t.valley_money;

  DataBuf[97] = 0x00;
  DataBuf[98] = 0x00;
  DataBuf[99] = 0x00;
  DataBuf[100] = 0x00;

  dword.v = gChargeRecord[ch].crgServiceFee*10;
  DataBuf[101] = dword.b[3];
  DataBuf[102] = dword.b[2];
  DataBuf[103] = dword.b[1];
  DataBuf[104] = dword.b[0];

  DataBuf[105] = 0x00;
  DataBuf[106] = 0x00;
  DataBuf[107] = 0x00;
  DataBuf[108] = 0x00;

  for (i = 0; i < 8; i++)
  {
    DataBuf[i + 109] = g_DeviceInfo.serNo[i];
  }

  return JWT_membuffer_alloc(0x08, CustomCloud_CMD_CRGRECORD_REPORT, DataBuf, 117);
}

uint8* JWT_tx_0x09_charge_progress(uint8 ch, uint8* data)
{
  uint8    i;
  uint8    DataBuf[120];
  co_dword dword;

  DataBuf[0] = ch + 1;

  for (i = 0; i < 8; i++)
  {
    DataBuf[i + 1] = g_JWT_info_t.order_NO[i];
  }

  DataBuf[9] = g_RecordData.crgType;

  for (i = 0; i < 8; i++)
  {
    DataBuf[i + 10] = g_RecordData.cardNO[i];
  }

  for (i = 0; i < 17; i++)
  {
    DataBuf[i + 18] = 0x31;
  }

  DataBuf[35] = g_RecordData.currentSOC;

  for (i = 0; i < 6; i++)
  {
    DataBuf[i + 36] = g_RecordData.crgStartTime[i];
  }

  for (i = 0; i < 6; i++)
  {
    DataBuf[i + 42] = g_RecordData.endStartTime[i];
  }

  dword.v =  (gChargeRecord[ch].EndEnerge - gChargeRecord[ch].StartEnerge)*10;
  DataBuf[48] = dword.b[3];
  DataBuf[49] = dword.b[2];
  DataBuf[50] = dword.b[1];
  DataBuf[51] = dword.b[0];
//add
  g_JWT_info_t.sharp_E =0;
  g_JWT_info_t.peak_E = 0;
  g_JWT_info_t.flat_E = 0;
  g_JWT_info_t.valley_E = 0;
  g_JWT_info_t.sharp_money = 0;
  g_JWT_info_t.peak_money = 0;
  g_JWT_info_t.flat_money = 0;
  g_JWT_info_t.valley_money = 0;
  uint32_t fee_val[4] = {0};
  for(i = 0; i < 48;i++)
  {
      if(g_Sys_Config_Param.FeeTime[i] == 1)
      {
          g_JWT_info_t.sharp_E += gChargeRecord[0].SectionEnerge[i];
          fee_val[0] = g_Sys_Config_Param.Price[i];
      }
      else if(g_Sys_Config_Param.FeeTime[i] == 2)
      {
          g_JWT_info_t.peak_E += gChargeRecord[0].SectionEnerge[i];
          fee_val[1] = g_Sys_Config_Param.Price[i];
      }
      else if(g_Sys_Config_Param.FeeTime[i] == 3)
      {
          g_JWT_info_t.flat_E += gChargeRecord[0].SectionEnerge[i];
          fee_val[2] = g_Sys_Config_Param.Price[i];
      }
      else
      {
          g_JWT_info_t.valley_E += gChargeRecord[0].SectionEnerge[i];
          fee_val[3] = g_Sys_Config_Param.Price[i];
      }
  }
  g_JWT_info_t.sharp_money = g_JWT_info_t.sharp_E * fee_val[0]/10;
  g_JWT_info_t.peak_money = g_JWT_info_t.peak_E * fee_val[1]/10;
  g_JWT_info_t.flat_money = g_JWT_info_t.flat_E * fee_val[2]/10;
  g_JWT_info_t.valley_money = g_JWT_info_t.valley_E * fee_val[3]/10;

  g_JWT_info_t.sharp_E *= 10; //放大10倍 系统使用两位小数
  g_JWT_info_t.peak_E *= 10; //
  g_JWT_info_t.flat_E *= 10; //
  g_JWT_info_t.valley_E *= 10; //
//end
  DataBuf[52] = g_JWT_info_t.sharp_E >> 24;
  DataBuf[53] = g_JWT_info_t.sharp_E >> 16;
  DataBuf[54] = g_JWT_info_t.sharp_E >> 8;
  DataBuf[55] = g_JWT_info_t.sharp_E;

  DataBuf[56] = g_JWT_info_t.peak_E >> 24;
  DataBuf[57] = g_JWT_info_t.peak_E >> 16;
  DataBuf[58] = g_JWT_info_t.peak_E >> 8;
  DataBuf[59] = g_JWT_info_t.peak_E;

  DataBuf[60] = g_JWT_info_t.flat_E >> 24;
  DataBuf[61] = g_JWT_info_t.flat_E >> 16;
  DataBuf[62] = g_JWT_info_t.flat_E >> 8;
  DataBuf[63] = g_JWT_info_t.flat_E;

  DataBuf[64] = g_JWT_info_t.valley_E >> 24;
  DataBuf[65] = g_JWT_info_t.valley_E >> 16;
  DataBuf[66] = g_JWT_info_t.valley_E >> 8;
  DataBuf[67] = g_JWT_info_t.valley_E;

  dword.v = gChargeRecord[ch].ChargeMoney*10;
  DataBuf[68] = dword.b[3];
  DataBuf[69] = dword.b[2];
  DataBuf[70] = dword.b[1];
  DataBuf[71] = dword.b[0];

  DataBuf[72] = g_JWT_info_t.sharp_money >> 24;
  DataBuf[73] = g_JWT_info_t.sharp_money >> 16;
  DataBuf[74] = g_JWT_info_t.sharp_money >> 8;
  DataBuf[75] = g_JWT_info_t.sharp_money;

  DataBuf[76] = g_JWT_info_t.peak_money >> 24;
  DataBuf[77] = g_JWT_info_t.peak_money >> 16;
  DataBuf[78] = g_JWT_info_t.peak_money >> 8;
  DataBuf[79] = g_JWT_info_t.peak_money;

  DataBuf[80] = g_JWT_info_t.flat_money >> 24;
  DataBuf[81] = g_JWT_info_t.flat_money >> 16;
  DataBuf[82] = g_JWT_info_t.flat_money >> 8;
  DataBuf[83] = g_JWT_info_t.flat_money;

  DataBuf[84] = g_JWT_info_t.valley_money >> 24;
  DataBuf[85] = g_JWT_info_t.valley_money >> 16;
  DataBuf[86] = g_JWT_info_t.valley_money >> 8;
  DataBuf[87] = g_JWT_info_t.valley_money;

  DataBuf[88] = 0x00;
  DataBuf[89] = 0x00;
  DataBuf[90] = 0x00;
  DataBuf[91] = 0x00;

  dword.v = gChargeRecord[ch].crgServiceFee*10;
  DataBuf[92] = dword.b[3];
  DataBuf[93] = dword.b[2];
  DataBuf[94] = dword.b[1];
  DataBuf[95] = dword.b[0];

  DataBuf[96] = 0x00;
  DataBuf[97] = 0x00;
  DataBuf[98] = 0x00;
  DataBuf[99] = 0x00;

  dword.v = gChargeRecord[ch].ChargeTime;;
  DataBuf[100] = dword.b[3];
  DataBuf[101] = dword.b[2];
  DataBuf[102] = dword.b[1];
  DataBuf[103] = dword.b[0];

  for (i = 0; i < 8; i++)
  {
    DataBuf[i + 104] = g_DeviceInfo.serNo[i];
  }
  return JWT_membuffer_alloc((g_JWT_info_t.progress_NO < 65533) ? g_JWT_info_t.heart_beat_NO++ : 0, CustomCloud_CMD_CRG_REALDATA, DataBuf, 112);
}

uint8* JWT_tx_0x32_charge_progress(uint8 ch, uint8* data,uint16 *len)
{
  uint8    i;
  uint8    DataBuf[400];
  co_dword dword;
  uint16   index=0;


  for (i = 0; i < 8; i++)
  {
    DataBuf[i + index] = g_JWT_info_t.order_NO[i];
  }
  index += 8;

  DataBuf[index++] = g_RecordData.crgType;

  for (i = 0; i < 8; i++)
  {
    DataBuf[i + index] = g_RecordData.cardNO[i];
  }
  index += 8;

  for (i = 0; i < 17; i++)
  {
    DataBuf[i + index] = 0x31;
  }
  index += 17;

  DataBuf[index++] = g_RecordData.currentSOC;

  dword.v = gChargeRecord[ch].ChargeTime;;
  DataBuf[index++] = dword.b[3];
  DataBuf[index++] = dword.b[2];
  DataBuf[index++] = dword.b[1];
  DataBuf[index++] = dword.b[0];

  dword.v =  (gChargeRecord[ch].EndEnerge - gChargeRecord[ch].StartEnerge)*10;
  DataBuf[index++] = dword.b[3];
  DataBuf[index++] = dword.b[2];
  DataBuf[index++] = dword.b[1];
  DataBuf[index++] = dword.b[0];

  dword.v =  gChargeRecord[ch].ChargeMoney *10;
  DataBuf[index++] = dword.b[3];
  DataBuf[index++] = dword.b[2];
  DataBuf[index++] = dword.b[1];
  DataBuf[index++] = dword.b[0];

  dword.v = gChargeRecord[ch].crgServiceFee*10;
  DataBuf[index++] = dword.b[3];
  DataBuf[index++] = dword.b[2];
  DataBuf[index++] = dword.b[1];
  DataBuf[index++] = dword.b[0];


  JWT_feenum_callback();
  DataBuf[index++] = stage_fee.feenum ;
  for(i=0;i<stage_fee.feenum;i++)
  {
    DataBuf[index++] = stage_fee.fee_stype[i];
    memcpy(DataBuf + index,stage_fee.fee_start_time[i],6);
    index += 6;
    memcpy(DataBuf + index,stage_fee.fee_end_time[i],6);
    index += 6;
    dword.v = stage_fee.elefee[i]*10;
    DataBuf[index++] = dword.b[1];
    DataBuf[index++] = dword.b[0];
    dword.v = stage_fee.serfee[i]*10;
    DataBuf[index++] = dword.b[1];
    DataBuf[index++] = dword.b[0];
    dword.v = stage_fee.stage_energy[i]*10;
    DataBuf[index++] = dword.b[3];
    DataBuf[index++] = dword.b[2];
    DataBuf[index++] = dword.b[1];
    DataBuf[index++] = dword.b[0];
    dword.v = stage_fee.stage_elefee[i]*10;
    DataBuf[index++] = dword.b[3];
    DataBuf[index++] = dword.b[2];
    DataBuf[index++] = dword.b[1];
    DataBuf[index++] = dword.b[0];
    dword.v = stage_fee.stage_serfee[i]*10;
    DataBuf[index++] = dword.b[3];
    DataBuf[index++] = dword.b[2];
    DataBuf[index++] = dword.b[1];
    DataBuf[index++] = dword.b[0];
  }

  DataBuf[index++] = ch + 1;

  for (i = 0; i < 8; i++)
  {
    DataBuf[i + index] = g_DeviceInfo.serNo[i];
  }
  index += 8;

  *len = index + 8;
  return JWT_membuffer_alloc((g_JWT_info_t.progress_NO < 65533) ? g_JWT_info_t.heart_beat_NO++ : 0, 0x32, DataBuf, index);
}


uint8* JWT_tx_0x0A_charge_progress_data(uint8 ch, uint8* data)
{
  uint8 i;
  uint8 DataBuf[160];
  co_dword dword;
  MEASURE_DATA st_Meate_Data = Get_Measrue_Data(0);

  DataBuf[0] = ch + 1;

  for (i = 0; i < 8; i++)
  {
    DataBuf[i + 1] = g_JWT_info_t.order_NO[i];
  }
    #if 0

  for (i = 0; i < 59; i++)
  {
    DataBuf[i + 9] = 0x00;
  }

  #else
  DataBuf[9] = g_Sys_Config_Param.LocalIp[0];
  DataBuf[10] = g_Sys_Config_Param.LocalIp[1];
  DataBuf[11] = g_Sys_Config_Param.LocalIp[2];
  DataBuf[12] = g_Sys_Config_Param.LocalIp[3];
  dword.v = g_Sys_Config_Param.restart_time;
  DataBuf[13] = dword.b[3];
  DataBuf[14] = dword.b[2];
  DataBuf[15] = dword.b[1];
  DataBuf[16] = dword.b[0];
  dword.v = get_test_num(1);
  DataBuf[17] = dword.b[3];
  DataBuf[18] = dword.b[2];
  DataBuf[19] = dword.b[1];
  DataBuf[20] = dword.b[0];
  dword.v = get_test_num(2);
  DataBuf[21] = dword.b[3];
  DataBuf[22] = dword.b[2];
  DataBuf[23] = dword.b[1];
  DataBuf[24] = dword.b[0];
  for (i = 0; i < 43; i++)
  {
    DataBuf[i + 25] = 0x00;
  }
  #endif
  DataBuf[68] = Get_system_Temp() >> 24;
  DataBuf[69] = Get_system_Temp() >> 16;
  DataBuf[70] = Get_system_Temp() >> 8;
  DataBuf[71] = Get_system_Temp();

  DataBuf[72] = Get_Gun_Temp() >> 24;
  DataBuf[73] = Get_Gun_Temp() >> 16;
  DataBuf[74] = Get_Gun_Temp() >> 8;
  DataBuf[75] = Get_Gun_Temp();

  dword.v = gChargeRecord[ch].MaxVolt*100;
  DataBuf[76] = dword.b[3];
  DataBuf[77] = dword.b[2];
  DataBuf[78] = dword.b[1];
  DataBuf[79] = dword.b[0];

  dword.v = gChargeRecord[ch].MaxCurent*10;
  DataBuf[80] = dword.b[3];
  DataBuf[81] = dword.b[2];
  DataBuf[82] = dword.b[1];
  DataBuf[83] = dword.b[0];

  dword.v = (gChargeRecord[ch].EndEnerge - gChargeRecord[ch].StartEnerge)*1000;
  DataBuf[84] = dword.b[3];
  DataBuf[85] = dword.b[2];
  DataBuf[86] = dword.b[1];
  DataBuf[87] = dword.b[0];

  dword.v = st_Meate_Data.VolageA*100;
  DataBuf[88] = dword.b[3];
  DataBuf[89] = dword.b[2];
  DataBuf[90] = dword.b[1];
  DataBuf[91] = dword.b[0];

  dword.v = st_Meate_Data.CurrentA*10;
  DataBuf[92] = dword.b[3];
  DataBuf[93] = dword.b[2];
  DataBuf[94] = dword.b[1];
  DataBuf[95] = dword.b[0];

  dword.v = (gChargeRecord[ch].EndEnerge - gChargeRecord[ch].StartEnerge)*1000;
  DataBuf[96] = dword.b[3];
  DataBuf[97] = dword.b[2];
  DataBuf[98] = dword.b[1];
  DataBuf[99] = dword.b[0];

  DataBuf[100] = 0x00;
  DataBuf[101] = 0x00;
  DataBuf[102] = 0x00;
  DataBuf[103] = 0x00;

  DataBuf[104] = 0x00;
  DataBuf[105] = 0x00;
  DataBuf[106] = 0x00;
  DataBuf[107] = 0x00;

  DataBuf[108] = st_Meate_Data.VolageA >> 24;
  DataBuf[109] = st_Meate_Data.VolageA >> 16;
  DataBuf[110] = st_Meate_Data.VolageA >> 8;
  DataBuf[111] = st_Meate_Data.VolageA;

  DataBuf[112] = st_Meate_Data.VolageB >> 24;
  DataBuf[113] = st_Meate_Data.VolageB >> 16;
  DataBuf[114] = st_Meate_Data.VolageB >> 8;
  DataBuf[115] = st_Meate_Data.VolageB;

  DataBuf[116] = st_Meate_Data.VolageC >> 24;
  DataBuf[117] = st_Meate_Data.VolageC >> 16;
  DataBuf[118] = st_Meate_Data.VolageC >> 8;
  DataBuf[119] = st_Meate_Data.VolageC;

  DataBuf[120] = st_Meate_Data.CurrentA >> 24;
  DataBuf[121] = st_Meate_Data.CurrentA >> 16;
  DataBuf[122] = st_Meate_Data.CurrentA >> 8;
  DataBuf[123] = st_Meate_Data.CurrentA;

  DataBuf[124] = st_Meate_Data.CurrentB >> 24;
  DataBuf[125] = st_Meate_Data.CurrentB >> 16;
  DataBuf[126] = st_Meate_Data.CurrentB >> 8;
  DataBuf[127] = st_Meate_Data.CurrentB;

  DataBuf[128] = st_Meate_Data.CurrentC >> 24;
  DataBuf[129] = st_Meate_Data.CurrentC >> 16;
  DataBuf[130] = st_Meate_Data.CurrentC >> 8;
  DataBuf[131] = st_Meate_Data.CurrentC;

  for (i = 0; i < 8; i++)
  {
    DataBuf[i + 132] = g_DeviceInfo.serNo[i];
  }

  return JWT_membuffer_alloc((g_JWT_info_t.progress_data_NO < 65533) ? g_JWT_info_t.heart_beat_NO++ : 0, CustomCloud_CMD_MONIROT_DATA, DataBuf, 140);
}

//2020.12.3
//void JWT_rx_0x0B_set_billing_rule_process(uint8 *data)
//{
//    uint32_t Fee_val[4];
//    int fee_num = 0;
//    int index = 6;
//    int k = 0;
//    int i = 0;
//
//    co_dword dword;
//    memcpy(g_Sys_Config_Param.FeeVersion,data + index,4);
//    index += 4;
//    memcpy(g_Sys_Config_Param.FeeVersion + 4,data + index,4);
//    index += 4;
//
//    index += 6;
//    index += 2;//book
//
//    dword.b[1] = data[index++];
//    dword.b[0] = data[index++];
//    g_Sys_Config_Param.CrgServFee = dword.v/10;  //缩小10倍 系统使用两位小数
//    dbg_printf(1,"CrgServFee %d",g_Sys_Config_Param.CrgServFee);
//    index += 2;//park
//
//    dword.b[1] = data[index++];
//    dword.b[0] = data[index++];
//    Fee_val[0] = dword.v/10;
//
//    dword.b[1] = data[index++];
//    dword.b[0] = data[index++];
//    Fee_val[1] = dword.v/10;
//
//    dword.b[1] = data[index++];
//    dword.b[0] = data[index++];
//    Fee_val[2] = dword.v/10;
//
//    dword.b[1] = data[index++];
//    dword.b[0] = data[index++];
//    Fee_val[3] = dword.v/10;
//
//    fee_num = data[index++];
//    dbg_printf(1,"fee num %d",fee_num);
//
//    dbg_printf(1,"fee val0 %d",Fee_val[0]);
//    dbg_printf(1,"fee val1 %d",Fee_val[1]);
//    dbg_printf(1,"fee val2 %d",Fee_val[2]);
//    dbg_printf(1,"fee val3 %d",Fee_val[3]);
//
//    for(i = 0; i < fee_num ;i++)
//    {
//        int end_h = 0;
//        int end_m = 0;
//        int leve = 0;
//        int s = 0;
//        index += 2;//
//        //end time
//        end_h = BcdToHex8(data[index++]);
//        end_m = BcdToHex8(data[index++]);
//        s = end_h * 2;
//        leve = data[index++];
//        if(leve > 4 || leve == 0)
//            leve = 1;
//
//        if(end_m >= 30)
//        {
//            s += 1;
//        }
//        if(s > 48)
//            s = 48;
//        for(;k < s;k++)
//        {
//            g_Sys_Config_Param.Price[k] = Fee_val[leve-1];
//            g_Sys_Config_Param.FeeTime[k] = leve;
//            dbg_printf(1,"fee val[%d] = %d",k,g_Sys_Config_Param.Price[k]);
//        }
//
//    }
//    if(k <= 48)
//    {
//        g_Sys_Config_Param.Price[47] = g_Sys_Config_Param.Price[46];
//        g_Sys_Config_Param.FeeTime[47] = g_Sys_Config_Param.FeeTime[46];
//    }
//    dbg_printf(1,"fee val[%d] = %d",k,g_Sys_Config_Param.Price[k]);
//    g_Sys_Config_Param.CrcVal = CRC16((uint16_t * )&g_Sys_Config_Param,sizeof(g_Sys_Config_Param)/2 - 1);
//    SPIFLASH_Method1_Save_Record(TYPE_SYSPARA,(uint8_t *)&g_Sys_Config_Param);
//    SPIFLASH_Method1_Save_Record(TYPE_FACTORY_PARAM,(uint8_t *)&g_Sys_Config_Param);
//
//}

void JWT_rx_0x0B_set_billing_rule_process(uint8 *data)
{
    uint32_t Fee_val[4];
    int fee_num = 0;
    int index = 6;
    int k = 0;
    int i = 0;

    co_dword dword;
    memcpy(g_Sys_Config_Param.FeeVersion,data + index,4);
    index += 4;
    memcpy(g_Sys_Config_Param.FeeVersion + 4,data + index,4);
    index += 4;

    index += 6;
    index += 2;//book

    dword.b[1] = data[index++];
    dword.b[0] = data[index++];
    g_Sys_Config_Param.CrgServFee = dword.v/10;  //缩小10倍 系统使用两位小数
    dbg_printf(1,"CrgServFee %d",g_Sys_Config_Param.CrgServFee);
    index += 2;//park

    dword.b[1] = data[index++];
    dword.b[0] = data[index++];
    Fee_val[0] = dword.v/10;

    dword.b[1] = data[index++];
    dword.b[0] = data[index++];
    Fee_val[1] = dword.v/10;

    dword.b[1] = data[index++];
    dword.b[0] = data[index++];
    Fee_val[2] = dword.v/10;

    dword.b[1] = data[index++];
    dword.b[0] = data[index++];
    Fee_val[3] = dword.v/10;

    fee_num = data[index++];
    dbg_printf(1,"fee num %d",fee_num);

    dbg_printf(1,"fee val0 %d",Fee_val[0]);
    dbg_printf(1,"fee val1 %d",Fee_val[1]);
    dbg_printf(1,"fee val2 %d",Fee_val[2]);
    dbg_printf(1,"fee val3 %d",Fee_val[3]);

    for(i = 0; i < fee_num ;i++)
    {
        int end_h = 0;
        int end_m = 0;
        int leve = 0;
        int s = 0;
        index += 2;//
        //end time
        end_h = BcdToHex8(data[index++]);
        end_m = BcdToHex8(data[index++]);

        s = end_h * 2;
        leve = data[index++];
        if(leve > 4 || leve == 0)
            leve = 1;

        if(end_m >= 30)
        {
            s += 1;
        }
        if(s > 48)
            s = 48;
        for(;k < s;k++)
        {
            g_Sys_Config_Param.Price[k] = Fee_val[leve-1];
            g_Sys_Config_Param.FeeTime[k] = leve;
            dbg_printf(1,"fee val[%d] = %d",k,g_Sys_Config_Param.Price[k]);
        }

    }
    if(k <= 48)
    {
        g_Sys_Config_Param.Price[47] = g_Sys_Config_Param.Price[46];
        g_Sys_Config_Param.FeeTime[47] = g_Sys_Config_Param.FeeTime[46];
    }
    dbg_printf(1,"fee val[%d] = %d",k,g_Sys_Config_Param.Price[k]);
    g_Sys_Config_Param.CrcVal = CRC16((uint16_t * )&g_Sys_Config_Param,sizeof(g_Sys_Config_Param)/2 - 1);
    SPIFLASH_Method1_Save_Record(TYPE_SYSPARA,(uint8_t *)&g_Sys_Config_Param);
    SPIFLASH_Method1_Save_Record(TYPE_FACTORY_PARAM,(uint8_t *)&g_Sys_Config_Param);

}


uint8* JWT_tx_0x8B_billing_rule_set_response(uint8* data)
{
  uint8 i;
  uint8 DataBuf[10];

  for (i = 0; i < 4; i++)
  {
    DataBuf[i] = g_Sys_Config_Param.FeeVersion[i];
  }

  for (i = 0; i < 4; i++)
  {
    DataBuf[i + 4] = g_Sys_Config_Param.FeeVersion[i+4];
  }

  DataBuf[8] = 0x00;
  for (i = 0; i < 8; i++)
  {
    DataBuf[i + 9] = g_DeviceInfo.serNo[i];
  }

  data = JWT_membuffer_alloc(0x0B, CustomCloud_CMD_SET_FEE_RET_8B, DataBuf, 17);
  return data;
}

uint8* JWT_tx_0x0B_billing_rule_query_response(uint8* data)
{
  uint8 i;
  uint8 DataBuf[100];
  uint16 fee_val[4]={0};
  uint16 index = 0;
  int k = 0;

  uint16 fee_time_start[48] = {0};
  uint16 fee_time_end[48] = {0};
  uint16 fee_time_val[48] = {0};
  for (i = 0; i < 4; i++)
  {
    DataBuf[i] = g_Sys_Config_Param.FeeVersion[i];
  }

  for (i = 0; i < 4; i++)
  {
    DataBuf[i + 4] = g_Sys_Config_Param.FeeVersion[i+4];
  }

  for (i = 0; i < 6; i++)
  {
    DataBuf[i + 8] = g_JWT_info_t.billing_effect_time[i];
  }

  DataBuf[14] = 0x00;
  DataBuf[15] = 0x00;

  DataBuf[16] = ((g_Sys_Config_Param.CrgServFee*10) >> 8);
  DataBuf[17] = (g_Sys_Config_Param.CrgServFee*10) & 0xff;

#if 0

#else
	//获取4个费率
  for(i = 0;i<48;i++)
  {
    if(g_Sys_Config_Param.FeeTime[i] == 1)
    {
        fee_val[0] = g_Sys_Config_Param.Price[i] * 10;
    }
    else if(g_Sys_Config_Param.FeeTime[i] == 2)
    {
        fee_val[1] = g_Sys_Config_Param.Price[i] * 10;
    }
    else if(g_Sys_Config_Param.FeeTime[i] == 3)
    {
        fee_val[2] = g_Sys_Config_Param.Price[i] * 10;
    }
    else if(g_Sys_Config_Param.FeeTime[i] == 4)
    {
        fee_val[3] = g_Sys_Config_Param.Price[i] * 10;
    }
  }
  //dbg_printf(1,"fee_val[0]=%x",fee_val[0]);

  index = 20;

  for(i = 0;i < 4;i++)
  {
      DataBuf[index++] = fee_val[i]>>8;
      DataBuf[index++] = fee_val[i]&0xff;
  }

//第1个时段从 00：00开始
  fee_time_start[0] = 0;
  fee_time_val[0] = g_Sys_Config_Param.FeeTime[0];

  for(i = 0;i<47;i++)
  {
    if(g_Sys_Config_Param.FeeTime[i] != g_Sys_Config_Param.FeeTime[i + 1])
    {
        int hour = 0;
        int min = 0;
        hour = (i+1)/2;
        if(((i+1)%2) != 0)
            min = 30;
        fee_time_end[k] = (HexToBcd(hour) << 8) + HexToBcd(min);

        k++;


		hour = (i+1)/2;
        if(((i+1)%2) != 0)
            min = 30;
        fee_time_start[k] = (HexToBcd(hour) << 8) + HexToBcd(min);

		//下一时段的费率标志值
		fee_time_val[k] = g_Sys_Config_Param.FeeTime[i+1];
    }
  }

  //最后时段 23：59
  fee_time_end[k] = (HexToBcd(23) << 8) + HexToBcd(59);

  DataBuf[index++] = k+1;
  if(k == 0)
       k = 1;
  for(i = 0;i < k+1;i++)
  {
      DataBuf[index++] = fee_time_start[i]>>8;
      DataBuf[index++] = fee_time_start[i]&0xff;

      DataBuf[index++] = fee_time_end[i]>>8;
      DataBuf[index++] = fee_time_end[i]&0xff;

	  DataBuf[index++] = fee_time_val[i];
  }

#endif
  for (i = 0; i < 8; i++)
  {
    DataBuf[index++] = g_DeviceInfo.serNo[i];
  }
  data = JWT_membuffer_alloc(0x0B, 0x0B, DataBuf, index);

  comm.Send(comm.Port,data,index+8,1);
  comm.SendLen=index+8;
  return data;
}

uint8* JWT_tx_0x0C_heart_beat(uint8 ch, uint8* data)
{
	uint8 i;
  uint8 DataBuf[10];

  DataBuf[0] = 0x01;

  if (Get_Alarm_State(ch) != FALSE)
  {
    DataBuf[1] = 0x05;
  }
  else
  {
    switch (g_Charge_Ctrl_Manager[ch].CurState)
      {
        case STATE_CHARGEING:
          {
            DataBuf[1] = 0x02;
          }
          break;

        case STATE_END_CHARGE:
          {
            DataBuf[1] = 0x03;
          }
          break;

        case STATE_BOOK:
          {
            DataBuf[1] = 0x04;
          }
          break;

        case STATE_IDLE:
          {
            if (Get_Car_Link(ch) != LEVEL_12_V)
            {
              DataBuf[1] = 0x01;
            }
            else
            {
              DataBuf[1] = 0x00;
            }
          }
          break;

        default:
          break;
      }
  }

	for (i = 0; i < 8; i++)
  {
    DataBuf[i + 2] = g_DeviceInfo.serNo[i];
  }
  return JWT_membuffer_alloc((g_JWT_info_t.heart_beat_NO < 65533) ? g_JWT_info_t.heart_beat_NO++ : 0, CustomCloud_CMD_HEART, DataBuf, 10);
}

uint8* JWT_tx_0x0D_charger_warning(uint8 ch, uint16 warning_code, uint8 warning_status, uint32 warning_value, uint8* data)
{
  uint8 i;
  uint8 DataBuf[22];
  co_dword dword;
  struct tm *st_time;
  uint32 now_time= time(NULL);
  DataBuf[0] = ch + 1;

  dword.w[0] = warning_code;
  DataBuf[1] = dword.b[1];
  DataBuf[2] = dword.b[0];

  st_time = gmtime((const time_t *)&now_time);
  DataBuf[3] = HexToBcd(st_time->tm_year - 100);
  DataBuf[4] = HexToBcd(st_time->tm_mon + 1);
  DataBuf[5] = HexToBcd(st_time->tm_mday);
  DataBuf[6] = HexToBcd(st_time->tm_hour);
  DataBuf[7] = HexToBcd(st_time->tm_min);
  DataBuf[8] = HexToBcd(st_time->tm_sec);

//  DataBuf[3] = 0x00;
//  DataBuf[4] = 0x00;
//  DataBuf[5] = 0x00;
//  DataBuf[6] = 0x00;
//  DataBuf[7] = 0x00;
//  DataBuf[8] = 0x00;

  DataBuf[9] = warning_status;

  dword.v = warning_value;
  DataBuf[10] = dword.b[3];
  DataBuf[11] = dword.b[2];
  DataBuf[12] = dword.b[1];
  DataBuf[13] = dword.b[0];

  for (i = 0; i < 8; i++)
  {
    DataBuf[i + 14] = g_DeviceInfo.serNo[i];
  }

  data = JWT_membuffer_alloc(0x0D, CustomCloud_CMD_SEND_FAULT, DataBuf, 22);
  return data;
}

uint8* JWT_tx_setting_response(uint8 cmd, uint8 result, uint8* data)
{
  uint8 i;
  uint8 DataBuf[9];

  DataBuf[0] = result;
  for (i = 0; i < 8; i++)
  {
    DataBuf[i + 1] = g_DeviceInfo.serNo[i];
  }

  return JWT_membuffer_alloc(cmd, cmd, DataBuf, 9);
}

uint8* JWT_tx_0x1C_QR_request(uint8 ch, uint8* data)
{
  uint8 i;
  uint8 DataBuf[9];

  DataBuf[0] = ch + 1;
  DataBuf[1] = ((g_Sys_Config_Param.Serial[0] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[1] & 0x0F);
  DataBuf[2] = ((g_Sys_Config_Param.Serial[2] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[3] & 0x0F);
  DataBuf[3] = ((g_Sys_Config_Param.Serial[4] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[5] & 0x0F);
  DataBuf[4] = ((g_Sys_Config_Param.Serial[6] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[7] & 0x0F);
  DataBuf[5] = ((g_Sys_Config_Param.Serial[8] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[9] & 0x0F);
  DataBuf[6] = ((g_Sys_Config_Param.Serial[10] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[11] & 0x0F);
  DataBuf[7] = ((g_Sys_Config_Param.Serial[12] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[13] & 0x0F);
  DataBuf[8] = ((g_Sys_Config_Param.Serial[14] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[15] & 0x0F);

  data = JWT_membuffer_alloc(0x1C, CustomCloud_CMD_SET_QRCode, DataBuf, 9);
  return data;
}

uint8* JWT_tx_0x1F_gun_status(uint8 ch, uint8* data)
{
  uint8 i;
  uint8 DataBuf[10];

  DataBuf[0] = ch + 1;
  if (Get_Alarm_State(ch) != FALSE)
  {
    DataBuf[1] = 0x05;
  }
  else
  {
    switch (g_Charge_Ctrl_Manager[ch].CurState)
      {
        case STATE_CHARGEING:
          {
            DataBuf[1] = 0x02;
          }
          break;

        case STATE_END_CHARGE:
          {
            DataBuf[1] = 0x03;
          }
          break;

        case STATE_BOOK:
          {
            DataBuf[1] = 0x04;
          }
          break;

        case STATE_IDLE:
          {
            if (Get_Car_Link(ch) != LEVEL_12_V)
            {
              DataBuf[1] = 0x01;
            }
            else
            {
              DataBuf[1] = 0x00;
            }
          }
          break;

        default:
          break;
      }
   }
  for (i = 0; i < 8; i++)
  {
    DataBuf[i + 2] = g_DeviceInfo.serNo[i];
  }

  data = JWT_membuffer_alloc(0x1C, CustomCloud_CMD_SEND_GUN_STATUS, DataBuf, 10);
  //print_hex_data(data,19);
  return data;
}

void JWT_rx_0xA2_TCP_Update(uint8 *data)
{
  uint8 ptr = 0;
  co_dword dword;
  ptr +=7;

  memcpy(TCP_Info.SoftVer,SoftVer,16);
  TCP_Info.SoftVer[1] = data[ptr]+'0';
  ptr ++;
  TCP_Info.SoftVer[3] = ( data[ptr] / 10 )+ '0';
  TCP_Info.SoftVer[4] = ( data[ptr] % 10 )+ '0';
  ptr ++;

  memcpy(TCP_Info.TCP_File_name,&data[ptr],32);
  ptr +=32;
  dbg_printf(1,"TCP_File_name : %s  ", TCP_Info.TCP_File_name);

  dword.b[3] = data[ptr];
  ptr ++;
  dword.b[2] = data[ptr];
  ptr ++;
  dword.b[1] = data[ptr];
  ptr ++;
  dword.b[0] = data[ptr];
  ptr ++;
  TCP_Info.TCP_File_length = dword.v;
  dbg_printf(1,"TCP_File_length : %d  ", TCP_Info.TCP_File_length);


}

uint8* JWT_tx_0x22_TCP_Update_response(uint8 *data , uint8 result)
{
//result 0-准备就绪		 1-准备失败 2-下发文件有误
  uint8 i;
  uint8 DataBuf[20];
  co_dword dword;

  DataBuf[0] = TCP_Info.SoftVer[1]- '0';
  DataBuf[1] = ((TCP_Info.SoftVer[3] -'0')*10) + (TCP_Info.SoftVer[4] -'0');


  if(result == 0)
  {
   DataBuf[2] = 0x00;
  }
  else if(result == 1)
  {
   DataBuf[2] = 0x01;
  }
  else if(result == 2)
  {
   DataBuf[2] = 0x02;
  }


  //单包长度为      256=0x100
  DataBuf[3] = 0x01;
  DataBuf[4] = 0x00;

  //总包数为 784 = 196*1024/256 =0x310
  DataBuf[5] = 0x03;
  DataBuf[6] = 0x10;


  DataBuf[7] = ((g_Sys_Config_Param.Serial[0] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[1] & 0x0F);
  DataBuf[8] = ((g_Sys_Config_Param.Serial[2] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[3] & 0x0F);
  DataBuf[9] = ((g_Sys_Config_Param.Serial[4] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[5] & 0x0F);
  DataBuf[10] = ((g_Sys_Config_Param.Serial[6] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[7] & 0x0F);
  DataBuf[11] = ((g_Sys_Config_Param.Serial[8] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[9] & 0x0F);
  DataBuf[12] = ((g_Sys_Config_Param.Serial[10] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[11] & 0x0F);
  DataBuf[13] = ((g_Sys_Config_Param.Serial[12] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[13] & 0x0F);
  DataBuf[14] = ((g_Sys_Config_Param.Serial[14] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[15] & 0x0F);

  data = JWT_membuffer_alloc(0x22, CustomCloud_CMD_UPDATA_SOFT_TCP_RET, DataBuf, 15);
  //print_hex_data(data,23);
  return data;
}

int JWT_rx_0xA3_TCP_Update_Data(uint8 *data)
{
  uint16 ptr = 0;
  co_dword dword;
  int i;

  ptr +=6;

  dword.b[1] = data[4];
  dword.b[0] = data[5];

  //报文长度与需求不一致	0x10A
  if(dword.v != 266)
  {
	dbg_printf(1,"bags_lenth != 266   bags_lenth : %d  ",dword.v);
	return 0;
  }
  else
  {
  	  TCP_Info.TCP_File_bags_serial_number = 0;
	  dword.v = 0;

	  dword.b[1] = data[ptr];
	  ptr ++;
	  dword.b[0] = data[ptr];
	  ptr ++;

	  TCP_Info.TCP_File_bags_serial_number = dword.v;
	  //dbg_printf(1,"num: %d ", dword.v);

	  if(TCP_Info.TCP_File_bags_serial_number != (g_Sys_Config_Param.TCP_ud_stop_serial_number + 1))
		{
			dbg_printf(1,"rcv_num != Sys_num+1: %d  ", g_Sys_Config_Param.TCP_ud_stop_serial_number +1 );
			dbg_printf(1,"rcv_num: %d   Sys_num: %d", TCP_Info.TCP_File_bags_serial_number,g_Sys_Config_Param.TCP_ud_stop_serial_number);
			return 2;
		}
	  else
	  	{
			for(i=0;i<LEN_READ;i++,ptr++)
			{
				TCP_Info.TCP_File_data[i]=data[ptr];
			}
	  	}
  }


  return 1;

}

uint8* JWT_tx_0x23_TCP_Update_Data_response(uint8 *data , uint8 result)
{
//result    0-成功     1-失败    2-序列号错误
  uint8 i;
  uint8 DataBuf[20];
  co_dword dword;
  int num[2]={0};

  if(result == 0)
  {
   DataBuf[0] = 0x00;
  }
  else if(result == 1)
  {
   DataBuf[0] = 0x01;
  }
  else
  {
	DataBuf[0] = 0x01;
  }


   if(result == 0)
  {
    dword.v = TCP_Info.TCP_File_bags_serial_number;
    DataBuf[1] = dword.b[1];
    DataBuf[2] = dword.b[0];
  }
  else
  {
	dword.v =  g_Sys_Config_Param.TCP_ud_stop_serial_number;
  	DataBuf[1] = dword.b[1];
  	DataBuf[2] = dword.b[0];
  }


  DataBuf[3] = ((g_Sys_Config_Param.Serial[0] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[1] & 0x0F);
  DataBuf[4] = ((g_Sys_Config_Param.Serial[2] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[3] & 0x0F);
  DataBuf[5] = ((g_Sys_Config_Param.Serial[4] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[5] & 0x0F);
  DataBuf[6] = ((g_Sys_Config_Param.Serial[6] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[7] & 0x0F);
  DataBuf[7] = ((g_Sys_Config_Param.Serial[8] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[9] & 0x0F);
  DataBuf[8] = ((g_Sys_Config_Param.Serial[10] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[11] & 0x0F);
  DataBuf[9] = ((g_Sys_Config_Param.Serial[12] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[13] & 0x0F);
  DataBuf[10] = ((g_Sys_Config_Param.Serial[14] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[15] & 0x0F);

  data = JWT_membuffer_alloc(0x23, CustomCloud_CMD_SEND_UPDATA_FILE_RET, DataBuf, 11);
  //print_hex_data(data,19);
  return data;
}


uint8* JWT_tx_0x24_TCP_Update_Data_result_response(uint8 *data, uint8 result)
{
//result 0-升级失败 	   1-成功 	2-下载失败
  uint8 i;
  uint8 DataBuf[20];

  if(result == 1)
	{
	  DataBuf[0] = 0x00;
	  DataBuf[1] = 0x00;
	}
  else
	{
	  DataBuf[0] = 0x01;
	  DataBuf[1] = 0x02;//文件校验失败
	}



  //用系统的软件版本号
  DataBuf[2] = TCP_Info.SoftVer[1]- '0';
  DataBuf[3] = ((TCP_Info.SoftVer[3] -'0')*10) + (TCP_Info.SoftVer[4] -'0');


  DataBuf[4] = ((g_Sys_Config_Param.Serial[0] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[1] & 0x0F);
  DataBuf[5] = ((g_Sys_Config_Param.Serial[2] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[3] & 0x0F);
  DataBuf[6] = ((g_Sys_Config_Param.Serial[4] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[5] & 0x0F);
  DataBuf[7] = ((g_Sys_Config_Param.Serial[6] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[7] & 0x0F);
  DataBuf[8] = ((g_Sys_Config_Param.Serial[8] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[9] & 0x0F);
  DataBuf[9] = ((g_Sys_Config_Param.Serial[10] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[11] & 0x0F);
  DataBuf[10] = ((g_Sys_Config_Param.Serial[12] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[13] & 0x0F);
  DataBuf[11] = ((g_Sys_Config_Param.Serial[14] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[15] & 0x0F);

  data = JWT_membuffer_alloc(0x24, CustomCloud_CMD_SEND_UPDATA_RESULT_TCP, DataBuf, 12);
  //print_hex_data(data,20);
  return data;
}



void JWT_rx_0xA5_Remote_Update(uint8 *data)
{
  uint8 i;
  uint8 Ch;
  uint8 ptr = 0;
  co_dword dword;

  ptr +=7;
  memcpy(JWT_UpdateInfo.Update_url, data+ptr, 64);
  ptr +=64;
  dbg_printf(1," Update_url : %s", &JWT_UpdateInfo.Update_url);

  dword.w[1] = data[ptr];
  ptr ++;
  dword.w[0] = data[ptr];
  ptr ++;
  JWT_UpdateInfo.Update_port = dword.v;

  memcpy(JWT_UpdateInfo.Update_Usr, data+ptr, 20);
  ptr +=20;
  dbg_printf(1," Update_Usr :  %s", &JWT_UpdateInfo.Update_Usr);

  memcpy(JWT_UpdateInfo.Update_Pass, data+ptr, 20);
  ptr +=20;
  dbg_printf(1," Update_Pass : %s", &JWT_UpdateInfo.Update_Pass);

  memcpy(JWT_UpdateInfo.Update_File, data+ptr, 32);
  ptr +=32;
  dbg_printf(1," Update_File : %s", &JWT_UpdateInfo.Update_File);

  memcpy(JWT_UpdateInfo.SoftVer, SoftVer, 16);
  JWT_UpdateInfo.SoftVer[1] = (data[ptr]+'0');
  ptr ++;

  JWT_UpdateInfo.SoftVer[3] = ((data[ptr] >> 4)+'0');
  JWT_UpdateInfo.SoftVer[4] = ((data[ptr] & 0x0F)+'0');

  ptr ++;
  dbg_printf(1," SoftVer : %s", &JWT_UpdateInfo.SoftVer);
  JWT_UpdateInfo.UpdateMode = FTP_MODE;

}

uint8* JWT_tx_0x25_Remote_Update_response(uint8 *data ,uint8 result)
{
//result 0-程序类型正确        1-程序类型错误
  uint8 i;
  uint8 DataBuf[20];

  if(result == 1)
	{
	  DataBuf[0] = 0x02;
	}
	else
	{
	  if(g_Charge_Ctrl_Manager[0].CurState == STATE_IDLE)
		{
			DataBuf[0] = 0x01;
		}
		else
		{
			DataBuf[0] = 0x00;
		}
	}



  DataBuf[1] = 0x00;

  DataBuf[2] = JWT_UpdateInfo.SoftVer[1] - '0';
  DataBuf[3] = ((JWT_UpdateInfo.SoftVer[3] -'0')*16) + (JWT_UpdateInfo.SoftVer[4] -'0');


  DataBuf[4] = ((g_Sys_Config_Param.Serial[0] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[1] & 0x0F);
  DataBuf[5] = ((g_Sys_Config_Param.Serial[2] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[3] & 0x0F);
  DataBuf[6] = ((g_Sys_Config_Param.Serial[4] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[5] & 0x0F);
  DataBuf[7] = ((g_Sys_Config_Param.Serial[6] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[7] & 0x0F);
  DataBuf[8] = ((g_Sys_Config_Param.Serial[8] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[9] & 0x0F);
  DataBuf[9] = ((g_Sys_Config_Param.Serial[10] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[11] & 0x0F);
  DataBuf[10] = ((g_Sys_Config_Param.Serial[12] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[13] & 0x0F);
  DataBuf[11] = ((g_Sys_Config_Param.Serial[14] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[15] & 0x0F);

  data = JWT_membuffer_alloc(0x25, CustomCloud_CMD_UPDATA_SOFT_FTP_RET, DataBuf, 12);
  //print_hex_data(data,20);
  return data;
}

uint8* JWT_tx_0x26_Remote_Update_result(uint8 *data ,uint8 result)
{
//result 0-升级失败        1-成功     2-下载失败
  uint8 i;
  uint8 DataBuf[20];

  if(result == 1)
	{
	  DataBuf[0] = 0x00;
	  DataBuf[1] = 0x00;
	}
  else if(result == 0)
	{
	  DataBuf[0] = 0x01;
	  DataBuf[1] = 0x02;
	}
  else if(result == 2)
	{
	  DataBuf[0] = 0x01;
	  DataBuf[1] = 0x01;
	}


  DataBuf[2] = JWT_UpdateInfo.SoftVer[1] - '0';
  DataBuf[3] = ((JWT_UpdateInfo.SoftVer[3] -'0')*16) + (JWT_UpdateInfo.SoftVer[4] -'0');

  DataBuf[4] = ((g_Sys_Config_Param.Serial[0] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[1] & 0x0F);
  DataBuf[5] = ((g_Sys_Config_Param.Serial[2] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[3] & 0x0F);
  DataBuf[6] = ((g_Sys_Config_Param.Serial[4] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[5] & 0x0F);
  DataBuf[7] = ((g_Sys_Config_Param.Serial[6] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[7] & 0x0F);
  DataBuf[8] = ((g_Sys_Config_Param.Serial[8] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[9] & 0x0F);
  DataBuf[9] = ((g_Sys_Config_Param.Serial[10] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[11] & 0x0F);
  DataBuf[10] = ((g_Sys_Config_Param.Serial[12] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[13] & 0x0F);
  DataBuf[11] = ((g_Sys_Config_Param.Serial[14] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[15] & 0x0F);

  data = JWT_membuffer_alloc(0x26, CustomCloud_CMD_SEND_UPDATA_RESULT_FTP, DataBuf, 12);
  //print_hex_data(data,20);
  return data;
}

uint8 JWT_rx_0xA5_HTTP_Update(uint8 *data)
{
  uint8 i;
  uint8 Ch;
  uint8 ptr = 6;
  uint8 ret = 0;
  co_dword dword;
  memset(&JWT_UpdateInfo,0,sizeof(&JWT_UpdateInfo));


  if(g_Charge_Ctrl_Manager[0].CurState == STATE_CHARGEING)
    return 0;

  ret = data[ptr++];
  if(ret == 3)
  {
    memcpy(JWT_UpdateInfo.Update_url, data+ptr, 64);
    ptr +=64;
    dbg_printf(1," Update_url : %s", &JWT_UpdateInfo.Update_url);

    JWT_UpdateInfo.SoftVer[1] = (data[ptr]+'0');
    ptr ++;
    JWT_UpdateInfo.SoftVer[3] = ((data[ptr] >> 4)+'0');
    JWT_UpdateInfo.SoftVer[4] = ((data[ptr] & 0x0F)+'0');
    ptr ++;
    dbg_printf(1," SoftVer : %s", &JWT_UpdateInfo.SoftVer);
  }

  return 2;

}

uint8* JWT_tx_0xA6_HTTP_Update_result(uint8 *data ,uint8 result)
{
//result 0-占用        2-确认成功
  uint8 i;
  uint8 DataBuf[20];

  DataBuf[0] = result;
  DataBuf[1] = 3;

  DataBuf[2] = JWT_UpdateInfo.SoftVer[1] - '0';
  DataBuf[3] = ((JWT_UpdateInfo.SoftVer[3] -'0')*16) + (JWT_UpdateInfo.SoftVer[4] -'0');

  DataBuf[4] = ((g_Sys_Config_Param.Serial[0] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[1] & 0x0F);
  DataBuf[5] = ((g_Sys_Config_Param.Serial[2] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[3] & 0x0F);
  DataBuf[6] = ((g_Sys_Config_Param.Serial[4] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[5] & 0x0F);
  DataBuf[7] = ((g_Sys_Config_Param.Serial[6] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[7] & 0x0F);
  DataBuf[8] = ((g_Sys_Config_Param.Serial[8] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[9] & 0x0F);
  DataBuf[9] = ((g_Sys_Config_Param.Serial[10] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[11] & 0x0F);
  DataBuf[10] = ((g_Sys_Config_Param.Serial[12] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[13] & 0x0F);
  DataBuf[11] = ((g_Sys_Config_Param.Serial[14] & 0x0F) << 4) | (g_Sys_Config_Param.Serial[15] & 0x0F);

  data = JWT_membuffer_alloc(0x26, 0x26, DataBuf, 12);
  print_hex_data(data,20);
  return data;
}



void yiyuan_restart(void)
{
  uint8_t SendBuf[20];
  uint16_t index = 0;

  SendBuf[index++] = 0x7B;
  SendBuf[index++] = 0xF3;
  SendBuf[index++] = 0x00;
  SendBuf[index++] = 0x10;
  SendBuf[index++] = 0x43;
  SendBuf[index++] = 0x46;
  SendBuf[index++] = 0x47;
  SendBuf[index++] = 0x3A;
  SendBuf[index++] = 0x52;
  SendBuf[index++] = 0x65;
  SendBuf[index++] = 0x73;
  SendBuf[index++] = 0x74;
  SendBuf[index++] = 0x61;
  SendBuf[index++] = 0x72;
  SendBuf[index++] = 0x74;
  SendBuf[index++] = 0x7B;

  Drv_Uart_Write(net_uart_port, (const uint8 *)SendBuf, index, 0);
}

void MT_4G_module_tx_0x2B_0x2B_0x2B(void)
{
	uint8_t  SendBuf[13];
  uint16_t index = 0;

  SendBuf[index++] = '+';
  SendBuf[index++] = '+';
  SendBuf[index++] = '+';
	SendBuf[index++] = 0x0D;
	SendBuf[index++] = 0x0A;
  Drv_Uart_Write(net_uart_port, (const uint8 *)SendBuf, index, 0);
}

void MT_4G_module_tx_a(void)
{
	uint8_t  SendBuf[1];
  uint16_t index = 0;

  SendBuf[index++] = 'a';
  Drv_Uart_Write(net_uart_port, (const uint8 *)SendBuf, index, 0);
}

void MT_4G_module_tx_AT_Z_reset(void)
{
	uint8_t  SendBuf[6];
  uint16_t index = 0;

  SendBuf[index++] = 'A';
  SendBuf[index++] = 'T';
  SendBuf[index++] = '+';
	SendBuf[index++] = 'Z';

	SendBuf[index++] = 0x0D;
	SendBuf[index++] = 0x0A;
  Drv_Uart_Write(net_uart_port, (const uint8 *)SendBuf, index, 0);
}

void MT_4G_module_setting_IP_port_NO(void)
{
	uint8_t  SendBuf[100];
  uint16_t index = 0;

  SendBuf[index++] = 'A';
  SendBuf[index++] = 'T';
  SendBuf[index++] = '+';
	SendBuf[index++] = 'S';
  SendBuf[index++] = 'O';
	SendBuf[index++] = 'C';
	SendBuf[index++] = 'K';
  SendBuf[index++] = 'A';
  SendBuf[index++] = '=';
	SendBuf[index++] = 'T';
  SendBuf[index++] = 'C';
	SendBuf[index++] = 'P';
	SendBuf[index++] = ',';
  SendBuf[index++] = '1';
  SendBuf[index++] = '2';
	SendBuf[index++] = '0';
  SendBuf[index++] = '.';
	SendBuf[index++] = '7';
	SendBuf[index++] = '9';
  SendBuf[index++] = '.';
  SendBuf[index++] = '6';
	SendBuf[index++] = '.';
  SendBuf[index++] = '2';
	SendBuf[index++] = '3';
	SendBuf[index++] = '3';
	SendBuf[index++] = ',';

  SendBuf[index++] = Get_Server_Port() / 1000 + 0x30 ;
	SendBuf[index++] = Get_Server_Port() / 100 % 10 + 0x30;
  SendBuf[index++] = Get_Server_Port() % 100 / 10 + 0x30;
	SendBuf[index++] = Get_Server_Port() % 10 + 0x30;

	SendBuf[index++] = 0x0D;
	SendBuf[index++] = 0x0A;
  Drv_Uart_Write(net_uart_port, (const uint8 *)SendBuf, index, 0);
}

void Ctrl_card_charge(uint8 Ch, uint8 control, uint8 *card_NO)
{
  MSG_Def  Msg;
  uint8    card[16];

  if (control == JWT_card_start)
  {
    memcpy(card, card_NO, 16);
    g_RecordData.cardNO[0] = ((card[0] & 0x0F) << 8) | (card[1] & 0x0F);
    g_RecordData.cardNO[1] = ((card[2] & 0x0F) << 8) | (card[3] & 0x0F);
    g_RecordData.cardNO[2] = ((card[4] & 0x0F) << 8) | (card[5] & 0x0F);
    g_RecordData.cardNO[3] = ((card[6] & 0x0F) << 8) | (card[7] & 0x0F);
    g_RecordData.cardNO[4] = ((card[8] & 0x0F) << 8) | (card[9] & 0x0F);
    g_RecordData.cardNO[5] = ((card[10] & 0x0F) << 8) | (card[11] & 0x0F);
    g_RecordData.cardNO[6] = ((card[12] & 0x0F) << 8) | (card[13] & 0x0F);
    g_RecordData.cardNO[7] = ((card[14] & 0x0F) << 8) | (card[15] & 0x0F);
  }
  Msg.Ch =Ch;
  Msg.type = control;
  MsgQueue_Send(MSG_QUEUE_TASK_CLOUD_COMM, Msg);
}
uint16 Change_fault_code(uint16 warning_code)
{
  uint16 ret = 0;
  if(warning_code == 1)//急停
  {
    ret = 0x2003;
  }
  else if(warning_code == 7)//CP接地
  {
    ret = 0x1014;
  }
  else if(warning_code == 9)//过压
  {
    ret = 0x3104;
  }
  else if(warning_code == 10)//欠压
  {
    ret = 0x3105;
  }
  else if(warning_code == 11)//过流
  {
    ret = 0x3107;
  }
  else if(warning_code == 21)//电表故障
  {
    ret = 0x1004;
  }
  else if(warning_code == 22)//读卡器故障
  {
    ret = 0x1002;
  }
  else if(warning_code == 37)//漏电流故障
  {
    ret = 0x2006;
  }

  dbg_printf(1,"warning_code=%d",ret);

  return ret;
}
void Ctrl_send_warning_message(uint8 Ch, uint16 warning_code, uint8 warning_status, uint32 warning_value)
{
  MSG_Def  Msg;
  co_dword dword;

  Msg.Ch =Ch;
  Msg.type = JWT_warning;

  //dword.w[0]  = warning_code;

  dword.w[0]  = Change_fault_code(warning_code);
  Msg.data[0] = dword.b[1];
  Msg.data[1] = dword.b[0];

  Msg.data[2] = warning_status;

  dword.v = warning_value;
  Msg.data[3] = dword.b[3];
  Msg.data[4] = dword.b[2];
  Msg.data[5] = dword.b[1];
  Msg.data[6] = dword.b[0];

  MsgQueue_Send(MSG_QUEUE_TASK_CLOUD_COMM, Msg);
}


uint8 Ctrl_send_charger_record(uint8 Ch)
{
	MSG_Def  Msg;

  Msg.Ch   = Ch;
	Msg.type = JWT_charge_record;
	MsgQueue_Send(MSG_QUEUE_TASK_CLOUD_COMM, Msg);
    return 1;
}

uint8 net_uart_offline(void)
{
  uint8  length;
  uint8* data;
  uint8  RcvData[200] = {0};
  data = JWT_tx_0x01_login(data);
  Drv_Uart_Write(net_uart_port, data, 37, 0);

  length = Drv_Uart_Read(net_uart_port , RcvData, sizeof(RcvData), 100);
  if (length > 0)
  {
    if (JWT_rx_handler(RcvData) == 0x81)
    {
      return 1;
    }
  }

  return 0;
}

uint8 net_uart_online(uint8 Ch, uint8 MsgFlag, MSG_Def *Msg, uint8 state)
{
  uint8*   data;
  uint8    length;
  uint8    MsgType = Msg->type;
  uint8    RcvData[200] = {0};
  co_dword dword;
  if (state == 1)
  {
    //data = JWT_tx_0x0C_heart_beat(Ch, data);
    //Drv_Uart_Write(net_uart_port, data, 18, 100);

	//osDelay(1000);
    data = JWT_tx_0x1F_gun_status(Ch, data);
    Drv_Uart_Write(net_uart_port, data, 18, 100);

    //data = JWT_tx_0x1C_QR_request(Ch, data);
    //Drv_Uart_Write(net_uart_port, data, 17, 100);

    state = 2;
  }
  else
  {
		data = JWT_tx_0x0C_heart_beat(Ch, data);
    Drv_Uart_Write(net_uart_port, data, 18, 100);
		osDelay(2000);

  if (g_Charge_Ctrl_Manager[Ch].CurState == STATE_CHARGEING)
  {
    memcpy(&g_RecordData.endStartTime[0], &g_JWT_info_t.heart_beat_time[0], 6);
  }
  else if ((g_RecordData.payMoney <= 5) && (g_RecordData.payMoney > 0))
  {
    g_RecordData.payMoney++;
    data = JWT_tx_0x08_charge_record(Ch, data);
    Drv_Uart_Write(net_uart_port, data, 125, 0);
  }

  if (MsgFlag)
  {
    switch (MsgType)
    {
      case JWT_card_start:
        {
          if (g_JWT_info_t.card_start_rx_flag != 1)
          {
            data = JWT_tx_0x04_card_charge_start(Ch, data);
            Drv_Uart_Write(net_uart_port, data, 38, 0);
          }
        }
        break;

      case JWT_card_end:
        {
          if (g_JWT_info_t.card_stop_rx_flag != 1)
          {
            data = JWT_tx_0x05_card_charge_stop(Ch, data);
            Drv_Uart_Write(net_uart_port, data, 33, 100);
          }
        }
        break;

      case JWT_APP_start_success:
        {
          data = JWT_tx_0x86_app_charge_start_response(Ch, 0, 0, data);
          Drv_Uart_Write(net_uart_port, data, 25, 100);
        }
        break;

      case JWT_APP_start_fail:
        {
          data = JWT_tx_0x86_app_charge_start_response(Ch, 2, 0, data);
          Drv_Uart_Write(net_uart_port, data, 25, 100);
        }
        break;

      case JWT_charge_progress:
        {
          if (g_Charge_Ctrl_Manager[Ch].CurState == STATE_CHARGEING)
          {
            data = JWT_tx_0x09_charge_progress(Ch, data);
            Drv_Uart_Write(net_uart_port, data, 120, 200);

            osDelay(500);
            data = JWT_tx_0x0A_charge_progress_data(Ch, data);
            Drv_Uart_Write(net_uart_port, data, 148, 200);
          }
        }
        break;

      case JWT_charge_progress_data:
        {

        }
        break;

      case JWT_warning:
        {
          uint16 warning_code;

          dword.b[1] = Msg->data[0];
          dword.b[0] = Msg->data[1];
          warning_code = dword.w[0];

          dword.b[3] = Msg->data[3];
          dword.b[2] = Msg->data[4];
          dword.b[1] = Msg->data[5];
          dword.b[0] = Msg->data[6];
          data = JWT_tx_0x0D_charger_warning(Ch, warning_code, Msg->data[2], dword.v, data);
          Drv_Uart_Write(net_uart_port, data, 30, 0);
        }
        break;

      case JWT_QR_code:
        {
          data = JWT_tx_0x1C_QR_request(Ch, data);
          Drv_Uart_Write(net_uart_port, data, 17, 0);
        }
        break;

      case JWT_gun_status_change:
        {
          data = JWT_tx_0x1F_gun_status(Ch, data);
          Drv_Uart_Write(net_uart_port, data, 18, 0);
        }
        break;

      default:
        break;
      }
    }
  }
    length = Drv_Uart_Read(net_uart_port , RcvData, sizeof(RcvData), 200);
    if (length > 0)
    {
      JWT_rx_handler(RcvData);
      #if 0
      switch (JWT_rx_handler(RcvData))
      {
        case 0x06:
          {
            data = JWT_tx_0x86_app_charge_start_response(Ch, 0, 0, data);
            Drv_Uart_Write(net_uart_port, (const uint8 *)data, sizeof(data), 0);
          }
          break;

        case 0x07:
          {
            data = JWT_tx_0x87_app_charge_stop_response(Ch, 0, 0, data);
            Drv_Uart_Write(net_uart_port, (const uint8 *)data, sizeof(data), 0);
          }
          break;

        case 0x0B:
          {
            data = JWT_tx_0x8B_billing_rule_set_response(data);
            Drv_Uart_Write(net_uart_port, (const uint8 *)data, sizeof(data), 0);
          }
          break;

        case 0x0E:
          {
            data = JWT_tx_setting_response(0x8E, 0, data);
            Drv_Uart_Write(net_uart_port, (const uint8 *)data, sizeof(data), 0);
          }
          break;

        default:
          break;
      }
      #endif
    }
    else
    {
      if (state++ > 3)
      {
        LED_Function(NET_LED,LED_Off);
        state = 0;
      }
    }
  return state;
}

void uart_net_daemon(void)
{
  uint8 state        = 0;
  uint8 log_in_times = 0;


	//USART_Init(COM2,STOP_1,P_NONE, 19200);

	USART_Init(COM4,STOP_1,P_NONE, 115200);
  while(1)
  {
    uint8    Ch      = 0;
    uint8    MsgFlag = 0;
    MSG_Def  Msg;

    MsgFlag = MsgQueue_Get(MSG_QUEUE_TASK_CLOUD_COMM, &Msg);
    if (MsgFlag)
    {
      Ch = Msg.Ch;
    }

    if (state == 0)
    {
      if (log_in_times <= 4)
      {
        state = net_uart_offline();
        log_in_times++;
        osDelay(10000);
      }
      else
      {
        log_in_times = 0;
        yiyuan_restart();
        osDelay(30000);
      }
    }
    else
    {
      LED_Function(NET_LED, LED_On);
      state = net_uart_online(Ch, MsgFlag, &Msg, state);

			osDelay(10000);
		}
  }
}




static int Cloud_Connect_Server_by_LAN(void)
{
    int fd = 0;

    char ip[40] ="192.168.1.66";
    int port = 8000;
    int connect_flag = 0;
    int ms = HAL_GetTick();

    do
    {
        osDelay(20);

        fd = eth_create_socket(0);

        if(fd < 0)
        {
            close(fd);

        }
        else
        {
            memset(ip,0,sizeof(ip));
            Get_Server_Ip((U8*)ip);   //
            port = Get_Server_Port();//
            connect_flag = eth_connect_server(fd,ip,port);//

            if(connect_flag == 1)
            {
                return fd;
            }
            else
            {
                close(fd);
                osDelay(2000);
            }
        }

     }while(HAL_GetTick() - ms < 5000);

     return -1;
}



static void Relase_connect(Cloud_Comm_Info *comm)
{
    if(comm->connectType == LAN)
    {
        close(comm->Port);
    }

}
/*

	鍋ョ綉绉戞妧 -- JWT

					*/

static uint8_t AT_Cmd_Parse(char *send,char *expect,char *ret,uint32_t timeout)
{

    uint32_t tick = xTaskGetTickCount();
    uint8 rcvbuf[128] = {0};
    int remain = sizeof(rcvbuf);
    uint32_t len = 0;
    uint32_t rcvLen = 0;
    char *str = NULL;

    memset(rcvbuf,0,sizeof(rcvbuf));

    len = Drv_Uart_Read(net_uart_port,(unsigned char *) rcvbuf,remain,200);//

    Drv_Uart_Write(net_uart_port,(const unsigned char *) send,strlen(send),100);
    osDelay(200);

    memset(rcvbuf,0,sizeof(rcvbuf));

    dbg_printf(1,"send %s",send);

    do
    {
        len = Drv_Uart_Read(net_uart_port,(unsigned char *)rcvbuf + rcvLen,remain,500);

        if(len > 0)
        {
            dbg_printf(1,"cmd rcv %s",rcvbuf);
            remain -= len;
            rcvLen += len;
            str = strstr((char *)rcvbuf,expect);
            if(str)
            {
                if(ret != NULL)
                {
                   strcpy(ret,str);
                }
                return TRUE;
            }
        }
        else
        {
            osDelay(20);
        }
        if(remain < 0)
        {
            break;
        }
    }while(xTaskGetTickCount() - tick < timeout);


    return FALSE;
}

uint8 GprsVersion4G(void)
{

    char cmd[64] = {0};

    char ip[54] = {0};

    uint32 port = 0;
    int ReConnectCount = 0;

    Get_Server_Ip((U8*)ip);

    port = Get_Server_Port();//

	while(ReConnectCount < 3)
	{
        dbg_printf(1,"+++");

        if(AT_Cmd_Parse("+++\r\n", "a",NULL, 500)== FALSE)
        {
	        ReConnectCount++;
	        osDelay(1000);
	        continue;
        }

        if(AT_Cmd_Parse("a\r\n", "+ok",NULL, 500)== FALSE)
        {
            ReConnectCount++;
            osDelay(1000);
            continue;
        }

        //
        osDelay(500);
        sprintf(cmd,"AT+SOCKA=TCP,%s,%d\r\n",ip,port);
        dbg_printf(1,"set dtu ip %s",ip);
        if(AT_Cmd_Parse(cmd, "OK",NULL,1000)== FALSE)
        {
            ReConnectCount++;
            continue;
        }

        osDelay(500);
        memset(cmd,0,sizeof(cmd));

        dbg_printf(1,"REBOOT 4G");
        if(AT_Cmd_Parse("AT+Z\r\n","OK",NULL, 1000)== FALSE)
        {
            ReConnectCount++;
            continue;
        }
        osDelay(30000);
        dbg_printf(1,"SET 4g OK");
        return TRUE;


	}

//	BSP_GPIO_OutPut(POWER_4G,OUT_HIGHE);
//    osDelay(200);
//    BSP_GPIO_OutPut(POWER_4G,OUT_LOW);
//    dbg_printf(1,"RSET 4g");
//    osDelay(1000);

	return FALSE;
}

//域格4g模块      old
//uint8 YvGe_4G(void)
//{
//
//  char cmd[64] = {0};
//  char cmd_2[64] = {0};
//  char ip[54] = {0};
//
//  uint32 port = 0;
//  int ReConnectCount = 0;
//
//  Get_Server_Ip((U8*)ip);
//
//  port = Get_Server_Port();
//
//  dbg_printf(1,"in YvGe_4G!");
//  while(ReConnectCount < 3)
//  {
//     //退出透传模式
//		if(AT_Cmd_Parse("+++", "OK",NULL, 1200)== FALSE)
//    {
//
//      //ReConnectCount++;
//      osDelay(1000);
//      //continue;
//    }
//		//断开已有连接
//		if(AT_Cmd_Parse("AT+QIPCLOSE=1\r", "OK",NULL, 500)== FALSE)
//    {
//	    osDelay(1000);
//
//    }
//		if(AT_Cmd_Parse("AT+CPIN?\r", "OK",NULL, 500)== FALSE)
//    {
//    	osDelay(1000);
//      ReConnectCount++;
//	    dbg_printf(1,"AT+CPIN? not ok!");
//      continue;
//    }
//		if(AT_Cmd_Parse("AT+CSQ\r", "OK",NULL, 500)== FALSE)
//    {
//    	osDelay(1000);
//      ReConnectCount++;
//	    dbg_printf(0,"AT+CSQ not ok!");
//      continue;
//    }
//		if(AT_Cmd_Parse("AT^SYSINFO\r", "OK",NULL, 500)== FALSE)
//    {
//    	osDelay(1000);
//      ReConnectCount++;
//	    dbg_printf(0,"AT^SYSINFO not ok!");
//      continue;
//    }
//		//激活一次过后，后面都会回error
//		if(AT_Cmd_Parse("AT+QIPCSGP=1,1,\"CMNET\"\r", "OK",NULL, 500)== FALSE)
//      {
//      	osDelay(1000);
//      }
//		//激活一次过后，后面都会回error
//		if(AT_Cmd_Parse("AT+QIPACT=1\r", "OK",NULL, 500)== FALSE)
//      {
//        //ReConnectCount++;
//        osDelay(1000);
//		    //dbg_printf(0,"AT+QIPACT=1 not ok！");
//      }
//
//		osDelay(500);
//		memset(cmd,0,sizeof(cmd));
//		memset(cmd_2,0,sizeof(cmd_2));
//
//		sprintf(cmd_2,"%s","\"");
//		sprintf(cmd_2,"%s%s",cmd_2,ip);
//		sprintf(cmd_2,"%s%s",cmd_2,"\"");
//        sprintf(cmd,"AT+QIPOPEN=1,1,%s,%s,%d,0,2\r","\"TCP\"",cmd_2,port);
//		dbg_printf(0,"cmd =%s",cmd);
//    //test //if(AT_Cmd_Parse("AT+QIPOPEN=1,1,\"TCP\",\"120.79.6.233\",8015,0,2\r", "CONNECT",NULL, 500)== FALSE)
//		if(AT_Cmd_Parse(cmd, "CONNECT",NULL, 500)== FALSE)
//     {
//		   ReConnectCount++;
//		   osDelay(1000);
//		   dbg_printf(1,"TCP CONNECT not ok!");
//		   continue;
//    }
//		osDelay(15000);
//    dbg_printf(1,"SET YG 4g OK");
//    return TRUE;
//  }
//
//  return FALSE;
//}


//域格模块
static uint8 YvGe_4G(void)
{

  char cmd[64] = {0};
  char cmd_2[64] = {0};
  char ip[54] = {0};

  uint32 port = 0;
  int ReConnectCount = 0;

  Get_Server_Ip((U8*)ip);

  port = Get_Server_Port();

  dbg_printf(1,"in YvGe_4G!");
  while(ReConnectCount < 3)
  {
    if(AT_Cmd_Parse("+++", "OK",NULL, 1200)== FALSE)
    {

      //ReConnectCount++;
      osDelay(1000);
      //continue;
    }
    if(AT_Cmd_Parse("ATE0\r", "OK",NULL, 500)== FALSE)
    {
     osDelay(1000);
      ReConnectCount++;
     dbg_printf(1,"ATE0 not ok!");
      continue;
    }
    if(AT_Cmd_Parse("AT+QIPCSGP=1,1,"","","",1\r", "OK",NULL, 500)== FALSE)
    {
     osDelay(1000);
     ReConnectCount++;
    }
    if(AT_Cmd_Parse("AT+QIPDEACT=1\r", "OK",NULL, 500)== FALSE)
    {
      ReConnectCount++;
      osDelay(1000);
    dbg_printf(0,"AT+QIPDEACT=1 not ok￡?");
    }

    if(AT_Cmd_Parse("AT+QIPACT=1\r", "OK",NULL, 500)== FALSE)
    {
      osDelay(1000);
    }

    osDelay(500);
    memset(cmd,0,sizeof(cmd));
    memset(cmd_2,0,sizeof(cmd_2));

    sprintf(cmd_2,"%s","\"");
    sprintf(cmd_2,"%s%s",cmd_2,ip);
    sprintf(cmd_2,"%s%s",cmd_2,"\"");
    sprintf(cmd,"AT+QIPOPEN=1,1,%s,%s,%d,0,2\r","\"TCP\"",cmd_2,port);
    dbg_printf(0,"cmd =%s",cmd);
    //if(AT_Cmd_Parse("AT+QIPOPEN=1,1,\"TCP\",\"120.78.213.97\",8015,0,2\r", "CONNECT",NULL, 500)== FALSE)
  if(AT_Cmd_Parse(cmd, "CONNECT",NULL, 500)== FALSE)
     {
       ReConnectCount++;
       osDelay(1000);
       dbg_printf(1,"TCP CONNECT not ok!");
       continue;
    }
    osDelay(10000);
    dbg_printf(1,"SET YG 4g OK");
    return TRUE;
  }

  return FALSE;
}

//MEIGE 美格模块
static uint8 MeiGe_4G(void)
{

  char cmd[64] = {0};
  char cmd_2[64] = {0};
  char ip[54] = {0};

  uint32 port = 0;
  int ReConnectCount = 0;

  Get_Server_Ip((U8*)ip);

  port = Get_Server_Port();

  dbg_printf(1,"in MeiGe_4G!");
  while(ReConnectCount < 3)
  {
  if(AT_Cmd_Parse("+++", "OK",NULL, 1000)== FALSE)
    {

      //ReConnectCount++;
      osDelay(1000);
      //continue;
    }
  if(AT_Cmd_Parse("ATE0\r", "OK",NULL, 1500)== FALSE)
    {
      osDelay(1000);
      ReConnectCount++;
      dbg_printf(1,"ATE0 not ok!");
      continue;
    }
   if(AT_Cmd_Parse("AT+QICSGP=1,1,\"\",\"\",\"\",1\r", "OK",NULL, 1500)== FALSE)
    {
     osDelay(1000);
      ReConnectCount++;
     dbg_printf(1,"AT+QICSGP not ok!");
      continue;
    }
   if(AT_Cmd_Parse("AT+QIDEACT=1\r", "OK",NULL, 1500)== FALSE)
    {
      osDelay(1000);
      ReConnectCount++;
      dbg_printf(1,"AT+QIDEACT not ok!");
      continue;
    }
  if(AT_Cmd_Parse("AT+QIACT=1\r", "OK",NULL, 6000)== FALSE)
    {
      osDelay(1000);
    }
    osDelay(200);
    memset(cmd,0,sizeof(cmd));
    memset(cmd_2,0,sizeof(cmd_2));

    sprintf(cmd_2,"%s","\"");
    sprintf(cmd_2,"%s%s",cmd_2,ip);
    sprintf(cmd_2,"%s%s",cmd_2,"\"");
    sprintf(cmd,"AT+QIOPEN=1,1,%s,%s,%d,0,2\r","\"TCP\"",cmd_2,port);
    dbg_printf(0,"cmd =%s",cmd);
    //if(AT_Cmd_Parse("AT+QIOPEN=1,1,\"TCP\",\"zijieyun.com\",1038,0,2\r", "CONNECT",NULL, 500)== FALSE)
    //if(AT_Cmd_Parse(cmd, "CONNECT",NULL, 1000)== FALSE)
    if(AT_Cmd_Parse(cmd, "CONNECT",NULL, 1000)== FALSE)
    {
      ReConnectCount++;
      osDelay(1000);
      dbg_printf(1,"TCP CONNECT not ok!");
      continue;
    }
    osDelay(10000);
    dbg_printf(1,"SET MeuGe 4g OK");
    return TRUE;
  }
  return FALSE;
}


//YIYUAN 移远模块
static uint8 YiYuan_4G(void)
{

  char cmd[64] = {0};
  char cmd_2[64] = {0};
  char ip[54] = {0};

  uint32 port = 0;
  int ReConnectCount = 0;

  Get_Server_Ip((U8*)ip);

  port = Get_Server_Port();

  dbg_printf(1,"in Yiyuan_4G!");
  while(ReConnectCount < 3)
  {
  if(AT_Cmd_Parse("+++", "OK",NULL, 1000)== FALSE)
    {

      //ReConnectCount++;
      osDelay(1000);
      //continue;
    }
  if(AT_Cmd_Parse("ATE0\r", "OK",NULL, 500)== FALSE)
    {
      osDelay(1000);
      ReConnectCount++;
      dbg_printf(1,"ATE0 not ok!");
      continue;
    }
   if(AT_Cmd_Parse("AT+QICSGP=1,1,\"\",\"\",\"\",1\r", "OK",NULL, 500)== FALSE)
    {
     osDelay(1000);
      ReConnectCount++;
     dbg_printf(1,"AT+QICSGP not ok!");
      continue;
    }
   if(AT_Cmd_Parse("AT+QIDEACT=1\r", "OK",NULL, 500)== FALSE)
    {
      osDelay(1000);
      ReConnectCount++;
      dbg_printf(1,"AT+QIDEACT not ok!");
      continue;
    }
  if(AT_Cmd_Parse("AT+QIACT=1\r", "OK",NULL, 1000)== FALSE)
    {
      osDelay(1000);
    }
    osDelay(200);
    memset(cmd,0,sizeof(cmd));
    memset(cmd_2,0,sizeof(cmd_2));

    sprintf(cmd_2,"%s","\"");
    sprintf(cmd_2,"%s%s",cmd_2,ip);
    sprintf(cmd_2,"%s%s",cmd_2,"\"");
    sprintf(cmd,"AT+QIOPEN=1,1,%s,%s,%d,0,2\r","\"TCP\"",cmd_2,port);
    dbg_printf(1,"cmd =%s",cmd);
    //if(AT_Cmd_Parse("AT+QIOPEN=1,1,\"TCP\",\"zijieyun.com\",1038,0,2\r", "CONNECT",NULL, 500)== FALSE)
    if(AT_Cmd_Parse(cmd, "CONNECT",NULL, 10000)== FALSE)
    {
      ReConnectCount++;
      osDelay(1000);
      dbg_printf(1,"TCP CONNECT not ok!");
      continue;
    }
    osDelay(10000);
    dbg_printf(1,"SET YiYuan 4g OK");
    return TRUE;
  }
  return FALSE;
}


static int  Cloud_Connect(Cloud_Comm_Info *comm)
{
    int fd = 0;
    int ret = 0;

    fd = Cloud_Connect_Server_by_LAN();

    if(fd >=0)
    {
       comm->Port = fd;

       comm->Read = (int (*)(uint8_t,uint8_t *,uint16_t,int))eth_read;
       comm->Send = (int (*)(uint8_t,uint8_t *,uint16_t,int))eth_write;
       comm->connectType = LAN;
       ret = 1;

       //2020.12.4
       if(first_conect_byLAN == 0)
          first_conect_byLAN = 1;
    }
    else if((fd  = GprsVersion4G()) > 0)
    {
       osDelay(1000);
       comm->Port = net_uart_port;

       comm->Read = Drv_Uart_Read;
       comm->Send = (int (*)(uint8_t,uint8_t *,uint16_t,int))Drv_Uart_Write;
       comm->connectType = DTU;
       ret = fd;
    }
    else if((fd  = YiYuan_4G()) > 0)
    {
      osDelay(1000);
      comm->Port = net_uart_port;

      comm->Read = Drv_Uart_Read;
      comm->Send = (int (*)(uint8_t,uint8_t *,uint16_t,int))Drv_Uart_Write;
      comm->connectType = DTU;
      ret = fd;
    }
    else
    {
//       comm->Port = net_uart_port_2G;
//
//       comm->Read = Drv_Uart_Read;
//       comm->Send = (int (*)(uint8_t,uint8_t *,uint16_t,int))Drv_Uart_Write;
//       comm->connectType = DTU;
//       ret = 1;
//       dbg_printf(1,"usr 2g connct");
       ret = 0;
    }

    return ret;
}

uint8* JWT_tx_0x08_charge_record_form_flash(uint8 ch, ChargeRecord st_Record )
{
  uint8 i;
  uint8 DataBuf[117];
  co_dword dword;
  struct tm *st_time;
  uint8 bcd_time[6] = {0};

  DataBuf[0] = ch + 1;

  for (i = 0; i < 8; i++)
  {
    DataBuf[1 + i] = st_Record.Reverse[i];
  }

  if(st_Record.ChargeType == BY_APP_START)
    DataBuf[9] = 2;
  else
    DataBuf[9] = 1;
  asc2hex(&DataBuf[10], st_Record.CardNo, 16);
  #if 0
  for (i = 0; i < 8; i++)
  {
    DataBuf[10 + i] = g_RecordData.cardNO[i];
  }
  #endif

  for (i = 0; i < 17; i++)
  {
    DataBuf[18 + i] = 0x01;
  }

  DataBuf[35] = 0;

  DataBuf[36] = st_Record.StopReason;
  st_time = gmtime((const time_t *)&st_Record.StartTime);

  bcd_time[0] = HexToBcd(st_time->tm_year - 100);
  bcd_time[1] = HexToBcd(st_time->tm_mon + 1);
  bcd_time[2] = HexToBcd(st_time->tm_mday);
  bcd_time[3] = HexToBcd(st_time->tm_hour);
  bcd_time[4] = HexToBcd(st_time->tm_min);
  bcd_time[5] = HexToBcd(st_time->tm_sec);

  for (i = 0; i < 6; i++)
  {
    DataBuf[37 + i] = bcd_time[i];
  }

  st_time = gmtime((const time_t *)&st_Record.EndTime);
  bcd_time[0] = HexToBcd(st_time->tm_year - 100);
  bcd_time[1] = HexToBcd(st_time->tm_mon + 1);
  bcd_time[2] = HexToBcd(st_time->tm_mday);
  bcd_time[3] = HexToBcd(st_time->tm_hour);
  bcd_time[4] = HexToBcd(st_time->tm_min);
  bcd_time[5] = HexToBcd(st_time->tm_sec);

  for (i = 0; i < 6; i++)
  {
    DataBuf[43 + i] = bcd_time[i];
  }

  dword.v = st_Record.StartEnerge*10;
  DataBuf[49] = dword.b[3];
  DataBuf[50] = dword.b[2];
  DataBuf[51] = dword.b[1];
  DataBuf[52] = dword.b[0];

  dword.v = st_Record.EndEnerge*10;
  DataBuf[53] = dword.b[3];
  DataBuf[54] = dword.b[2];
  DataBuf[55] = dword.b[1];
  DataBuf[56] = dword.b[0];

  dword.v = (st_Record.EndEnerge - st_Record.StartEnerge)*10;
  DataBuf[57] = dword.b[3];
  DataBuf[58] = dword.b[2];
  DataBuf[59] = dword.b[1];
  DataBuf[60] = dword.b[0];
  //
  g_JWT_info_t.sharp_E =0;
  g_JWT_info_t.peak_E = 0;
  g_JWT_info_t.flat_E = 0;
  g_JWT_info_t.valley_E = 0;
  g_JWT_info_t.sharp_money = 0;
  g_JWT_info_t.peak_money = 0;
  g_JWT_info_t.flat_money = 0;
  g_JWT_info_t.valley_money = 0;
  uint32_t fee_val[4] = {0};
  for(i = 0; i < 48;i++)
  {
      if(g_Sys_Config_Param.FeeTime[i] == 1)
      {
          g_JWT_info_t.sharp_E += st_Record.SectionEnerge[i];
          fee_val[0] = g_Sys_Config_Param.Price[i];
      }
      else if(g_Sys_Config_Param.FeeTime[i] == 2)
      {
          g_JWT_info_t.peak_E += st_Record.SectionEnerge[i];
          fee_val[1] = g_Sys_Config_Param.Price[i];
      }
      else if(g_Sys_Config_Param.FeeTime[i] == 3)
      {
          g_JWT_info_t.flat_E += st_Record.SectionEnerge[i];
          fee_val[2] = g_Sys_Config_Param.Price[i];
      }
      else
      {
          g_JWT_info_t.valley_E += st_Record.SectionEnerge[i];
          fee_val[3] = g_Sys_Config_Param.Price[i];
      }
  }
  g_JWT_info_t.sharp_money = g_JWT_info_t.sharp_E * fee_val[0]/10;
  g_JWT_info_t.peak_money = g_JWT_info_t.peak_E * fee_val[1]/10;
  g_JWT_info_t.flat_money = g_JWT_info_t.flat_E * fee_val[2]/10;
  g_JWT_info_t.valley_money = g_JWT_info_t.valley_E * fee_val[3]/10;

  g_JWT_info_t.sharp_E *= 10; //放大10倍 系统使用两位小数
  g_JWT_info_t.peak_E *= 10; //
  g_JWT_info_t.flat_E *= 10; //
  g_JWT_info_t.valley_E *= 10; //

  DataBuf[61] = g_JWT_info_t.sharp_E >> 24;
  DataBuf[62] = g_JWT_info_t.sharp_E >> 16;
  DataBuf[63] = g_JWT_info_t.sharp_E >> 8;
  DataBuf[64] = g_JWT_info_t.sharp_E;

  DataBuf[65] = g_JWT_info_t.peak_E >> 24;
  DataBuf[66] = g_JWT_info_t.peak_E >> 16;
  DataBuf[67] = g_JWT_info_t.peak_E >> 8;
  DataBuf[68] = g_JWT_info_t.peak_E;

  DataBuf[69] = g_JWT_info_t.flat_E >> 24;
  DataBuf[70] = g_JWT_info_t.flat_E >> 16;
  DataBuf[71] = g_JWT_info_t.flat_E >> 8;
  DataBuf[72] = g_JWT_info_t.flat_E;

  DataBuf[73] = g_JWT_info_t.valley_E >> 24;
  DataBuf[74] = g_JWT_info_t.valley_E >> 16;
  DataBuf[75] = g_JWT_info_t.valley_E >> 8;
  DataBuf[76] = g_JWT_info_t.valley_E;

  dword.v = st_Record.ChargeMoney*10;
  DataBuf[77] = dword.b[3];
  DataBuf[78] = dword.b[2];
  DataBuf[79] = dword.b[1];
  DataBuf[80] = dword.b[0];

  DataBuf[81] = g_JWT_info_t.sharp_money >> 24;
  DataBuf[82] = g_JWT_info_t.sharp_money >> 16;
  DataBuf[83] = g_JWT_info_t.sharp_money >> 8;
  DataBuf[84] = g_JWT_info_t.sharp_money;

  DataBuf[85] = g_JWT_info_t.peak_money >> 24;
  DataBuf[86] = g_JWT_info_t.peak_money >> 16;
  DataBuf[87] = g_JWT_info_t.peak_money >> 8;
  DataBuf[88] = g_JWT_info_t.peak_money;

  DataBuf[89] = g_JWT_info_t.flat_money >> 24;
  DataBuf[90] = g_JWT_info_t.flat_money >> 16;
  DataBuf[91] = g_JWT_info_t.flat_money >> 8;
  DataBuf[92] = g_JWT_info_t.flat_money;

  DataBuf[93] = g_JWT_info_t.valley_money >> 24;
  DataBuf[94] = g_JWT_info_t.valley_money >> 16;
  DataBuf[95] = g_JWT_info_t.valley_money >> 8;
  DataBuf[96] = g_JWT_info_t.valley_money;

  DataBuf[97] = 0x00;
  DataBuf[98] = 0x00;
  DataBuf[99] = 0x00;
  DataBuf[100] = 0x00;

  dword.v = st_Record.crgServiceFee*10;
  DataBuf[101] = dword.b[3];
  DataBuf[102] = dword.b[2];
  DataBuf[103] = dword.b[1];
  DataBuf[104] = dword.b[0];

  DataBuf[105] = 0x00;
  DataBuf[106] = 0x00;
  DataBuf[107] = 0x00;
  DataBuf[108] = 0x00;

  for (i = 0; i < 8; i++)
  {
    DataBuf[i + 109] = g_DeviceInfo.serNo[i];
  }

  return JWT_membuffer_alloc(0x08, 0x08, DataBuf, 117);
}


uint8* JWT_tx_0x33_charge_record(uint8 ch, ChargeRecord st_Record,uint16 *len)
{
  uint16 index=0;
  uint8 i;
  uint8 DataBuf[400];
  co_dword dword;
  struct tm *st_time;
  uint8 bcd_time[6] = {0};

  for (i = 0; i < 8; i++)
  {
    DataBuf[index + i] = st_Record.Reverse[i];
  }
  index +=8;

  if(st_Record.ChargeType == BY_APP_START)
    DataBuf[index++] = 2;
  else
    DataBuf[index++] = 1;

  asc2hex(&DataBuf[index], st_Record.CardNo, 16);
  index +=8;

  for (i = 0; i < 17; i++)
  {
    DataBuf[index + i] = 0x31;
  }
  index+=17;

  DataBuf[index++] = 0;
  DataBuf[index++] = 0;

  DataBuf[index++] = st_Record.StopReason;
  st_time = gmtime((const time_t *)&st_Record.StartTime);
  bcd_time[0] = HexToBcd(st_time->tm_year - 100);
  bcd_time[1] = HexToBcd(st_time->tm_mon + 1);
  bcd_time[2] = HexToBcd(st_time->tm_mday);
  bcd_time[3] = HexToBcd(st_time->tm_hour);
  bcd_time[4] = HexToBcd(st_time->tm_min);
  bcd_time[5] = HexToBcd(st_time->tm_sec);

  for (i = 0; i < 6; i++)
  {
    DataBuf[index + i] = bcd_time[i];
  }
  index+=6;

  st_time = gmtime((const time_t *)&st_Record.EndTime);
  bcd_time[0] = HexToBcd(st_time->tm_year - 100);
  bcd_time[1] = HexToBcd(st_time->tm_mon + 1);
  bcd_time[2] = HexToBcd(st_time->tm_mday);
  bcd_time[3] = HexToBcd(st_time->tm_hour);
  bcd_time[4] = HexToBcd(st_time->tm_min);
  bcd_time[5] = HexToBcd(st_time->tm_sec);

  for (i = 0; i < 6; i++)
  {
    DataBuf[index + i] = bcd_time[i];
  }
  index+=6;

  dword.v = st_Record.StartEnerge*10;
  DataBuf[index++] = dword.b[3];
  DataBuf[index++] = dword.b[2];
  DataBuf[index++] = dword.b[1];
  DataBuf[index++] = dword.b[0];

  dword.v = st_Record.EndEnerge*10;
  DataBuf[index++] = dword.b[3];
  DataBuf[index++] = dword.b[2];
  DataBuf[index++] = dword.b[1];
  DataBuf[index++] = dword.b[0];

  dword.v = (st_Record.EndEnerge - st_Record.StartEnerge)*10;
  DataBuf[index++] = dword.b[3];
  DataBuf[index++] = dword.b[2];
  DataBuf[index++] = dword.b[1];
  DataBuf[index++] = dword.b[0];

  dword.v = st_Record.ChargeMoney*10;
  DataBuf[index++] = dword.b[3];
  DataBuf[index++] = dword.b[2];
  DataBuf[index++] = dword.b[1];
  DataBuf[index++] = dword.b[0];

  dword.v = st_Record.crgServiceFee*10;
  DataBuf[index++] = dword.b[3];
  DataBuf[index++] = dword.b[2];
  DataBuf[index++] = dword.b[1];
  DataBuf[index++] = dword.b[0];

  JWT_record_feenum_callback(st_Record);
  DataBuf[index++] = stage_fee.feenum;
  for(i=0;i<stage_fee.feenum;i++)
  {
    DataBuf[index++] = stage_fee.fee_stype[i];
    memcpy(DataBuf + index,stage_fee.fee_start_time[i],6);
    index += 6;
    memcpy(DataBuf + index,stage_fee.fee_end_time[i],6);
    index += 6;
    dword.v = stage_fee.elefee[i]*10;
    DataBuf[index++] = dword.b[1];
    DataBuf[index++] = dword.b[0];
    dword.v = stage_fee.serfee[i]*10;
    DataBuf[index++] = dword.b[1];
    DataBuf[index++] = dword.b[0];
    dword.v = stage_fee.stage_energy[i]*10;
    DataBuf[index++] = dword.b[3];
    DataBuf[index++] = dword.b[2];
    DataBuf[index++] = dword.b[1];
    DataBuf[index++] = dword.b[0];
    dword.v = stage_fee.stage_elefee[i]*10;
    DataBuf[index++] = dword.b[3];
    DataBuf[index++] = dword.b[2];
    DataBuf[index++] = dword.b[1];
    DataBuf[index++] = dword.b[0];
    dword.v = stage_fee.stage_serfee[i]*10;
    DataBuf[index++] = dword.b[3];
    DataBuf[index++] = dword.b[2];
    DataBuf[index++] = dword.b[1];
    DataBuf[index++] = dword.b[0];
  }

  DataBuf[index++] = ch + 1;

  for (i = 0; i < 8; i++)
  {
    DataBuf[i + index] = g_DeviceInfo.serNo[i];
  }
  index +=8;

  *len = index + 8;
  return JWT_membuffer_alloc(0, 0x33, DataBuf, index);
}


int Cloud_Rcv_Data_Parse(Cloud_Comm_Info *comm)
{
    uint8 rcvBuf[300] ={0};
    co_dword dword;
    co_dword dword_dataLen;
    uint16 i,rcvLen=0;
    int tmpLen = 0;
    uint32 currentTime,getCrc;
    uint32 startTime = 0;
	currentTime= xTaskGetTickCount();
    do
	{
        tmpLen = comm->Read(comm->Port,rcvBuf,1,200);

        if(tmpLen > 0)
        {
            startTime= xTaskGetTickCount();
            for(i = 0; i < tmpLen; i++)
            {

                if(rcvLen == 0)
                {
                    if(rcvBuf[i] == 0X68)
                    {
                        comm->RcvBuf[rcvLen++] = rcvBuf[i];
                        continue;
                    }
                    rcvLen = 0;
                    continue;
                }

                if(rcvLen>=1&&rcvLen < 6)
                {
                    comm->RcvBuf[rcvLen++] = rcvBuf[i];
                    continue;
                }
                else
                {
                    dword_dataLen.b[0] = comm->RcvBuf[5];
                    dword_dataLen.b[1] = comm->RcvBuf[4];

                }
                if(rcvLen <((comm->RcvBuf[5]+comm->RcvBuf[4]*256) + 8))
                {
                    comm->RcvBuf[rcvLen++] = rcvBuf[i];
                }
                if(rcvLen==((comm->RcvBuf[5]+comm->RcvBuf[4]*256) + 8))
                {
         			dword.b[1] = comm->RcvBuf[6 + dword_dataLen.w[0]];
        			dword.b[0] = comm->RcvBuf[7 + dword_dataLen.w[0]];
        			getCrc = dword.w[0];

                    //uint16 crc = crc16_xmodem(&comm->RcvBuf[1],&comm->RcvBuf[rcvLen - 3]);
                    uint16 crc = crc16_xmodem(&comm->RcvBuf[1], &comm->RcvBuf[6 + dword_dataLen.w[0]]);
                    if(crc == getCrc)
                    {
                        comm->RcvLen =rcvLen;
                        return comm->RcvBuf[1];
                    }
                    else
                    {
                        rcvLen=0;
                        dbg_printf(1,"crc err");
                        return 0;
                    }
                }
            }
        }
        else
        {
            osDelay(20);
            return 0;
        }
        currentTime= xTaskGetTickCount();
	}while((currentTime-startTime)<=1000);
	return 0;
}

void Cloud_Task(const void * p)
{
    uint32 tick[4] = {0};

    //2021.8.13
    uint16 Fault = 0;//故障标记
    uint8  Fault_send_flag = 0;//故障上传标记

    u8_t login_flag = 0;
    uint8    Ch = 0;
    uint8    MsgFlag = 0;
    MSG_Def  Msg;

	  u8_t g_connect_flag = 0;
    co_dword dword;
    uint16 lenth = 0;
	  USART_Init(COM4,STOP_1,P_NONE, 115200);
    //10.29
    static uint32 wdog_timeout = 1;
    Tick_Out(&wdog_timeout,0);
    osDelay(5000);

    #if JWT_32_33
      osDelay(3000);
      JWT_first_fill_feenum();
    #endif

    while(1)
    {

		osDelay(20);
		login_flag = 0;
		int Rcvcmd = 0;
		loginStatus = e_LOGIN_OUT;
		comm.heartTick = time(NULL);
		int login_tick = 0;
		g_connect_flag = Cloud_Connect(&comm);
    //2020.12.4
    if(first_conect_byLAN == 1)
    {
      Relase_connect(&comm);
      first_conect_byLAN = 2;
      g_connect_flag = 0;
    }
		osDelay(2000);
    while(g_connect_flag)
    {
      if(loginStatus == e_LOGIN_OUT)
			{
				if(xTaskGetTickCount() - login_tick >  10000)
				{
		      dbg_printf(1,"send 01 cmd~~~~");
          comm.SendBuf = JWT_tx_0x01_login(comm.SendBuf);
          comm.Send(comm.Port,comm.SendBuf,37,0);
          osDelay(1000);
          login_flag ++;
					if(login_flag >= 5)
					{
						g_connect_flag =0;
            Relase_connect(&comm);
					}

						login_tick = xTaskGetTickCount();
				 }

			}
        #if 1
        else
	    {
    		MsgFlag = MsgQueue_Get(MSG_QUEUE_TASK_CLOUD_COMM, &Msg);
    		if (MsgFlag)
    		{
                Ch = 0;//Msg.Ch;
    		}
			//LED_Function(NET_LED, LED_On);
			if (MsgFlag)
			{
				switch (Msg.type)
				{
          case JWT_card_start:
          {
              //if (g_JWT_info_t.card_start_rx_flag != 1)
              {
                  comm.SendBuf = comm.SendBuf = JWT_tx_0x04_card_charge_start(0, comm.SendBuf );
                  comm.Send(comm.Port,comm.SendBuf,38,0);
              }
          }
          break;

		      case JWT_card_end:
    			{
    			    if (g_JWT_info_t.card_stop_rx_flag != 1)
    			    {
    				    comm.SendBuf = JWT_tx_0x05_card_charge_stop(0, comm.SendBuf );
    					comm.Send(comm.Port,comm.SendBuf,33,0);
    			    }
    			}
			    break;

          case JWT_APP_start_success:
          {
            dbg_printf(1,"send 86 cmd charge success");
          	comm.SendBuf = JWT_tx_0x86_app_charge_start_response(0, 0, 0, comm.SendBuf );
          	comm.Send(comm.Port,comm.SendBuf,25,0);
          }
          break;

	        case JWT_APP_start_fail:
          {
            dbg_printf(1,"send 86 cmd charge fail");
          	comm.SendBuf = JWT_tx_0x86_app_charge_start_response(0, 2, 0, comm.SendBuf );
          	comm.Send(comm.Port,comm.SendBuf,25,0);
          }
          break;

            case JWT_charge_progress:
    			{
              if (g_Charge_Ctrl_Manager[0].CurState == STATE_CHARGEING)
              {
                  if(Tick_Out(&tick[0],10000))
                  {
                      Tick_Out(&tick[0],0);
                      #if JWT_32_33
                        comm.SendBuf = JWT_tx_0x32_charge_progress(0, comm.SendBuf,&lenth);
                        comm.Send(comm.Port,comm.SendBuf,lenth,0);
                        osDelay(100);
                        dbg_printf(1,"send 32 cmd");
                        print_hex_data(comm.SendBuf,lenth);
                        osDelay(500);
                      #else if
                        dbg_printf(1,"send 09 cmd");
                        comm.SendBuf = JWT_tx_0x09_charge_progress(0, comm.SendBuf );
                        comm.Send(comm.Port,comm.SendBuf,120,0);
                        osDelay(500);
                      #endif
                      comm.SendBuf = JWT_tx_0x0A_charge_progress_data(0, comm.SendBuf );
                      comm.Send(comm.Port,comm.SendBuf,148,0);
                      dbg_printf(1,"send 0A CMD");
				              osDelay(500);

                  }
              }
    			}
	        break;

            case JWT_warning:
    			{
						uint16 warning_code;

						dword.b[1] = Msg.data[0];
						dword.b[0] = Msg.data[1];
						warning_code = dword.w[0];
            //dbg_printf(1,"11 warning_code=%d",warning_code);
						dword.b[3] = Msg.data[3];
						dword.b[2] = Msg.data[4];
						dword.b[1] = Msg.data[5];
						dword.b[0] = Msg.data[6];

						comm.SendBuf = JWT_tx_0x0D_charger_warning(0, warning_code, Msg.data[2], dword.v, comm.SendBuf );
						comm.Send(comm.Port,comm.SendBuf,30,0);
            osDelay(1000);
            dbg_printf(1,"send 0D cmd %d",Msg.data[2]);
            //print_hex_data(comm.SendBuf,30);
    			}
	        break;

            case JWT_QR_code:
    			{
    				comm.SendBuf = JWT_tx_0x1C_QR_request(0, comm.SendBuf );
    				comm.Send(comm.Port,comm.SendBuf,17,0);
    			}
	        break;

            case JWT_gun_status_change:
    			{
						dbg_printf(1,"send 1f CMD");
						//osDelay(2000);
						comm.SendBuf = JWT_tx_0x1F_gun_status(0, comm.SendBuf );
						comm.Send(comm.Port,comm.SendBuf,18,0);
						osDelay(2000);
    			}
	        break;

    			case JWT_charge_record:
  				{
  				}
	        break;

          case CTRL_BOOK_CANCEL:
  				{
  				}
	        break;

		      default:
			      break;
		        }
		    }
#if 1
    			if (g_Charge_Ctrl_Manager[0].CurState == STATE_CHARGEING)
    			{
    				memcpy(&g_RecordData.endStartTime[0], &g_JWT_info_t.heart_beat_time[0], 6);

    			}
//              #if 0
//    			else if ((g_RecordData.payMoney > 0) && (g_RecordData.payMoney <= 5) && (g_Charge_Ctrl_Manager[Ch].LastState == STATE_CHARGEING))
//    			{
//    			    g_RecordData.payMoney++;

//    				comm.SendBuf = JWT_tx_0x08_charge_record_form_flash(Ch, NULL);
//
//    				comm.Send(comm.Port,comm.SendBuf,125,0);
//    			}
//              #endif
            if(Tick_Out(&tick[2],15000)
                && (xTaskGetTickCount() - login_tick) > 35000)
            {
              static uint32_t start_time = 0;
              static char cnt = 0;
              if(Get_UnReport_Data(TYPE_CHARGELOG,(uint8*) &st_Record,0))
              {
                  #if JWT_32_33
                    comm.SendBuf = JWT_tx_0x33_charge_record(0, st_Record,&lenth);
          					comm.Send(comm.Port,comm.SendBuf,lenth,0);
                    osDelay(1000);
                    dbg_printf(1,"send 33 cmd");
                    print_hex_data(comm.SendBuf,lenth);
                  #else if
                    dbg_printf(1,"send 08 cmd");
          					comm.SendBuf = JWT_tx_0x08_charge_record_form_flash(0, st_Record);
          					comm.Send(comm.Port,comm.SendBuf,125,0);
                  #endif
                  if(start_time != st_Record.StartTime)
                  {
                      start_time = st_Record.StartTime;
                      cnt = 0;
                  }
                  else
                  {
                      cnt++;
                      if(cnt > 30)
                      {
                          cnt = 0;
                          Updata_UnReport_Data(TYPE_CHARGELOG,(uint8_t *)&st_Record,0);
                          dbg_printf(1,"give up send 08 cmd~~~~");
                      }
                  }
              }
			      else
      			{
      				if(g_Charge_Ctrl_Manager[0].CurState == STATE_IDLE && g_Sys_Config_Param.TCP_ud_flag == 2)
      				{


      					//重置
      			    g_Sys_Config_Param.TCP_ud_flag = 0;
      					g_Sys_Config_Param.TCP_ud_stop_adr = 0;
      					g_Sys_Config_Param.TCP_ud_stop_serial_number = 0;
      					//保存参数
      					API_Save_Sys_Param_Direct();
      					osDelay(2000);
      					dbg_printf(1,"state in idle  ");
      					//重启
      					dbg_printf(1,"reboot.....\n");
      					set_test_num(2);
      					NVIC_SystemReset();

      				}

      			}

			        Tick_Out(&tick[2],0);
            }
    			if(Tick_Out(&tick[1],15000))
            {
              dbg_printf(1,"send 0c cmd");
              Tick_Out(&tick[1],0);
              comm.SendBuf = JWT_tx_0x0C_heart_beat(0,comm.SendBuf );
              #if testcode_llz
              //comm.SendBuf = JWT_tx_0x0A_charge_progress_data(0, comm.SendBuf );
              #endif
              comm.Send(comm.Port,comm.SendBuf,18,0);
              osDelay(1000);
    			  }
          //2021.8.13
          if(Get_Alarm_State(Ch) != 0 && Fault_send_flag == 0)
          {
            Fault = Get_FaultBit(Ch);
            Ctrl_send_warning_message(Ch,Fault,1,0);
            Fault_send_flag = 1;
          }
          else if(Get_Alarm_State(Ch) == 0)
          {
            if(Fault_send_flag == 1)
            {
              Ctrl_send_warning_message(Ch,Fault,0,0);
              Fault_send_flag = 0;
              Fault = 0;
            }
          }
#endif
	}
        #endif

        Rcvcmd = Cloud_Rcv_Data_Parse(&comm);
        if(Rcvcmd >0)
        {
            comm.heartTick = time(NULL);
            dbg_printf(1,"rcv cmd %x",comm.RcvBuf[1]);
						if(Rcvcmd==0x81)
							dbg_printf(1,"cmd 81 result %x",comm.RcvBuf[14]);
          //switch(Rcvcmd)
          {
            if(Rcvcmd==0x81 && comm.RcvBuf[14] == 0)
            {
                loginStatus = e_LOGIN_IN;
								comm.SendBuf = JWT_tx_setting_response(0x8E, 0x00, comm.SendBuf );
                comm.Send(comm.Port,comm.SendBuf,17,1);

								osDelay(500);

								comm.SendBuf = JWT_tx_0x0B_billing_rule_query_response(comm.SendBuf );
								//print_hex_data(comm.SendBuf,comm.SendLen);


								osDelay(500);
								comm.SendBuf = JWT_tx_0x1F_gun_status(0, comm.SendBuf );
								comm.Send(comm.Port,comm.SendBuf,18,1);

								if(QR_flag == 0)
								{
									osDelay(1000);
									comm.SendBuf = JWT_tx_0x1C_QR_request(Ch,comm.SendBuf);
									comm.Send(comm.Port,comm.SendBuf,17,1);
									dbg_printf(1,"send 1C cmd");
									QR_flag = 1;
								}


          }
          else
          {

              uint16 rcv_cmd = JWT_daemon_rx_message_query(comm.RcvBuf,comm.RcvLen);
              if(rcv_cmd == 0x0b)
              {
                  comm.SendBuf = JWT_tx_0x8B_billing_rule_set_response(comm.SendBuf );
                  comm.Send(comm.Port,comm.SendBuf,25,1);
									//print_hex_data(comm.SendBuf, 25);
									//2020.12.5
									osDelay(500);
									JWT_first_fill_feenum();
              }

          }


					if(Rcvcmd == 0xA3)
					{
						Tick_Out(&tick[3],0);
					}
					else if(Rcvcmd == 0xA2)
					{
						Tick_Out(&tick[3],0);
					}
					else if(Rcvcmd == 0xA4)
					{
						Tick_Out(&tick[3],0);
					}

			//5min超时
			if(Tick_Out(&tick[3],300000) && g_Sys_Config_Param.TCP_ud_flag == 1)
       {
				//重置
		    g_Sys_Config_Param.TCP_ud_flag = 0;
				g_Sys_Config_Param.TCP_ud_stop_adr = 0;
				g_Sys_Config_Param.TCP_ud_stop_serial_number = 0;
				memset(&TCP_Info,0,sizeof(TCP_Info));

				//保存参数
				API_Save_Sys_Param_Direct();
				osDelay(1000);

				dbg_printf(1," 5 min timeout  clear %d",g_Sys_Config_Param.TCP_ud_flag);
				Tick_Out(&tick[3],0);
			}

            }
        }
        else
        {
            osDelay(20);
        }

			if(loginStatus==e_LOGIN_IN)
			{
			    if(abs(time(NULL) - comm.heartTick) > 90) //
			    {
            dbg_printf(1,"close");
						g_connect_flag =0;
            Relase_connect(&comm);

            //LED_Function(NET_LED, LED_Off);
			    }
			}
    }
    }
}
