

/******************************************************************************
* Includes
******************************************************************************/
#include "main.h"
#include "Tsk_API_Meter.h"

#include "Sys_Config.h"
#include <string.h>
#include <stdio.h>
#include "Tsk_Charge_Ctrl.h"
#include "MsgQueue.h"
#include "Tsk_API_Alarm.h"
#include "bsp_Led_Function.h"
#include <time.h>
#include "BSP_PWM.h"
#include "BSP_Adc.h"
#include "BSP_GPIO.h"
#include "Tsk_API_Card.h"
#include "BSP_Buzzer.h"
#include "BSP_WatchDog.h"
#include "lwip.h"
#include "Tsk_TouchDW.h"
#include "bsp_driver_uart.h"
#include "Tsk_Cloud_Com.h"
#include "Task_Spiflash.h"
#include "Tsk_Modbus.h"
#include "Drv_MMA8652FC.h"
//#include "BSP_I2C.h"
#include "Tsk_JWT.h"


#define DBG_INFO 0
#define DBG_CHARGE 1
/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/
#define MAX_LIFT_WAITE_TIME   60 * 3 //最大离开等待时间
#define MAX_BOOK_WAITE_TIME   60     //最大的等待预约取消时间

#define MIN_MONEY  100 //最小金额预留

#define Min_curr_10min_enable   1

/******************************************************************************
* Global Variable Definitions
******************************************************************************/

/******************************************************************************
* Data Type Definitions
******************************************************************************/
Charge_Ctrl g_Charge_Ctrl_Manager[2];
ChargeRecord gChargeRecord[2];
osThreadId Charge_Ctrl_Thread_Id;

uint16 get_current[10] = {0};

static uint32_t tick_lcd = 0;

/******************************************************************************
* Function Prototype Definitions
******************************************************************************/
ChargeRecord *Get_ChargeRecord(uint8 Ch);
osThreadId Get_Charge_Ctrl_Task_Id(void);
static void Charge_Ctrl_Init(void);
static uint8 Calculate_Pwm_Dutycycle(void);
uint8 Get_Charge_Ctrl_State(uint8 Ch);
uint8 Get_Charge_StartType(uint8 Ch);
static void State_Switch(uint8 CurState,uint8 Switch_To_State,uint8 Ch);
static void Change_Money_Compute_Fun(MEASURE_DATA MeterData,uint8 Ch);
static void Creat_Start_Charge_Record(uint8 Ch);
static void Change_Record_Compute(uint8 Ch);
void State_Idle_Msg_Deal(MSG_Def *Msg);
static void Give_UP_Charge_Proc(uint8 Ch);
static void State_Idle(uint8 Ch,uint8 MsgFlag,MSG_Def *Msg);
void State_Charge_Msg_Deal(MSG_Def *Msg);
static void State_Chargeing(uint8 Ch,uint8 MsgFlag,MSG_Def *Msg);
void State_Charge_Stop_Msg_Deal(MSG_Def *Msg);

static void State_Charge_Stop(uint8 Ch,uint8 MsgFlag,MSG_Def *Msg);
void State_Charge_Book_Msg_Deal(MSG_Def *Msg);
static void State_Charge_Book(uint8 Ch,uint8 MsgFlag,MSG_Def *Msg);
void Charge_Ctrl_Thread(const void *p);

uint8 Notice_Lcd_State(uint8 LcdState,uint8 Ch)
{
    uint8_t null_buf[16] = {0};
    uint32 Fault = Get_FaultBit(Ch); 
    dbg_printf(DBG_INFO,"Notice_Lcd_State %d",LcdState);
    if(CARD_NO_MONEY == LcdState)
    {
        osDelay(100);
        LCD_asc_display_frame(2,190,30,"余额不足",8);
        osDelay(5000);
        Dwin_Lcd_State(connected,0);
        //LCD_asc_display_frame(2,190,30,null_buf,16); 
    }
    else if(CARD_NOT_ACCOUNT == LcdState)
    {
        osDelay(100);
        LCD_asc_display_frame(2,160,30,"卡被锁定",8); 
        osDelay(1000);
        Dwin_Lcd_State(connected,0); 
        //LCD_asc_display_frame(2,160,30,null_buf,16);         
    }
    else if(END_CARD_NOT_MATCH == LcdState)
    {
        osDelay(100);
        LCD_asc_display_frame(1,160,2,"卡不匹配",10);
        osDelay(2000);
    }
    else if(SWIP_END_CARD == LcdState)
    {
        osDelay(100);
    		LCD_asc_display_frame(2,160,30,"请刷卡结算",10);
    }
    else if(CHARGE_FAIL == LcdState)
    {
        dbg_printf(DBG_INFO,"Notice_Lcd_State CHARGE_FAIL");

        osDelay(100);
        LCD_asc_display_frame(2,160,50,"启动超时",10);
        osDelay(3000);

        Dwin_Lcd_State(connected,0);
        
        //LCD_asc_display_frame(2,160,50,null_buf,16); 
    }
    
    if(RERTURN_IDLE == LcdState && Fault == 0)
    {  
      if(Get_Car_Link(Ch) == LEVEL_12_V)
      {
          if (g_Dwin_data.control_page != disconnected)
          {
						Dwin_Lcd_State(disconnected, Ch);
		        g_Dwin_data.control_page = disconnected;
						dbg_printf(DBG_INFO,"disconnected");
						osDelay(500);
          }

      }
      else
      {
          if (g_Dwin_data.control_page != connected)
          {
          	 osDelay(500);
             Dwin_Lcd_State(connected, Ch);
             g_Dwin_data.control_page = connected;
          }

      }
    }

    return 1;
}
/*  JWT  */

uint8 Dwin_Lcd_State(uint8 LcdState,uint8 Ch)
{
  uint8 i;
  char Buff[100] = {0};
  uint8 CurIndex     = 0;
  uint8 CurTimeHour  = 0;
  struct tm *curTime = {0};
  time_t time_val = time(NULL);
  curTime = gmtime(&time_val);
  CurTimeHour = curTime->tm_hour;
  CurIndex = CurTimeHour * 2;
  uint8 len;
  char dispalay_ip[36]= {0};
	uint8 local_ip[4] = {0};
	
  if (curTime->tm_min > 30)
  {
    CurIndex += 1;
  }

  if ((CurIndex < 0) || (CurIndex > 47))
  {
    CurIndex = 0;
  }
		
  
	Tick_Out(&tick_lcd, 0);

  //10.28
  Get_Local_Ip(local_ip);
  snprintf(dispalay_ip,16,"%d.%d.%d.%d",local_ip[0] ,local_ip[1] ,local_ip[2] ,local_ip[3]);
  osDelay(15);

  switch(LcdState)
  {
    case welcome_page:
      {
        LCD_switch_images_frame(welcome_page);
      }
      break;

    case disconnected:
      {
        osDelay(50);
        LCD_switch_images_frame(homepage);
        osDelay(300);
        LCD_ICON_display_frame(disconnected);
        osDelay(10);
        LCD_variables_display_frame(2, 3, 2, 0x135, 0xA2, g_Sys_Config_Param.Price[CurIndex]);
				osDelay(15);
        LCD_variables_display_frame(2, 3, 2, 0x13C, 0xD2, g_Sys_Config_Param.CrgServFee);
        osDelay(15);      
        //LCD_asc_display_frame(1,432,258,SoftVer,5);
        LCD_asc_display_frame(1,432,258,"V1.30",5);
        osDelay(15);
        LCD_asc_display_frame(1,350,4,g_Sys_Config_Param.Serial,16);
				osDelay(50);
        LCD_asc_display_frame(1,5,258,dispalay_ip,16);
				osDelay(15);

      }
      break;

    case connected:
      {
        //LCD_switch_images_frame(homepage);
        //osDelay(200);
        osDelay(50);
        LCD_switch_images_frame(turn_on_page);
				osDelay(100);

				i = 0;
				len = 0;

				memcpy(Buff,g_Sys_Config_Param.QRCode,strlen(g_Sys_Config_Param.QRCode));
				len += strlen(g_Sys_Config_Param.QRCode);

				memcpy(Buff+len,g_Sys_Config_Param.Serial,16);
				len += 16;
				
				Buff[len++] = 0x30;
				Buff[len++] = 0x31;

				dbg_printf(1,"QR= %s" ,Buff);
				LCD_QR_code_display_frame(Buff, len);
		
      }
      break;

    case conuntdown:
      {
        dbg_printf(1,"in conuntdown page");
        LCD_switch_images_frame(homepage);
				osDelay(20);
        LCD_switch_images_frame(starting_page);
        osDelay(300);
        
      }
      break;

    case charging:
      {
		MEASURE_DATA MeterData = {0};
        MeterData = Get_Measrue_Data(Ch);
        osDelay(50);
        LCD_switch_images_frame(homepage);
        osDelay(20);
        LCD_ICON_display_frame(charging);
		    osDelay(200);

        LCD_variables_display_frame(2, 3, 0, 0x135, 0x45, gChargeRecord[Ch].ChargeTime/60);
				osDelay(15);
				LCD_variables_display_frame(2, 3, 2, 0x80, 0x73, (gChargeRecord[Ch].EndEnerge  - gChargeRecord[Ch].StartEnerge));
				osDelay(15);
        LCD_variables_display_frame(2, 3, 2, 0x135, 0x73, gChargeRecord[Ch].totalFee);
				osDelay(15);
        LCD_variables_display_frame(2, 3, 2, 0x80, 0xA2, MeterData.CurrentA);
				osDelay(15);
        LCD_variables_display_frame(2, 3, 2, 0x135, 0xA2, g_Sys_Config_Param.Price[CurIndex]);
				osDelay(15);
        LCD_variables_display_frame(2, 3, 2, 0x80, 0xD2, MeterData.VolageA*10);
				osDelay(15);
        LCD_variables_display_frame(2, 3, 2, 0x13C, 0xD2, g_Sys_Config_Param.CrgServFee);
        osDelay(15);      
        //LCD_asc_display_frame(1,432,258,SoftVer,5);  
        LCD_asc_display_frame(1,432,258,"V1.30",5); 
        osDelay(15);
        LCD_asc_display_frame(1,350,4,g_Sys_Config_Param.Serial,16);
        osDelay(50);
        LCD_asc_display_frame(1,5,258,dispalay_ip,16);
        osDelay(15);
      }
      break;


    case bill_page:
      {
      	//LCD_switch_images_frame(homepage);
				osDelay(50);
        LCD_switch_images_frame(bill_page);
				osDelay(300);

        LCD_variables_display_frame(2, 3, 2, 0xC0, 0x50, gChargeRecord[Ch].totalFee);
				osDelay(15);
				LCD_variables_display_frame(2, 3, 2, 0xC0, 0x70, (gChargeRecord[Ch].EndEnerge  - gChargeRecord[Ch].StartEnerge));
				osDelay(15);
        LCD_variables_display_frame(2, 3, 2, 0xC0, 0x8E, gChargeRecord[Ch].ChargeTime/60 *100);
				osDelay(15);
        LCD_variables_display_frame(2, 4, 2, 0xCD, 0xAD, gChargeRecord[Ch].Balance);
      }
      break;

    case finished:
      {
        osDelay(50);
        LCD_switch_images_frame(homepage);
				osDelay(20);
        LCD_ICON_display_frame(finished);
        osDelay(100);
        LCD_variables_display_frame(2, 3, 0, 0x135, 0x45, gChargeRecord[Ch].ChargeTime/60);
				osDelay(15);
				LCD_variables_display_frame(2, 3, 2, 0x80, 0x73, gChargeRecord[Ch].EndEnerge  - gChargeRecord[Ch].StartEnerge);
				osDelay(15);
        LCD_variables_display_frame(2, 3, 2, 0x135, 0x73, gChargeRecord[Ch].totalFee);
				osDelay(15);
        LCD_variables_display_frame(2, 3, 2, 0x135, 0xA2, g_Sys_Config_Param.Price[CurIndex]);
				osDelay(15);
				LCD_variables_display_frame(2, 3, 2, 0x13C, 0xD2, g_Sys_Config_Param.CrgServFee);
        osDelay(150);
      }
      break;

    case fault:
      {
      	dbg_printf(1,"Get_FaultBit(Ch) = %d ",Get_FaultBit(Ch));
        LCD_switch_images_frame(homepage);
				osDelay(20);         
        LCD_switch_images_frame(fault_page);
				osDelay(300);
        LCD_variables_display_frame(4, 2, 0, 0xE6, 0x86, Get_FaultBit(Ch));
				osDelay(150);
      }
      break;

    default:
      break;
  }

	return 0;
}
/********************************************************
描    述 : 获取充电时的数据
输入参数 :CH 通道
输出参数 :无
返    回 :

作:>>?
创建时间 :18-06-07
*********************************************************/
ChargeRecord *Get_ChargeRecord(uint8 Ch)
{
    return &gChargeRecord[Ch];
}

/********************************************************
描    述 : 获取本任务的ID
输入参数 :CH 通道
输出参数 :无
返    回 :

作    者 :

创建时间 :18-06-07
*********************************************************/
osThreadId Get_Charge_Ctrl_Task_Id(void)
{
    return Charge_Ctrl_Thread_Id;
}

/*********************************************************************
描    述 : 充电控制数据初始化
输入参数 :无
输出参数 :无
返    回 :无

作    者 :

创建时间 :18-06-07
**********************************************************************/
static void Charge_Ctrl_Init(void)   
{
    int GunNum = 0;
    char Ch = 0;
    uint8 cardType = 0;
    uint8 CurType = Get_Cur_Out_Type();


    if(Get_DoubelGunEn() == DISABLE)
    {
        GunNum = 1;
    }
    else
    {
        GunNum = 2;
    }

    if(CurType == CUR_16)
    {
        EXTI_ZCD_Config(80);//过零检测8ms延时定时器
    }

    for(Ch = 0; Ch < GunNum;Ch++)
    {
        g_Charge_Ctrl_Manager[Ch].StateStamp= HAL_GetTick();
        g_Charge_Ctrl_Manager[Ch].LastState = STATE_IDLE;
        g_Charge_Ctrl_Manager[Ch].CurState = STATE_IDLE;

        Notice_Ctrl_Lock(LOCK_GUN,Ch);
    }
    //读卡器相关初始化调用
    //USART_Init(COM5,STOP_1,0,115200);       //BaudRate 刷卡波特率  wangyu mask

    cardType = CARD_M2; //Get_Card_Read_Type();

    switch(cardType)
    {
        case CARD_M2://M2卡
        {
            M2_Card_Read_init();
            break;
        }//预留给其他卡的类型
        default:
            break;
    }

    //BSP_Pwm_Init();//pwm初始化 //wangyu mask
//wangyu mask begin
//    BSP_LED_Init(RED_LED1);
//    BSP_LED_Init(YALLOW_LED1);
//    BSP_LED_Init(GRERN_LED1);
//    BSP_LED_Init(BOARD_LED);
//    BSP_LED_Init(NET_LED);

//    LED_Function(GRERN_LED1,LED_On);
//    LED_Function(RED_LED1,LED_Off);
//    LED_Function(YALLOW_LED1,LED_Off);
//    LED_Function(NET_LED,LED_Off);
//wangyu mask end 
    //MX_ADC1_Init();         //ADC 初始化 CP信号采集   wangyu mask 

//    BSP_Buzzer_Init();//蜂鸣器
//wangyu mask begin 
    //Dwin_Lcd_State(welcome_page,Ch);
		//wangyu mask end
}

/********************************************************
描    述 :计算PWM占空比
输入参数 :void

输出参数 :无
返    回 :无

作    者 :

创建时间 :18-06-07
*********************************************************/
static uint8 Calculate_Pwm_Dutycycle(void)
{

    uint8 CurType = Get_Cur_Out_Type();
    float Pwm = 0;
    uint8 CurVal = 0;
    if(CurType == CUR_16)
    {
        CurVal = 16;
    }
    else if(CurType == CUR_32)
    {
        CurVal = 32;
    }
    else if(CurType == CUR_63)
    {
        CurVal = 63;
    }
    else
    {
       CurVal = 16;
    }

    if(CurVal <= 51)
    {
        Pwm = CurVal/0.6;//(放大一百倍)
    }
    else
    {
        Pwm =  CurVal/2.5 + 64;
    }


    return Pwm;

}


/********************************************************
描    述 :充电状态机
输入参数 :Ch 枪号

输出参数 :无
返    回 :无

作    者 :

创建时间 :18-06-07
*********************************************************/
uint8 Get_Charge_Ctrl_State(uint8 Ch)
{
    return g_Charge_Ctrl_Manager[Ch].CurState;
}

uint32 Get_Book_Time(uint8 Ch)
{
    return g_Charge_Ctrl_Manager[Ch].BookTime;
}

uint8 Get_Continue_Charge(uint8 Ch)
{
    return g_Charge_Ctrl_Manager[Ch].ChargeContinu;
}

/********************************************************
描    述 :获取启动类型
输入参数 :Ch 枪号

输出参数 :无
返    回 :无

作    者 :

创建时间 :18-06-07
*********************************************************/
uint8 Get_Charge_StartType(uint8 Ch)
{
    return g_Charge_Ctrl_Manager[Ch].ChargeType;
}


/********************************************************
描    述 : 切换函数
输入参数 :Ch 枪号

输出参数 :无
返    回 :无

作    者 :

创建时间 :18-06-07
*********************************************************/

static void State_Switch(uint8 CurState,uint8 Switch_To_State,uint8 Ch)
{
   MSG_Def Msg = {0};
   Msg.type = JWT_gun_status_change;

    if(g_Charge_Ctrl_Manager[Ch].CurState != Switch_To_State)
    {
        g_Charge_Ctrl_Manager[Ch].StateStamp = time(NULL);

        MsgQueue_Send(MSG_QUEUE_TASK_CLOUD_COMM,Msg);
    }

    g_Charge_Ctrl_Manager[Ch].LastState = CurState;
    g_Charge_Ctrl_Manager[Ch].CurState = Switch_To_State;

    dbg_printf(DBG_INFO,"switch state CurState %d To %d",CurState,Switch_To_State);
}


/********************************************************
描    述 : 充电金额计算函数 支持多费率
输入参数 :Ch 枪号
         MeterData 表的读数
输出参数 :无
返    回 :无

作    者 :

创建时间 :18-06-07
*********************************************************/
static void Change_Money_Compute_Fun(MEASURE_DATA MeterData,uint8 Ch)
{
    uint8 CurTimeHour = 0;
    struct tm *curTime = {0};
    time_t time_val = time(NULL);
    static uint8 old_min[2] = {0xff,0xff};
    static uint32 old_Meter_Val[2] = {0};
    static float TotMoney = 0;
    uint32 Totol_Energy = 0;
    uint8 CurIndex = 0;
    int i = 0;
    SYS_CONFIG_PARAM  *sysParam = GetSys_Config_Param();
    curTime = gmtime(&time_val);
		
    if(gChargeRecord[Ch].SectionNum == 0)
    {
        old_Meter_Val[Ch] = gChargeRecord[Ch].StartEnerge;
        old_min[Ch] = curTime->tm_min;//clear
        gChargeRecord[Ch].SectionNum = 1;
        memset(gChargeRecord[Ch].SectionEnerge,0,sizeof(gChargeRecord[Ch].SectionEnerge));
        TotMoney = 0;//clear
    }

    else if((curTime->tm_min == 30 ||curTime->tm_min == 0)
         && old_min[Ch]  != curTime->tm_min)
    {
        old_min[Ch] = curTime->tm_min;
        gChargeRecord[Ch].SectionNum++;
        dbg_printf(DBG_INFO,"时间等于整点");
    }

    Totol_Energy = MeterData.Totol_Energy - gChargeRecord[Ch].StartEnerge;

    #if 0
    if(gChargeRecord[Ch].SectionNum == 1)
    {
        gChargeRecord[Ch].SectionEnerge[0] = Totol_Energy;
    }
    else
    {
        uint32 Front = 0;
        for(i = 0; i < gChargeRecord[Ch].SectionNum - 1;i++)
        {
            Front += gChargeRecord[Ch].SectionEnerge[i];
        }
        gChargeRecord[Ch].SectionEnerge[i] = Totol_Energy - Front;
    }
    #endif

    if(old_Meter_Val[Ch] != MeterData.Totol_Energy
        && MeterData.Totol_Energy - old_Meter_Val[Ch] < 1000)
    {

        uint32 tmp_Energy = MeterData.Totol_Energy - old_Meter_Val[Ch];
        float money  = 0;


        CurTimeHour = curTime->tm_hour;
        CurIndex = CurTimeHour * 2;

        if(curTime->tm_min > 30)
        {
            CurIndex += 1;//当前时间的区间
        }
				
				
        money = tmp_Energy * Get_Price(CurIndex);
        money /= 100.0;
        TotMoney += money;
        dbg_printf(DBG_INFO,"money %d 电表数 %d - %d, 第%d个时间区间",(uint16)TotMoney,MeterData.Totol_Energy,old_Meter_Val[Ch],CurIndex);

        gChargeRecord[Ch].ChargeMoney = (uint16)TotMoney;
        old_Meter_Val[Ch] = MeterData.Totol_Energy;
				gChargeRecord[Ch].crgServiceFee = (Totol_Energy * sysParam->CrgServFee)/100;
				gChargeRecord[Ch].totalFee = TotMoney + (Totol_Energy * sysParam->CrgServFee)/100;

		
		
        uint32 Front = 0;
        
        for(i = 0; i< 48;i++)
        {
            
            if(i != CurIndex)
            {
                Front += gChargeRecord[Ch].SectionEnerge[i];
            }
        }
        
        gChargeRecord[Ch].SectionEnerge[CurIndex] = Totol_Energy - Front;
    }
}
/********************************************************
描    述 : 转化卡号
输入参数 :DispBuf 转换后
          BCD码的卡号
输出参数 :无
返    回 :无

作    者 :

创建时间 :18-06-07
*********************************************************/
void shift_card_no(uint8 *DispBuf,uint8 *card,int len)
{

	int i = 0;
	int index = 0;

	memset(DispBuf,0,len * 2);

	for(i = 0; i < len;i++)
	{
		sprintf((char*)DispBuf + index ,"%2.2X",card[i]);

		index = strlen((const char*)DispBuf);

	}

}
/********************************************************
描    述 : 创建开始充电记录
输入参数 :CH 通道
输出参数 :无
返    回 :无

作    者 :

创建时间 :18-06-07
*********************************************************/
static void Creat_Start_Charge_Record(uint8 Ch)
{
    MEASURE_DATA MeterData = {0};

    MeterData = Get_Measrue_Data(Ch);
    if(g_Charge_Ctrl_Manager[Ch].ChargeType == BY_APP_START)
    {//app直接转化
        memcpy(gChargeRecord[Ch].CardNo,g_Charge_Ctrl_Manager[Ch].CardData.Card_no,sizeof(gChargeRecord[Ch].CardNo));
        gChargeRecord[Ch].ListID = g_Charge_Ctrl_Manager[Ch].ListId;

        memcpy(gChargeRecord[0].Reverse,g_JWT_info_t.order_NO,8);
    }
    else
    {//卡号是hex码转换为ascii
        shift_card_no(gChargeRecord[Ch].CardNo,g_Charge_Ctrl_Manager[Ch].CardData.Card_no,8);
        gChargeRecord[Ch].ListID = 0;
        Ctrl_card_charge(Ch, JWT_card_start, gChargeRecord[Ch].CardNo);
    }

    gChargeRecord[Ch].Balance = g_Charge_Ctrl_Manager[Ch].CardData.Balance;

    gChargeRecord[Ch].ChargeMode = g_Charge_Ctrl_Manager[Ch].ChargeMode;
    gChargeRecord[Ch].MaxVolt = 0;
    gChargeRecord[Ch].MaxCurent = 0;
    gChargeRecord[Ch].ChargeType = g_Charge_Ctrl_Manager[Ch].ChargeType;
    gChargeRecord[Ch].PayFlag = UN_PAY;//没有付费
    gChargeRecord[Ch].Report = 0x1;     //没有上报
    gChargeRecord[Ch].EndFlag = NOT_END;   //没有正常结束 防止突然断电
    gChargeRecord[Ch].StartCardType = g_Charge_Ctrl_Manager[Ch].CardData.CardType;
    gChargeRecord[Ch].EndCardType = g_Charge_Ctrl_Manager[Ch].CardData.CardType;
    gChargeRecord[Ch].ChargeMoney = 0;
    gChargeRecord[Ch].totalFee = 0;
    gChargeRecord[Ch].ChargeTime = 0;
    gChargeRecord[Ch].StartEnerge = MeterData.Totol_Energy;
    gChargeRecord[Ch].EndEnerge = MeterData.Totol_Energy;
    gChargeRecord[Ch].StartTime =  time(NULL);
    gChargeRecord[Ch].EndTime =  time(NULL);
    gChargeRecord[Ch].SectionNum = 0;
    gChargeRecord[Ch].StopReason = 0;
    gChargeRecord[Ch].crgServiceFee = 0;//清零
    Save_Start_Charge_Info(Ch);
    g_Charge_Ctrl_Manager[Ch].current_1A_tick = 0;
    g_Charge_Ctrl_Manager[Ch].continue_flag = TRUE;
//    dbg_printf(DBG_INFO,"start charge time %s",ctime((const time_t *)&gChargeRecord[Ch].StartTime));
//    dbg_printf(DBG_INFO,"start energe %d",gChargeRecord[Ch].StartEnerge);
//    dbg_printf(DBG_INFO,"start charge mode %d",gChargeRecord[Ch].ChargeMode);
//    dbg_printf(DBG_INFO,"start card no");
//    dbg_printf(DBG_INFO,"start money %d",g_Charge_Ctrl_Manager[Ch].CardData.Balance);
//    print_hex_data(gChargeRecord[Ch].CardNo,sizeof(gChargeRecord[Ch].CardNo));
}
/********************************************************
描    述 : 计算更新充电时的数据
输入参数 :CH 通道
输出参数 :无
返    回 :无

作    者 :

创建时间 :18-06-07
*********************************************************/
static void Change_Record_Compute(uint8 Ch)
{
    MSG_Def Msg_send = {0};
    //MSG_Def Msg_progress_data = {0};
    MEASURE_DATA MeterData = {0};
    static uint8 OldState[2] = {0};
    static uint32 OldMaxVol[2] ={0};
    static uint32 OldMaxCur[2] ={0};
    uint32 Book = Get_Book_Time(Ch);
    MeterData = Get_Measrue_Data(Ch);

    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR3,gChargeRecord[Ch].totalFee);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR4,gChargeRecord[Ch].EndEnerge);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR5,(gChargeRecord[Ch].EndEnerge >> 16));
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR6,gChargeRecord[Ch].ChargeTime);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR7,gChargeRecord[Ch].EndTime);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR8,gChargeRecord[Ch].EndTime>>16);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR9,Book);//当前的预约时间
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR10,Book>>16);//当前的预约时间
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR11,g_Charge_Ctrl_Manager[Ch].CurState);//预约状态实时存
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR14,gChargeRecord[Ch].ChargeMoney);
    if(g_Charge_Ctrl_Manager[Ch].CurState == STATE_CHARGEING
        && OldState[Ch] == STATE_IDLE)
    {//充电开始数据记录

        if(g_Charge_Ctrl_Manager[Ch].ChargeContinu == FALSE)
        {
            Creat_Start_Charge_Record(Ch);
						
            OldMaxVol[Ch] = 0;
            OldMaxCur[Ch] = 0;

        }

    }
    else if((OldState[Ch] == STATE_CHARGEING)
        && g_Charge_Ctrl_Manager[Ch].CurState != STATE_CHARGEING)
    {   //充电结束记录数据

        uint32 Energe = gChargeRecord[Ch].EndEnerge  - gChargeRecord[Ch].StartEnerge;

        gChargeRecord[Ch].EndCardType = g_Charge_Ctrl_Manager[Ch].CardData.CardType;
        gChargeRecord[Ch].EndEnerge = MeterData.Totol_Energy;
        gChargeRecord[Ch].EndTime =  time(NULL);

        dbg_printf(1,"充电账单");
        dbg_printf(1,"充电时间 %d：%d：%d",gChargeRecord[Ch].ChargeTime/3600,(gChargeRecord[Ch].ChargeTime/60)%60,(gChargeRecord[Ch].ChargeTime%60));
        dbg_printf(1,"充电金额 %d.%d 元",gChargeRecord[Ch].totalFee/100,gChargeRecord[Ch].totalFee % 100);
        dbg_printf(1,"充电电量 %d.%2.2d Kwh",Energe/100,Energe%100);
        dbg_printf(1,"充电卡号：");
        print_hex_data(gChargeRecord[Ch].CardNo,16);
    }
    else if(g_Charge_Ctrl_Manager[Ch].CurState == STATE_CHARGEING
        && OldState[Ch] == STATE_CHARGEING)
    {//充电中记录数据
        static uint32 tick[2] = {0};

        if(MeterData.VolageA > OldMaxVol[Ch])
        {
             OldMaxVol[Ch] = MeterData.VolageA;
        }

        if(MeterData.CurrentA > OldMaxCur[Ch])
        {
             OldMaxCur[Ch] = MeterData.CurrentA;
        }
				gChargeRecord[Ch].EndEnerge = MeterData.Totol_Energy;
				
        gChargeRecord[Ch].MaxVolt =  OldMaxVol[Ch];
        gChargeRecord[Ch].MaxCurent = OldMaxCur[Ch];
        gChargeRecord[Ch].EndTime =  time(NULL);
        gChargeRecord[Ch].ChargeTime = gChargeRecord[Ch].EndTime - gChargeRecord[Ch].StartTime;
        Change_Money_Compute_Fun(MeterData,Ch);

				
        if(Tick_Out(&tick[Ch],10000))
        {
            uint32 Energe = gChargeRecord[Ch].EndEnerge  - gChargeRecord[Ch].StartEnerge;
            dbg_printf(DBG_INFO,"充电时间 %d：%d：%d",gChargeRecord[Ch].ChargeTime/3600,(gChargeRecord[Ch].ChargeTime/60)%60,(gChargeRecord[Ch].ChargeTime%60));
            dbg_printf(DBG_INFO,"充电电能 %d.%2.2d Kwh",Energe/100,Energe%100);
            dbg_printf(DBG_INFO,"充电电流  %d.%2.2d A",MeterData.CurrentA/100,MeterData.CurrentA%100);
            dbg_printf(DBG_INFO,"充电电压  %d.%d V",MeterData.VolageA/10,MeterData.VolageA%10);
            dbg_printf(DBG_INFO,"充电金额  %d.%2.2d 元",gChargeRecord[Ch].totalFee/100,gChargeRecord[Ch].totalFee%100);
            Tick_Out(&tick[Ch],0);//DBG_INFO
					  Dwin_Lcd_State(charging, Ch);
            g_Dwin_data.control_page = charging;
            Msg_send.type = JWT_charge_progress;
            MsgQueue_Send(MSG_QUEUE_TASK_CLOUD_COMM, Msg_send);

            //Msg_progress_data.type = JWT_charge_progress_data;
            //MsgQueue_Send(MSG_QUEUE_TASK_CLOUD_COMM, Msg_progress_data);
        }

    }

    else if(OldState[Ch] == STATE_END_CHARGE &&
        g_Charge_Ctrl_Manager[Ch].CurState != STATE_END_CHARGE)
    {//clear
        if(g_Charge_Ctrl_Manager[Ch].ChargeContinu == FALSE)
        {
            memset(&g_Charge_Ctrl_Manager[Ch],0,sizeof(g_Charge_Ctrl_Manager[Ch]));
            dbg_printf(1,"clear manger strcut");
        }

			  //osDelay(3000);
			  //Notice_Lcd_State(homepage, Ch);
    }

    if (OldState[Ch] != g_Charge_Ctrl_Manager[Ch].CurState)
    {
        OldState[Ch] = g_Charge_Ctrl_Manager[Ch].CurState;
    }
}


/*********************************************************************
描    述 : 充电控制任务空闲状态状态函数
输入参数 :
          Msg 消息结构体
输出参数 :无
返    回 :无

作    者 :

创建时间 :18-06-07
**********************************************************************/

void State_Idle_Msg_Deal(MSG_Def *Msg)
{

    uint8 MsgType = Msg->type;
    uint8 Ch = Msg->Ch;
    uint8 Pwm = Calculate_Pwm_Dutycycle();
    switch(MsgType)
    {
        case CTRL_START_CHARGE:
        case CTRL_CONTUNE_CHARGE:
        {
//            //app启动充电
//            /*
//                1.发pwm
//                2.提示启动中
//            */

            {//
                if(Get_Alarm_State(Ch) == 0LL)
                {
                    if(Get_LocKEn())
                    {//先开锁
                        Notice_Ctrl_Lock(UNLOCK_GUN,Ch);
						dbg_printf(DBG_INFO,"开锁.......");
                    }
                    if(MsgType == CTRL_CONTUNE_CHARGE)
                    {
                        g_Charge_Ctrl_Manager[Ch].ChargeContinu = TRUE;

                        dbg_printf(DBG_INFO,"继续充电");
                    }
                    else
                    {
                        g_Charge_Ctrl_Manager[Ch].ChargeContinu = FALSE;
                    }

                    if (g_Dwin_data.control_page != conuntdown)
                    {
                      Dwin_Lcd_State(conuntdown,Ch);
                      g_Dwin_data.control_page = conuntdown;
                    }
                    g_Charge_Ctrl_Manager[Ch].ChargeType = BY_APP_START;
                    g_Charge_Ctrl_Manager[Ch].StateStamp = time(NULL);//记录状态变化

                }
                else
                {
                    dbg_printf(DBG_INFO,"故障无法启动充电");
                }
            }
            break;
        }
        case AUTH_ONLINE_CARD:
        {//、、0-19 no 20-23 mooney 24 ztai
            uint8 statu = Msg->data[24];
            if(statu == 0)
            {//正常

                if(Get_LocKEn() == TRUE )
                {

                }
//                memcpy(g_Charge_Ctrl_Manager[Ch].CardData.Card_no,Msg->data,20);//卡号
//                g_Charge_Ctrl_Manager[Ch].CardData.CardType = ONLINE_CARD;
                memcpy(&g_Charge_Ctrl_Manager[Ch].CardData.Balance,&Msg->data[20],4);
                if(g_Charge_Ctrl_Manager[Ch].CardData.Balance >= MIN_ALLOW_MONEY)
                {
                    uint8 CP = Get_Car_Link(Ch);
                    if(g_Charge_Ctrl_Manager[Ch].ChargeMode == BOOK_MODE)
                    {//预约充电
                        g_Charge_Ctrl_Manager[Ch].ChargeType = BY_CARD_BOOK;
                        //预约成功
                        if(CP == LEVEL_9_V || CP == LEVEL_6_V)
                        {
                            Notice_Lcd_State(BOOK_SUCCES,Ch);
                            State_Switch(STATE_IDLE,STATE_BOOK,Ch);
                        }
                        else
                        {//提示插枪
                            Notice_Lcd_State(INSERT_GUN,Ch);
                        }

                    }
                    else
                    {
                        Notice_Lcd_State(READY_CHARGE,Ch);
                        if(g_Charge_Ctrl_Manager[Ch].ChargeType == BY_ON_LINE_AUTH)
                        {
                            g_Charge_Ctrl_Manager[Ch].ChargeType = BY_CARD_START;
                            dbg_printf(1,"on line card start charge!");
                        }
                        else
                        {
                            g_Charge_Ctrl_Manager[Ch].ChargeType = BY_APP_START;
                            dbg_printf(1,"app start charge!");
                        }
                        g_Charge_Ctrl_Manager[Ch].StateStamp = time(NULL);//记录状态变化
                    }

                    if (CP != LEVEL_12_V)
                    {
                        if (g_Dwin_data.control_page != conuntdown)
                        {
                          Dwin_Lcd_State(conuntdown,Ch);
                          g_Dwin_data.control_page = conuntdown;
                        }
                    }
                }
                else
                {//余额不足
                    Notice_Lcd_State(CARD_NO_MONEY,Ch);//正在使用
										dbg_printf(1,"CARD_NO_MONEY 卡余额不足");
                    g_Charge_Ctrl_Manager[Ch].ChargeType = CHARGE_OPT_NULL;
                }


            }
            else if(statu ==1)
            {//未结算

                Notice_Lcd_State(CARD_NOT_ACCOUNT,Ch);//正在使用
                g_Charge_Ctrl_Manager[Ch].ChargeType = CHARGE_OPT_NULL;
            }
            else if(statu == 2)
            {//冻结
                Notice_Lcd_State(CARD_ACCOUNT_LOCK,Ch);
                g_Charge_Ctrl_Manager[Ch].ChargeType = CHARGE_OPT_NULL;
            }
            else
            {
                Notice_Lcd_State(CARD_ONLINE_ERR,Ch);
                g_Charge_Ctrl_Manager[Ch].ChargeType = CHARGE_OPT_NULL;
            }
            break;
        }
        case CTRL_BOOK_CHARGE:
        {
            dbg_printf(DBG_INFO,"空闲收到预约命令 成功");
            State_Switch(STATE_IDLE,STATE_BOOK,Ch);
            Notice_Lcd_State(BOOK_SUCCES,Ch);
            g_Charge_Ctrl_Manager[Ch].ChargeType = BY_APP_BOOK;
            if(Get_LocKEn())
            {
                Notice_Ctrl_Lock(UNLOCK_GUN,Ch);//开锁
            }
            Creat_Start_Charge_Record(Ch);//创建一条没有消费的记录
            break;
        }

        default:
            break;
    }
    dbg_printf(1,"空闲收到 %x",MsgType);
}
/********************************************************
描    述 : 放弃充电或者超时处理流程
输入参数 :CH 通道
输出参数 :无
返    回 :无

作    者 :

创建时间 :18-06-07
*********************************************************/
static void Give_UP_Charge_Proc(uint8 Ch)
{
    uint8 CardRet = 0;
    static uint32 tick[2] = {0};
    int Balance = 0;
    uint8 CP = Get_Car_Link(Ch);
    if(Get_LocKEn())
    {//有锁的版本先要归位

        if(Get_Lock_Seat(Ch) == TRUE &&
            (CP == LEVEL_12_V))
        {//充电枪归位

            CARD_RECORD cardRecord = {0};

            if(g_Charge_Ctrl_Manager[Ch].CardData.CardType == BUSINESS_CARD)
            {//是运营卡

                if(Tick_Out(&tick[Ch],2000))
                {
                    Tick_Out(&tick[Ch],00);
                    Notice_Lcd_State(SWIP_END_CARD,Ch);
                }

                cardRecord.EndTime = gChargeRecord[Ch].EndTime;
                cardRecord.startTime = gChargeRecord[Ch].StartTime;
                cardRecord.Energe = gChargeRecord[Ch].EndEnerge - gChargeRecord[Ch].StartEnerge;
                cardRecord.Money = gChargeRecord[Ch].totalFee;//扣除总费用
                cardRecord.type = gChargeRecord[Ch].StartCardType;
                CardRet = API_End_Charge_Card(cardRecord,&Balance,Ch);
                //卡停止
                if(CardRet == END_OK)
                {
                    Notice_Beep(ONCE_RING,Ch);
                    BSP_Pwm_Out(100,Ch);
                    g_Charge_Ctrl_Manager[Ch].ChargeType = CHARGE_OPT_NULL;
                    g_Charge_Ctrl_Manager[Ch].CardData.Balance = Balance;
                    Notice_Ctrl_Lock(LOCK_GUN,Ch);//锁定
                    Notice_Lcd_State(RERTURN_IDLE,Ch);
                    //
                    Save_End_Charge_Opt_Info(Ch);
                }
                else if(CardRet == END_NOT_MATCH)
                {
                    Notice_Lcd_State(END_CARD_NOT_MATCH,Ch);
                }
            }
            else
            {
                BSP_Pwm_Out(100,Ch);
                g_Charge_Ctrl_Manager[Ch].ChargeType = CHARGE_OPT_NULL;
                Notice_Lcd_State(RERTURN_IDLE,Ch);//
                Notice_Ctrl_Lock(LOCK_GUN,Ch);//锁定
            }

        }
        else
        {
            if(Get_Lock_Seat(Ch) == FALSE)
            {
                if(Tick_Out(&tick[Ch],2000))
                {
                    Tick_Out(&tick[Ch],00);
                    Notice_Lcd_State(TAKE_BACK_GUN,Ch);
                }
            }
        }

    }
    else
    {//座子无锁的 刷卡第二次直接解锁卡片
        {
            if(g_Charge_Ctrl_Manager[Ch].CardData.CardType == BUSINESS_CARD)
            {//是运营卡

                //if(CP == LEVEL_12_V)
                {
                    CARD_RECORD cardRecord = {0};
                    if(Tick_Out(&tick[Ch],2000))
                    {
                        Tick_Out(&tick[Ch],00);
                        Notice_Lcd_State(SWIP_END_CARD,Ch);
                    }

                    cardRecord.EndTime = gChargeRecord[Ch].EndTime;
                    cardRecord.startTime = gChargeRecord[Ch].StartTime;
                    cardRecord.Energe = gChargeRecord[Ch].EndEnerge - gChargeRecord[Ch].StartEnerge;
                    cardRecord.Money = gChargeRecord[Ch].totalFee;//扣除总费用
                    cardRecord.type = gChargeRecord[Ch].StartCardType;
                    CardRet = API_End_Charge_Card(cardRecord,&Balance,Ch);
                    //卡停止
                    if(CardRet == END_OK)
                    {
                        Notice_Beep(ONCE_RING,Ch);

                        g_Charge_Ctrl_Manager[Ch].ChargeType = CHARGE_OPT_NULL;
                        g_Charge_Ctrl_Manager[Ch].CardData.Balance = Balance;
                        Notice_Lcd_State(RERTURN_IDLE,Ch);

                        Save_End_Charge_Opt_Info(Ch);
                    }
                    else if(CardRet == END_NOT_MATCH)
                    {
                        Notice_Lcd_State(END_CARD_NOT_MATCH,Ch);
                    }

                }
//                else
//                {//收回充电枪
//                    if(Tick_Out(&tick[Ch],2000))
//                    {
//                        Tick_Out(&tick[Ch],00);
//                        Notice_Lcd_State(TAKE_BACK_GUN,Ch);
//                    }
//                }

            }
            else
            {
                if(time(NULL) - g_Charge_Ctrl_Manager[Ch].StateStamp > 20)
                {
                    BSP_Pwm_Out(100,Ch);
                    g_Charge_Ctrl_Manager[Ch].ChargeType = CHARGE_OPT_NULL;
                    Notice_Lcd_State(RERTURN_IDLE,Ch);//
                    dbg_printf(1,"启动超时车辆没有准备到6V");
                }
            }

        }
    }

    if(CP == LEVEL_12_V
        && (time(NULL) - g_Charge_Ctrl_Manager[Ch].StateStamp > MAX_LIFT_WAITE_TIME))
    {
        memset(&g_Charge_Ctrl_Manager[Ch],0,sizeof(g_Charge_Ctrl_Manager[0]));//clear
        Notice_Lcd_State(RERTURN_IDLE,Ch);
        Save_UpPay_Info(Ch);
        if(Get_LocKEn())
        {
            Notice_Ctrl_Lock(LOCK_GUN,Ch);//锁定
        }
        dbg_printf(1,"人走了枪也不在了 等待超3min");
        g_Charge_Ctrl_Manager[Ch].ChargeType = CHARGE_OPT_NULL;
				Dwin_Lcd_State(disconnected, Ch);
		    g_Dwin_data.control_page = disconnected;
    }
}

static void NO_Card_Start_Charge_Probe(uint8 Ch)
{
    static uint8 Old_CP = LEVEL_12_V;
    uint8 CP = Get_Car_Link(Ch);
    if(Get_Card_Read_Type() == NO_CARD_START)
    {
        if(CP != LEVEL_12_V && Old_CP == LEVEL_12_V)
        {
            UsrAccount stUsrAccount = {0};
            stUsrAccount.Money = 9999;
            Ctrl_power_on(0,&stUsrAccount);
        }
    }
    if(CP != Old_CP)
    {
        Old_CP = CP;
    }
}
/*********************************************************************
描    述 : 充电控制任务空闲状态状态函数
输入参数 :
          Ch 通道
          MsgFlag 收到消息标记
          Msg 消息结构体
输出参数 :无
返    回 :无

作    者 :

创建时间 :18-06-07
**********************************************************************/

static void State_Idle(uint8 Ch,uint8 MsgFlag,MSG_Def *Msg)
{
    uint8 CardRet = OPT_ERR;

    MSG_Def Msg_send = {0};
    Msg_send.type = JWT_gun_status_change;
    CHARGE_MODE_INFO stMode_Info = {0};
    uint8 Pwm = Calculate_Pwm_Dutycycle();
    static uint32 NoticeSec[2] = {0};
    static uint32 readySec[2] = {0};
    static uint8 last_cp_val[2] = {0xff,0xff};
    uint8 CP = Get_Car_Link(Ch);
    CARD_DATA CardData = {0};
    //static uint8 updata_flag = 0;
	static uint32 idle_tick = 0;


	//if(Tick_Out(&idle_tick,60000) && g_Dwin_data.control_page != connected)
	if(Tick_Out(&idle_tick,60000))
	{
		if(g_Dwin_data.control_page == disconnected)
		{
			Tick_Out(&idle_tick,0);
			Dwin_Lcd_State(disconnected, Ch);
			g_Dwin_data.control_page = disconnected;
			dbg_printf(1,"1min turn to disconnected");
			osDelay(500);
		}
		else if(g_Dwin_data.control_page == fault)
		{
			Tick_Out(&idle_tick,0);
			Dwin_Lcd_State(fault, Ch);
			g_Dwin_data.control_page = fault;
			dbg_printf(1,"1min turn to fault");
			osDelay(500);
		}
		else if(g_Dwin_data.control_page == connected)
		{
			Tick_Out(&idle_tick,0);
			Dwin_Lcd_State(connected, Ch);
			g_Dwin_data.control_page = connected;
			dbg_printf(1,"1min turn to connected");
			osDelay(500);
		}
		else
		{
			Tick_Out(&idle_tick,0);
			dbg_printf(1,"1min not_change_page num=%d ",g_Dwin_data.control_page);
		}
		
	}
		
	
    if(last_cp_val[Ch] != CP)
    {
        last_cp_val[Ch] = CP;
        if(CP == LEVEL_9_V)
        {
            g_Charge_Ctrl_Manager[Ch].StateStamp = time(NULL);//记录状态变化
        }
        MsgQueue_Send(MSG_QUEUE_TASK_CLOUD_COMM,Msg_send);
    }
    if(Trial_Time_Check())
    {
        Notice_Lcd_State(TRIAL_TIME_END,Ch);
        osDelay(10000);
        return;
    }
    //升级中不能充电
//    if(Get_Cloud_Updata_Statu())
//    {
//        updata_flag = 1;
//        Notice_Lcd_State(UPDATAING,Ch);
//        osDelay(8000);
//        return;
//    }
//    else
//    {
//        if(updata_flag == 1)
//        {
//            updata_flag = 0;
//            Notice_Lcd_State(RERTURN_IDLE,Ch);
//        }
//
//    }

    //清除操作
    if(g_Charge_Ctrl_Manager[Ch].ChargeType == BY_CARD_STOP
        || g_Charge_Ctrl_Manager[Ch].ChargeType == BY_CARD_STOP
        || g_Charge_Ctrl_Manager[Ch].ChargeType == BY_OTEHER_STOP)
    {
        memset(&g_Charge_Ctrl_Manager[Ch],0,sizeof(g_Charge_Ctrl_Manager[0]));//clear
        g_Charge_Ctrl_Manager[Ch].ChargeType = CHARGE_OPT_NULL;
        dbg_printf(1,"clera opt");
    }

    if(MsgFlag == TRUE)
    {//处理本状态的消息
        State_Idle_Msg_Deal(Msg);
    }

    //故障不能启动
    if(Get_Alarm_State(Ch) != FALSE)
    {
        if(g_Charge_Ctrl_Manager[Ch].ChargeType != CHARGE_OPT_NULL)
        {
            g_Charge_Ctrl_Manager[Ch].ChargeType = CHARGR_OPT_TIME_OUT;

            if(Tick_Out(&NoticeSec[Ch],3000))
            {
                dbg_printf(1,"-opt timeout");
                Tick_Out(&NoticeSec[Ch],0);
            }
        }
    }

    NO_Card_Start_Charge_Probe(Ch);

    if(g_Charge_Ctrl_Manager[Ch].ChargeType == CHARGE_OPT_NULL)
    {
        
        if((Get_Alarm_State(Ch) == 0))
        {
            Notice_Lcd_State(RERTURN_IDLE,Ch);
            
            if(Get_Card_Read_Type() != CARD_NONE)
            {//等待用户刷卡启动


                //int Page = PAGE_ID_MAIN_0;//Get_Page_ID(Ch);
                if(Get_Car_Link(Ch) != LEVEL_12_V)
                {
                    CardRet = API_Start_Charge_Card(&CardData,Ch);
                }
                if(OPT_ERR != CardRet)
                {
                    //Notice_Lcd_State(CTRL_BAKE_LIGHT,Ch);
                }
                if(CardRet == OPT_OK)
                {
                    //stMode_Info = Get_Charge_Mode_Info(Ch);

                    g_Charge_Ctrl_Manager[Ch].StateStamp = time(NULL);//有刷卡刷新状态时间戳

                    if(stMode_Info.ModeSetFlag == 0xff)
                    {//判断触摸屏那边是否是自定义充电等方式
                        time_t cur_sec = time(NULL);
                        struct tm *ptime = gmtime(&cur_sec);

                        g_Charge_Ctrl_Manager[Ch].ChargeMode = stMode_Info.SelectMode;
                        g_Charge_Ctrl_Manager[Ch].ChargeModeVal = stMode_Info.Para;

                        ptime->tm_hour = stMode_Info.Booktime[0];
                        ptime->tm_min = stMode_Info.Booktime[1];
                        ptime->tm_sec = 0;
                        g_Charge_Ctrl_Manager[Ch].BookTime = mktime(ptime) - 8 * 3600;/*转化为本地时间*/
                        //g_Charge_Ctrl_Manager[Ch].BookTime = (stMode_Info.Booktime[0]<<8)+stMode_Info.Booktime[1];
                        dbg_printf(1,"curtime %d BookTime %d",cur_sec,g_Charge_Ctrl_Manager[Ch].BookTime);
                        if(cur_sec > g_Charge_Ctrl_Manager[Ch].BookTime)
                        {//于次日启动
                            g_Charge_Ctrl_Manager[Ch].BookTime += 24 * 3600;
                            dbg_printf(1,"adjust curtime %d BookTime %d",cur_sec,g_Charge_Ctrl_Manager[Ch].BookTime);
                        }
                        if(g_Charge_Ctrl_Manager[Ch].ChargeMode == TIME_MODE)
                        {
                            g_Charge_Ctrl_Manager[Ch].ChargeModeVal *= 60;//转化为秒
                        }
                    }
                    else
                    {//
                        g_Charge_Ctrl_Manager[Ch].ChargeMode = AUTO_MODE;
                    }
                    if(CardData.CardType != ONLINE_CARD)
                    {
                        Notice_Beep(ONCE_RING,Ch);
                        if(Get_LocKEn())
                        {
                            Notice_Ctrl_Lock(UNLOCK_GUN,Ch);//开锁
                        }
                        g_Charge_Ctrl_Manager[Ch].StateStamp = time(NULL);
                        g_Charge_Ctrl_Manager[Ch].CardData = CardData;
                        if(g_Charge_Ctrl_Manager[Ch].ChargeMode == BOOK_MODE)
                        {//预约充电

                            g_Charge_Ctrl_Manager[Ch].ChargeType = BY_CARD_BOOK;
                            //预约成功
                            if(CP == LEVEL_9_V || CP == LEVEL_6_V)
                            {
                                Notice_Lcd_State(BOOK_SUCCES,Ch);
                                State_Switch(STATE_IDLE,STATE_BOOK,Ch);
                            }
                            else
                            {//提示插枪
                                Notice_Lcd_State(INSERT_GUN,Ch);
                            }

                        }
                        else
                        {//正常启动充电
                            g_Charge_Ctrl_Manager[Ch].ChargeType = BY_CARD_START;
                            dbg_printf(DBG_INFO,"开始刷卡启动...");

                        }

                        Creat_Start_Charge_Record(Ch);//创建一条开始的记录

                    }
                    else
                    {   //在线卡认证
//                         if(Get_Cloud_Comm_Statu() == e_LOGIN_OUT
//                             && g_ModbusCom == FALSE)
//                         {
//                             Notice_Lcd_State(OFF_LINE,Ch);
//
//                         }
//                         else
//                         {
//                            MSG_Def Msg = {0};

//                            Notice_Beep(ONCE_RING,Ch);
//                            //认证
//                            Notice_Lcd_State(AUTH_CARD,Ch);

//                            Msg.type = MSG_CLOUD_AUTH_FIRST_CARD;

//                            Msg.Ch = Ch;
//
//                            memcpy(Msg.data,CardData.Card_no,16);

//                            MsgQueue_Send(MSG_QUEUE_TASK_CLOUD_COMM,Msg);
//                            g_Charge_Ctrl_Manager[Ch].CardData = CardData;
//                            g_Charge_Ctrl_Manager[Ch].ChargeType = BY_ON_LINE_AUTH;
//                            //add
//                            memcpy(st_UsrAccount.AccountNo,CardData.Card_no,16);
//                         }

                    }
                }
                else if(OPT_LOCK == CardRet)
                {//补扣成功
                    ChargeRecord ChargeRecordData = {0};
                    uint8 cardNo[16] = {0};
                    shift_card_no(cardNo,CardData.Card_no,8);
                    if(Get_Un_Pay_UsrInfo(&ChargeRecordData,cardNo))
                    {
                        CARD_RECORD cardRecord = {0};
                        cardRecord.EndTime = ChargeRecordData.EndTime;
                        cardRecord.startTime = ChargeRecordData.StartTime;
                        cardRecord.Energe = ChargeRecordData.EndEnerge - ChargeRecordData.StartEnerge;
                        cardRecord.Money = ChargeRecordData.totalFee;//扣除总费用
                        CardRet = API_Add_Charge_Card(cardRecord,Ch);
                        dbg_printf(DBG_CHARGE,"补扣 余额%d 金额%d",ChargeRecordData.Balance,ChargeRecordData.totalFee);
                        if(OPT_OK == CardRet)
                        {
                            memcpy(&gChargeRecord[Ch],(uint8*)&ChargeRecordData,sizeof(ChargeRecord));
                            Notice_Lcd_State(CONSUM_INFO,Ch);
                            dbg_printf(DBG_CHARGE,"补扣成功");
                            osDelay(3000);
                            Notice_Lcd_State(RERTURN_IDLE,Ch);
                            Del_Un_Pay_UsrInfo(&ChargeRecordData);
                        }
                    }
                    else
                    {
                        Notice_Lcd_State(CARD_NOT_ACCOUNT,Ch);
                        dbg_printf(DBG_INFO,"卡被锁定");
                    }
                }
                else if(OPT_BALANCE_LACK == CardRet)
                {
                    Notice_Lcd_State(CARD_NO_MONEY,Ch);
                    dbg_printf(1,"State_Idle 卡余额不足");
                }
            }
        }
        else
        {
            //故障的时候写卡模式
            if(Get_FaultBit(Ch) & (1LL<<ALARM_REASON_CARD)
                || (Get_Card_Read_Type() != CARD_NONE))
            {//故障时候检测卡
                M2_Set_ReadWrite_Mode();
                API_Search_Card(&CardData);
            }

            //Notice_Lcd_State(RERTURN_IDLE,Ch);
        }

    }
    else
    {//标示已经有刷卡动作
        if(g_Charge_Ctrl_Manager[Ch].ChargeType == CHARGR_OPT_TIME_OUT)
        {
             Give_UP_Charge_Proc(Ch);//充电超时处理流程
             g_Charge_Ctrl_Manager[Ch].ChargeMode = AUTO_MODE;//清除
        }
    }
    //

    if(g_Charge_Ctrl_Manager[Ch].ChargeType == BY_ON_LINE_AUTH)
    {
        if(time(NULL) - g_Charge_Ctrl_Manager[Ch].StateStamp > 20)//
        {
            g_Charge_Ctrl_Manager[Ch].ChargeType = CHARGE_OPT_NULL;

            Notice_Lcd_State(RERTURN_IDLE,Ch);

            dbg_printf(DBG_INFO,"验证超时");
        }
    }

    if(CP == LEVEL_9_V)
    {
        if(g_Charge_Ctrl_Manager[Ch].ChargeType == BY_APP_START
            || g_Charge_Ctrl_Manager[Ch].ChargeType == BY_CARD_START
            ||g_Charge_Ctrl_Manager[Ch].ChargeType == BY_ON_LINE_AUTH)
        {

            if(time(NULL) - g_Charge_Ctrl_Manager[Ch].StateStamp < 20)//
            {
                if(Tick_Out(&NoticeSec[Ch],3000))
                {
                    BSP_Pwm_Out(Pwm,Ch);//PWM是什么值  wangyu q
                    dbg_printf(1,"BSP_Pwm_Out BSP_Pwm_Out");
                    //Notice_Lcd_State(READY_CHARGE,Ch);
                    if (g_Dwin_data.control_page != conuntdown)
                    {
                      Dwin_Lcd_State(conuntdown,Ch);
                      g_Dwin_data.control_page = conuntdown;
                    }
                    Tick_Out(&NoticeSec[Ch],0);
                }
            }
            else
            {//超时

                Notice_Lcd_State(CHARGE_FAIL,Ch);//
                dbg_printf(1,"启动超时 CHARGE_FAIL");
                if(g_Charge_Ctrl_Manager[Ch].ChargeContinu == TRUE)
                {//如果是继续充电 启动失败则跳转到结束流程
                    //Notice_Lcd_State(CHARGE_STOP_EXCEP,Ch);
                    Notice_Lcd_State(finished,Ch);
                    State_Switch(STATE_CHARGEING,STATE_END_CHARGE,Ch);
                }
                else
                {
                    if(g_Charge_Ctrl_Manager[Ch].ChargeType == BY_APP_START)
                    {
                        MSG_Def Msg = {0};
                        Msg.type = JWT_APP_start_fail;
                        Msg.Ch = Ch;
                        MsgQueue_Send(MSG_QUEUE_TASK_CLOUD_COMM,Msg);

                    }
                    g_Charge_Ctrl_Manager[Ch].ChargeType = CHARGR_OPT_TIME_OUT;
                }
            }
        }
        else if(g_Charge_Ctrl_Manager[Ch].ChargeType == BY_CARD_BOOK)
        {//预约成功
            Notice_Lcd_State(BOOK_SUCCES,Ch);
            State_Switch(STATE_IDLE,STATE_BOOK,Ch);
        }

    }
    else if( CP == LEVEL_12_V)
    {
        if(g_Charge_Ctrl_Manager[Ch].ChargeType == BY_APP_START
        || g_Charge_Ctrl_Manager[Ch].ChargeType == BY_CARD_START
        || g_Charge_Ctrl_Manager[Ch].ChargeType == BY_ON_LINE_AUTH
        || g_Charge_Ctrl_Manager[Ch].ChargeType == BY_CARD_BOOK )
        {
            //充电启动过程中拔枪
            if(time(NULL) - g_Charge_Ctrl_Manager[Ch].StateStamp < OPT_TIME_OUT)//一分钟等待
            {
                if(Tick_Out(&NoticeSec[Ch],3000))
                {
                    Notice_Lcd_State(INSERT_GUN,Ch);
                    Tick_Out(&NoticeSec[Ch],0);
                }
            }
            else
            {//超时
                //
                Notice_Lcd_State(CHARGE_FAIL,Ch);//
                if(g_Charge_Ctrl_Manager[Ch].ChargeContinu == TRUE)
                {//如果是继续充电 启动失败则跳转到结束流程
                    Notice_Lcd_State(CHARGE_STOP_EXCEP,Ch);
                    State_Switch(STATE_CHARGEING,STATE_END_CHARGE,Ch);
                }
                else
                {
                    if(g_Charge_Ctrl_Manager[Ch].ChargeType == BY_APP_START)
                    {
                        MSG_Def Msg = {0};
                        Msg.type = JWT_APP_start_fail;
                        Msg.Ch = Ch;
                        MsgQueue_Send(MSG_QUEUE_TASK_CLOUD_COMM,Msg);

                    }
                    g_Charge_Ctrl_Manager[Ch].ChargeType = CHARGR_OPT_TIME_OUT;
                }
            }
        }
    }
    //6v启动
    if(CP == LEVEL_6_V)
    {
        if((g_Charge_Ctrl_Manager[Ch].ChargeType == BY_APP_START
            ||g_Charge_Ctrl_Manager[Ch].ChargeType == BY_CARD_START))
        {
            BSP_Pwm_Out(Pwm,Ch);

            if(readySec[Ch] == 0)
            {
                readySec[Ch] = time(NULL);
            }
            else
            {
                if(time(NULL) - readySec[Ch] >= 2)
                {
                    dbg_printf(DBG_INFO,"就绪进入充电");
                    readySec[Ch] = 0;
                    State_Switch(STATE_IDLE,STATE_CHARGEING,Ch);

                    Notice_Lcd_State(CHARGEING,Ch);

                    Notice_Switch(SWITCH_CLOSE,Ch);//闭合

                    if(g_Charge_Ctrl_Manager[Ch].ChargeType == BY_APP_START)
                    {
                        MSG_Def Msg = {0};
                        Msg.type = JWT_APP_start_success;
                        Msg.Ch = Ch;
                        MsgQueue_Send(MSG_QUEUE_TASK_CLOUD_COMM,Msg);

                    }
                }
            }
        }
        else if(g_Charge_Ctrl_Manager[Ch].ChargeType == BY_CARD_BOOK)
        {//预约成功
            Notice_Lcd_State(BOOK_SUCCES,Ch);
            State_Switch(STATE_IDLE,STATE_BOOK,Ch);
        }
    }
    else
    {
        readySec[Ch] = 0;

        if(CP == LEVEL_12_V)
        {
            BSP_Pwm_Out(100,Ch);
        }

    }

}

/*********************************************************************
描    述 : 充电前消息处理函数
输入参数 :
          Msg 消息结构体
输出参数 :无
返    回 :无

作    者 :

创建时间 :18-06-07
**********************************************************************/

void State_Charge_Msg_Deal(MSG_Def *Msg)
{

    uint8 MsgType = Msg->type;
    uint8 Ch = Msg->Ch;

    switch(MsgType)
    {
        case CTRL_STOP_CHARGE:
        {
            if(Msg->data[0] == STOP_OFF_LINE ||
                Msg->data[0] == STOP_CANCEL_CHARGE
                ||Msg->data[0] == STOP_REASON_MANNAL_STOP)
            {
                g_Charge_Ctrl_Manager[Ch].ChargeType = BY_APP_STOP;
                gChargeRecord[Ch].StopReason = Msg->data[0];
            }
            else
            {
                gChargeRecord[Ch].StopReason = STOP_REASON_MANNAL_STOP;
                g_Charge_Ctrl_Manager[Ch].ChargeType = BY_OTEHER_STOP;

            }

            dbg_printf(DBG_INFO," 充电中收到停止命令 %d ",Msg->data[0]);

            break;
        }

        default:
            break;
    }
}

/*********************************************************************
描    述 : 充电前消息处理函数
输入参数 :
           Alarm 告警当前编码
输出参数 :无
返    回 :无

作    者 :

创建时间 :18-06-07
**********************************************************************/
static uint8 Shift_Alarm_Stop_Reason(uint64 Alarm)
{
    int i = 0;

    for(i = 0; i< 64;i++)
    {
        if((1LL << i) & Alarm)
        {
            return i;
        }
    }

    return 0;
}
/*********************************************************************
描    述 : 判断达到设定的条件
输入参数 :
          CH 通道
输出参数 :无
返    回 :无

作    者 :

创建时间 :18-06-07
**********************************************************************/
static uint8 Check_Reach_Seting(uint8 Ch)
{
    int i = 0;

     //判断金额模式
    if((gChargeRecord[i].totalFee + MIN_MONEY) >= g_Charge_Ctrl_Manager[Ch].CardData.Balance
        &&(g_Charge_Ctrl_Manager[Ch].CardData.CardType != SUPER_MANAGE))
    {
        if(gChargeRecord[i].totalFee >= g_Charge_Ctrl_Manager[Ch].CardData.Balance)
        {
            gChargeRecord[i].totalFee = g_Charge_Ctrl_Manager[Ch].CardData.Balance;
        }

        return STOP_BALANCE_LACK;
    }
    //判断最大充电时间
    if(gChargeRecord[i].EndTime < gChargeRecord[i].StartTime)
    {//时间有更新
        gChargeRecord[i].StartTime = time(NULL) - gChargeRecord[Ch].ChargeTime;
    }

    if(gChargeRecord[i].EndTime  - gChargeRecord[i].StartTime >= MAX_CHARGE_TIME)
    {
       return STOP_MAX_CHARGE_TIME;
    }

    if(g_Charge_Ctrl_Manager[Ch].ChargeMode == AUTO_MODE)
    {
        return 0;
    }
    else if(g_Charge_Ctrl_Manager[Ch].ChargeMode == TIME_MODE)
    {//判断时间模式

        if((gChargeRecord[i].EndTime  - gChargeRecord[i].StartTime) >= g_Charge_Ctrl_Manager[Ch].ChargeModeVal)
        {
            return STOP_REASON_REACH_SET_STOP;
        }
    }
    else if(g_Charge_Ctrl_Manager[Ch].ChargeMode == ENERGE_MODE)
    {//判断电量模式
        if(gChargeRecord[i].EndEnerge - gChargeRecord[i].StartEnerge >= g_Charge_Ctrl_Manager[Ch].ChargeModeVal)
        {
            return STOP_REASON_REACH_SET_STOP;
        }
    }
    else if(g_Charge_Ctrl_Manager[Ch].ChargeMode == MONEY_MODE)
    {//判断金额模式
        if(gChargeRecord[i].totalFee >= g_Charge_Ctrl_Manager[Ch].ChargeModeVal)
        {
			      gChargeRecord[i].totalFee = g_Charge_Ctrl_Manager[Ch].ChargeModeVal;//停止的时候限制一下
            return STOP_REASON_REACH_SET_STOP;
        }
    }


    return 0;
}


// 每10S 保存一次电流
void save_10s_current(int Ch)
{
	MEASURE_DATA MeterData = {0};
	MeterData = Get_Measrue_Data(0);
	int current_time = 0;//计算10s
	int count = 0;
    static int last_time = 0;

	if(last_time == time(NULL))
    {
    	return;
    }
	else
    {
    	last_time = time(NULL);
    }

	if((gChargeRecord[0].ChargeTime % 10) == 0)
    {
    	count = (gChargeRecord[0].ChargeTime / 10) % 10;
		get_current[count] = MeterData.CurrentA;

		dbg_printf(DBG_INFO," get_current[%d] = %d ",count,get_current[count]);
	}

}
/*

	健网科技 -- JWT

					*/

//获取 10S 前的电流
int  Get_10S_Charging_Current(uint8 Ch)
{
	int count = 0;
	
	int current = 0;
	
	count = (gChargeRecord[0].ChargeTime / 10) % 10;

	if(count == 0)
	{
		current = get_current[9];
	}
	else
	{
		current = get_current[count - 1];
		dbg_printf(1," current = %d ",current);
	}

	return current;
}


/*********************************************************************
描    述 : 充电控制任务状态机函数
输入参数 :State 状态机
          Ch 通道
          MsgFlag 收到消息标记
          Msg 消息结构体
输出参数 :无
返    回 :无

作    者 :

创建时间 :18-06-07
**********************************************************************/

static void State_Chargeing(uint8 Ch,uint8 MsgFlag,MSG_Def *Msg)
{
    static int num_cnt = 0;

    uint64 Fault = 0;           //故障标记
    uint8 Seting_ret = 0;       //判断设定的结果
    uint8 Stop = 0;             //停止标记
    int Balance = 0;
    CARD_RECORD cardRecord = {0};
    END_OPT_CARD Opt;
    uint8 CP = Get_Car_Link(Ch);
    gChargeRecord[Ch].StopReason = 0;
    //static uint32_t min_current_tick = 0;    
		MEASURE_DATA MeterData = {0};
		MeterData = Get_Measrue_Data(0);

	  //每10s保存一次电流
	  save_10s_current(Ch);

		if(CP == LEVEL_6_V)
		{
				Notice_Switch(SWITCH_CLOSE,Ch);
        g_Charge_Ctrl_Manager[Ch].pause_charge = FALSE; 

        num_cnt = 0;
		}
    //是否启用十分钟小电流跳枪
    #if Min_curr_10min_enable
	  if(MeterData.CurrentA < 100 )
	  {
	  		if(g_Charge_Ctrl_Manager[Ch].current_1A_tick == 0)
	  		{
	  			g_Charge_Ctrl_Manager[Ch].current_1A_tick = time(NULL);
	  		}
				else
			  {
          int delay_time = 60 * 10;
       
			  	if(time(NULL) - g_Charge_Ctrl_Manager[Ch].current_1A_tick > delay_time)
			    {
							gChargeRecord[Ch].StopReason = STOP_REASON_CURR_LOW_TIMEOUT;
							dbg_printf(1," curr_min_1A stop ");	
							Stop = TRUE;
							Notice_Switch(SWITCH_DISCONNECT,Ch);//通知断开
			    }
			  }
	  }
		else
	  {
	  		g_Charge_Ctrl_Manager[Ch].current_1A_tick = 0;
	  }
    #endif
    if(MsgFlag == TRUE)
    {//处理本状态的消息
        State_Charge_Msg_Deal(Msg);
    }

    Fault = Get_Alarm_State(Ch);
    Seting_ret = Check_Reach_Seting(Ch);

    if(g_Charge_Ctrl_Manager[Ch].ChargeType == BY_CARD_START)
    {
        uint64 Bit = Get_FaultBit(Ch);
        //读卡器停止
        if(Bit & (1LL<<ALARM_REASON_CARD))
        {
            Fault = TRUE;
        }
        if(time(NULL) - g_Charge_Ctrl_Manager[Ch].StateStamp > CARD_TIME_GAP)
        {//防止立即停止影响刷卡的用户体验
            cardRecord.EndTime = gChargeRecord[Ch].EndTime;
            cardRecord.startTime = gChargeRecord[Ch].StartTime;
            cardRecord.Energe = gChargeRecord[Ch].EndEnerge - gChargeRecord[Ch].StartEnerge;
            cardRecord.Money = gChargeRecord[Ch].totalFee;
            cardRecord.type = gChargeRecord[Ch].StartCardType;
            Opt = API_End_Charge_Card(cardRecord,&Balance,Ch);
            //卡停止
            if(Opt == END_OK)
            {
                g_Charge_Ctrl_Manager[Ch].ChargeType = BY_CARD_STOP;
                g_Charge_Ctrl_Manager[Ch].CardData.Balance = Balance;
                gChargeRecord[Ch].Balance = Balance;//更新用户余额
                Notice_Beep(ONCE_RING,Ch);

                Notice_Lcd_State(CONSUM_INFO,Ch);

                Stop = TRUE;
            }
            else if(Opt == END_NOT_MATCH)
            {
                Notice_Beep(ONCE_RING,Ch);

                Notice_Lcd_State(END_CARD_NOT_MATCH,Ch);
                dbg_printf(1,"卡不匹配");
            }
        }

    }

    if(Fault != 0)
    {
        Fault = Get_FaultBit(Ch);
        gChargeRecord[Ch].StopReason = Fault;//Shift_Alarm_Stop_Reason(Fault);
        dbg_printf(1,"fault stop %d ",Fault);
        BSP_Pwm_Out(100,Ch);//充电机停止时 先 pwm 停止
        Notice_Switch(SWITCH_DISCON_IM,Ch);//立即断开

        Stop = TRUE;
    }
    else if(Seting_ret)
    {
        //充电机停止时 先 pwm 停止
        gChargeRecord[Ch].StopReason = Seting_ret;
        BSP_Pwm_Out(100,Ch);

        Notice_Switch(SWITCH_DISCONNECT,Ch);//断开
        Stop = TRUE;
        
        dbg_printf(1,"Seting_ret stop %d",Seting_ret);
    }
    else if(CP == LEVEL_9_V)
    {
	
				Notice_Switch(SWITCH_DISCON_IM,Ch);//断开
				if(num_cnt == 0)
        {    
				  dbg_printf(1," 9V stop switch %d",Get_Car_Link(Ch));	
          num_cnt = 1;
        }
        
				//Stop = TRUE;
				//gChargeRecord[Ch].StopReason = STOP_REASON_CAR_STOP;
				//dbg_printf(1," full stop ");
        g_Charge_Ctrl_Manager[Ch].pause_charge = TRUE; 
    }
		else if(Get_Car_Link(Ch) == LEVEL_12_V)
		{
				Notice_Switch(SWITCH_DISCON_IM,Ch);//断开
				Stop = TRUE;
				gChargeRecord[Ch].StopReason = STOP_REASON_CHARGE_DRAW_GUN_STOP;
				dbg_printf(1," 充电中拔枪 ");
		}
		

    //10.28   else if改为if
    if(g_Charge_Ctrl_Manager[Ch].ChargeType == BY_APP_STOP
        || g_Charge_Ctrl_Manager[Ch].ChargeType == BY_OTEHER_STOP
        || g_Charge_Ctrl_Manager[Ch].ChargeType == BY_CARD_STOP)
    {

        BSP_Pwm_Out(100,Ch);
        Notice_Switch(SWITCH_DISCONNECT,Ch);//断开

        Stop = TRUE;
        dbg_printf(1,"app or card stop %d");
    }

    if(Stop)
    {
        if(Fault != 0)
        {
            Notice_Lcd_State(CHARGE_STOP_EXCEP,Ch);
        }
        else if(g_Charge_Ctrl_Manager[Ch].ChargeType != BY_CARD_STOP)
        {
            Notice_Lcd_State(CHARGE_STOP,Ch);
        }
        //g_Charge_Ctrl_Manager[Ch].ChargeType = BY_OTEHER_STOP;
        g_Charge_Ctrl_Manager[Ch].pause_charge = FALSE; 
        State_Switch(STATE_CHARGEING,STATE_END_CHARGE,Ch);

        dbg_printf(1,"stop reason %d",gChargeRecord[Ch].StopReason);
    }
}


/*********************************************************************
描    述 : 充电前消息处理函数
输入参数 :
          Msg 消息结构体
输出参数 :无
返    回 :无

作    者 :

创建时间 :18-06-07
**********************************************************************/

void State_Charge_Stop_Msg_Deal(MSG_Def *Msg)
{

    uint8 MsgType = Msg->type;
    uint8 Ch = Msg->Ch;

    switch(MsgType)
    {

        case CTRL_CONTUNE_CHARGE:
        {
            g_Charge_Ctrl_Manager[Ch].ChargeContinu = TRUE;
            dbg_printf(1,"继续充电");
            //g_Charge_Ctrl_Manager[Ch].ChargeType = BY_APP_START;
            g_Charge_Ctrl_Manager[Ch].StateStamp = time(NULL);//记录状态变化
            State_Switch(STATE_END_CHARGE,STATE_IDLE,Ch);
			
            break;
        }
        case CTRL_STOP_CHARGE:
        {
            if(Msg->data[0] == STOP_OFF_LINE ||
                Msg->data[0] == STOP_CANCEL_CHARGE

                ||Msg->data[0] == STOP_REASON_MANNAL_STOP)
            {
                g_Charge_Ctrl_Manager[Ch].ChargeType = BY_APP_STOP;
                gChargeRecord[Ch].StopReason = Msg->data[0];
            }
            else
            {
                gChargeRecord[Ch].StopReason = STOP_REASON_MANNAL_STOP;
                g_Charge_Ctrl_Manager[Ch].ChargeType = BY_OTEHER_STOP;

            }

            dbg_printf(DBG_INFO," 充电中收到停止命令 %d ",Msg->data[0]);

            break;
        }
        case CTRL_BOOK_CHARGE:
        {
            if( g_Charge_Ctrl_Manager[Ch].ChargeType != BY_CARD_START)
            {
                State_Switch(STATE_IDLE,STATE_BOOK,Ch);
            }

            break;
        }
        default :break;
    }
}

/*********************************************************************
描    述 : 充电控制任务状态机函数
输入参数 :State 状态机
          Ch 通道
          MsgFlag 收到消息标记
          Msg 消息结构体
输出参数 :无
返    回 :无

作    者 :

创建时间 :18-06-07
**********************************************************************/

static void State_Charge_Stop(uint8 Ch,uint8 MsgFlag,MSG_Def *Msg)
{

//    uint8 Ret = 0;
    static int stop_cnt = 0;
    static int stop_cnt_1 = 0;

    uint64 Fault = 0;           //故障标记
    static uint32 timer = 0;
    static uint32 last_cp_val[2] = {0};
    uint8 CP = Get_Car_Link(Ch);
    int Balance = 0;
		uint8 report_num = 0;
    if(last_cp_val[Ch] != CP)
    {
        last_cp_val[Ch] = CP;

        g_Charge_Ctrl_Manager[Ch].StateStamp = time(NULL);//记录状态变化
    }
//
//    if(Ch == 0)
//    {
//        Ret = BSP_Read_InPut(KM1_DETECT);
//    }
//    else
//    {
//        Ret = BSP_Read_InPut(KM2_DETECT);
//    }
    Fault = Get_Alarm_State(Ch);

    if(CP == LEVEL_12_V || Fault != 0)
    {//断开后再关PWM
       BSP_Pwm_Out(100,Ch);//关PWM
       if(stop_cnt == 0)
        {    
				  dbg_printf(1,"LEVEL_12_V close Pwm 0");	
          stop_cnt = 1;
        }
       
    }

    if(MsgFlag == TRUE)
    {//处理本状态的消息
        State_Charge_Stop_Msg_Deal(Msg);
        return;
    }

    //判断启动方式
    if(g_Charge_Ctrl_Manager[Ch].ChargeType == BY_CARD_START)
    {//用卡启动的
        CARD_RECORD cardRecord = {0};
        END_OPT_CARD Opt;
        cardRecord.EndTime = gChargeRecord[Ch].EndTime;
        cardRecord.startTime = gChargeRecord[Ch].StartTime;
        cardRecord.Energe = gChargeRecord[Ch].EndEnerge - gChargeRecord[Ch].StartEnerge;
        cardRecord.Money = gChargeRecord[Ch].totalFee;
        cardRecord.type = gChargeRecord[Ch].StartCardType;
        Opt = API_End_Charge_Card(cardRecord,&Balance,Ch);

        if(Opt == END_OK)
        {
            g_Charge_Ctrl_Manager[Ch].ChargeType = BY_CARD_STOP;
            g_Charge_Ctrl_Manager[Ch].StateStamp = time(NULL);//记录状态变化
            g_Charge_Ctrl_Manager[Ch].CardData.Balance =  Balance;
            Notice_Beep(ONCE_RING,Ch);

            Notice_Lcd_State(CONSUM_INFO,Ch);

            gChargeRecord[Ch].PayFlag = PAY_YET;//已经付费
        }
        else if(Opt == END_NOT_MATCH)
        {
            Notice_Lcd_State(END_CARD_NOT_MATCH,Ch);
        }

    }


    //新增恢复到待机
    if(g_Charge_Ctrl_Manager[Ch].ChargeType == BY_APP_STOP
        || g_Charge_Ctrl_Manager[Ch].ChargeType == BY_CARD_STOP
        || g_Charge_Ctrl_Manager[Ch].ChargeType == BY_APP_START
        || g_Charge_Ctrl_Manager[Ch].ChargeType == BY_OTEHER_STOP)
    {
        int retFlag = FALSE;

        if(CP == LEVEL_12_V)
        {//拔枪则恢复
            //dbg_printf(1,"回到待机0");
            retFlag = TRUE;
        }

        if(g_Charge_Ctrl_Manager[Ch].ChargeType == BY_APP_STOP
        || g_Charge_Ctrl_Manager[Ch].ChargeType == BY_CARD_STOP
        || g_Charge_Ctrl_Manager[Ch].ChargeType == BY_OTEHER_STOP)

        {
            //dbg_printf(1,"回到待机1");
            retFlag = TRUE;
        }

        if(g_Charge_Ctrl_Manager[Ch].ChargeType == BY_APP_START)
        {
            if(g_Charge_Ctrl_Manager[Ch].ChargeContinu == TRUE)
            {//如果是继续充电结束的 则进入待机
                retFlag = TRUE;
            }

        }

        if(retFlag == TRUE)
        {
            if(Get_LocKEn())
            {
                if(Get_Lock_Seat(Ch) == TRUE
                    && CP == LEVEL_12_V)
                {
                    Notice_Ctrl_Lock(LOCK_GUN,Ch);//锁定
                    g_Charge_Ctrl_Manager[Ch].ChargeType = CHARGE_OPT_NULL;
                    g_Charge_Ctrl_Manager[Ch].ChargeMode = AUTO_MODE;
                    gChargeRecord[Ch].PayFlag = PAY_YET;//已经付费
                    Notice_Lcd_State(RERTURN_IDLE,Ch);
                    dbg_printf(1,"回到待机3");
                    g_Charge_Ctrl_Manager[Ch].ChargeContinu = FALSE;
                    State_Switch(STATE_END_CHARGE,STATE_IDLE,Ch);

					
                }


            }
            else
            {
                if(time(NULL) - g_Charge_Ctrl_Manager[Ch].StateStamp > CONSUM_TIME_GAP)
                {

                    g_Charge_Ctrl_Manager[Ch].ChargeType = CHARGE_OPT_NULL;
                    g_Charge_Ctrl_Manager[Ch].ChargeMode = AUTO_MODE;
                    gChargeRecord[Ch].PayFlag = PAY_YET;//已经付费

                    dbg_printf(1,"turn to idle 4");
                    g_Charge_Ctrl_Manager[Ch].ChargeContinu = FALSE;
                    State_Switch(STATE_END_CHARGE,STATE_IDLE,Ch);
                    osDelay(CONSUM_TIME_GAP*1000);//停留时间
                    Notice_Lcd_State(RERTURN_IDLE,Ch);

                }
            }
        }
    }
    //拔枪之后恢复
    if(CP == LEVEL_12_V)
    {
        g_Charge_Ctrl_Manager[Ch].continue_flag = FALSE;
        BSP_Pwm_Out(100,Ch);
        if(stop_cnt_1 == 0)
        {    
				  dbg_printf(1,"LEVEL_12_V close Pwm 1");	
          stop_cnt_1 = 1;
        }
        if(time(NULL) - g_Charge_Ctrl_Manager[Ch].StateStamp > 180)//停止充电后3min未进行刷卡
        {
            if(g_Charge_Ctrl_Manager[Ch].ChargeType == BY_CARD_START
                &&((g_Charge_Ctrl_Manager[Ch].CardData.CardType == BUSINESS_CARD
                    ||g_Charge_Ctrl_Manager[Ch].CardData.CardType == USR_CARD)))
            {
                Save_UpPay_Info(Ch);
                dbg_printf(1,"保存异常刷卡信息");
            }

            g_Charge_Ctrl_Manager[Ch].ChargeType = CHARGE_OPT_NULL;
            g_Charge_Ctrl_Manager[Ch].ChargeMode = AUTO_MODE;

            //Notice_Lcd_State(disconnected,Ch);
            if(Get_LocKEn())
            {
                Notice_Ctrl_Lock(LOCK_GUN,Ch);//锁定
            }
            dbg_printf(1,"turn to idle 5");
            g_Charge_Ctrl_Manager[Ch].ChargeContinu = FALSE;
            State_Switch(STATE_END_CHARGE,STATE_IDLE,Ch);

            //add
            stop_cnt = 0;
            stop_cnt_1 = 0;
        }

    }

		#if 0
    if(gChargeRecord[Ch].StopReason == STOP_REASON_CAR_STOP
      && (CP == LEVEL_6_V) && g_Charge_Ctrl_Manager[Ch].continue_flag == TRUE)
    {
        gChargeRecord[Ch].StopReason = 0;
        //Ctrl_Continu_Charge(Ch);
        dbg_printf(1,"9v->6V continue charge");
    }
		#endif
		
}

/*********************************************************************
描    述 : 预约状态下消息处理函数
输入参数 :
          Msg 消息结构体
输出参数 :无
返    回 :无

作    者 :

创建时间 :18-06-07
**********************************************************************/

void State_Charge_Book_Msg_Deal(MSG_Def *Msg)
{

    uint8 MsgType = Msg->type;
    uint8 Ch = Msg->Ch;

    switch(MsgType)
    {
        case CTRL_START_CHARGE:
        {
            uint8 Pwm = Calculate_Pwm_Dutycycle();
            if(Get_Car_Link(Ch) == LEVEL_9_V
                ||Get_Car_Link(Ch) == LEVEL_6_V)
            {
                BSP_Pwm_Out(Pwm,Ch);

                //Notice_Lcd_State(conuntdown,Ch);

                g_Charge_Ctrl_Manager[Ch].ChargeType = BY_APP_START;
                State_Switch( STATE_BOOK,STATE_IDLE,Ch);
                Notice_Switch(SWITCH_CLOSE,Ch);//
                dbg_printf(DBG_INFO,"预约时间到开始充电");
            }
            else
            {//启动失败
                //MSG_Def Msg = {0};
                //Msg.type = MSG_CLOUD_START_CRG_FAIL;
                //Msg.Ch = Ch;
                //MsgQueue_Send(MSG_QUEUE_TASK_CLOUD_COMM,Msg);
                g_Charge_Ctrl_Manager[Ch].ChargeType = CHARGE_OPT_NULL;

                dbg_printf(DBG_INFO,"启动失败");
            }

            break;
        }
        case CTRL_BOOK_CANCEL:
        {
            g_Charge_Ctrl_Manager[Ch].ChargeType = CHARGE_OPT_NULL;

            Notice_Beep(ONCE_RING,Ch);

            Notice_Lcd_State(homepage,Ch);

            State_Switch( STATE_BOOK,STATE_IDLE,Ch);

            dbg_printf(DBG_INFO,"APP取消预约");

            Save_End_Charge_Opt_Info(Ch);
            break;
        }

        default:
            break;
    }
    //待扩展
}

static void State_Charge_Book(uint8 Ch,uint8 MsgFlag,MSG_Def *Msg)
{

//    uint8 CardRet = 0;
//    CARD_DATA CardData = {0};
//    struct tm *stTime = {0};
    time_t Sec = time(NULL);
    if(MsgFlag == TRUE)
    {//处理本状态的消息
        State_Charge_Book_Msg_Deal(Msg);
    }
    //时间到则开机启动
    if(g_Charge_Ctrl_Manager[Ch].ChargeType == BY_CARD_BOOK)
    {
        //stTime = gmtime(&Sec);
        if(Sec >= (g_Charge_Ctrl_Manager[Ch].BookTime))
        {
            uint8 Pwm = Calculate_Pwm_Dutycycle();

            if(Get_Alarm_State(Ch) == FALSE)                                    /*无故障直接启动，有故障就一直等*/
            {
                if(Get_Car_Link(Ch) == LEVEL_9_V
                    ||Get_Car_Link(Ch) == LEVEL_6_V)
                {
                    BSP_Pwm_Out(Pwm,Ch);
                    g_Charge_Ctrl_Manager[Ch].ChargeType = BY_CARD_START;
                    State_Switch( STATE_BOOK,STATE_IDLE,Ch);
                    dbg_printf(DBG_INFO,"预约时间到开始充电");
                }
            }
        }
        if(Get_Car_Link(Ch) == LEVEL_12_V)                                     /*等待拔枪直接取消*/
        {
            //存储未结算
            if(g_Charge_Ctrl_Manager[Ch].CardData.CardType == BUSINESS_CARD)
            {
                Save_UpPay_Info(Ch);
            }
            g_Charge_Ctrl_Manager[Ch].ChargeType = CHARGE_OPT_NULL;
            g_Charge_Ctrl_Manager[Ch].ChargeMode = AUTO_MODE;
            if(Get_LocKEn())
            {
                Notice_Ctrl_Lock(LOCK_GUN,Ch);//锁
            }

            Notice_Lcd_State(BOOK_CANCEL,Ch);
        }
    }
    else if(g_Charge_Ctrl_Manager[Ch].ChargeType == BY_APP_BOOK)
    {
        if(Sec >= (g_Charge_Ctrl_Manager[Ch].BookTime))
        {
            g_Charge_Ctrl_Manager[Ch].ChargeType = CHARGE_OPT_NULL;
            State_Switch( STATE_BOOK,STATE_IDLE,Ch);
            //Notice_Lcd_State(homepage,Ch);
            dbg_printf(DBG_INFO,"预约时间超");
            //
            if(Get_LocKEn())
            {
                Notice_Ctrl_Lock(LOCK_GUN,Ch);//锁
            }
        }
    }
    //取消预约的功能
    if(g_Charge_Ctrl_Manager[Ch].ChargeType == BY_CARD_BOOK)
    {
        CARD_RECORD cardRecord = {0};
        END_OPT_CARD Opt ;
        int balence = 0;
        cardRecord.EndTime = gChargeRecord[Ch].EndTime;
        cardRecord.startTime = gChargeRecord[Ch].StartTime;
        cardRecord.Energe = gChargeRecord[Ch].EndEnerge - gChargeRecord[Ch].StartEnerge;
        cardRecord.Money = gChargeRecord[Ch].totalFee;
        cardRecord.type = gChargeRecord[Ch].StartCardType;
        Opt = API_End_Charge_Card(cardRecord,&balence,Ch);

        if(Opt == END_OK)
        {
            g_Charge_Ctrl_Manager[Ch].ChargeType = CHARGE_OPT_NULL;
            g_Charge_Ctrl_Manager[Ch].ChargeMode = AUTO_MODE;
            Notice_Beep(ONCE_RING,Ch);

            Notice_Lcd_State(BOOK_CANCEL,Ch);

            dbg_printf(DBG_INFO,"刷卡取消预约");

            Save_End_Charge_Opt_Info(Ch);
        }
        else if(Opt == END_NOT_MATCH)
        {
            Notice_Lcd_State(END_CARD_NOT_MATCH,Ch);
            dbg_printf(DBG_INFO,"刷卡取消预约 卡不对");
        }
    }
    if(g_Charge_Ctrl_Manager[Ch].ChargeType == CHARGE_OPT_NULL)
    {//
        static uint32 Sec = 0;

        if(Sec == 0)
        {
            Sec = time(NULL);
        }
        else
        {
            if(time(NULL) - Sec > 5)
            {
                State_Switch( STATE_BOOK,STATE_IDLE,Ch);
                Sec = 0;
                dbg_printf(DBG_INFO,"取消预约回到待机");
            }
        }
    }
}

/********************************************************
描    述 : 提供给外部的接口 启动充电
输入参数 :CH 通道
输出参数 :无
返    回 :SYS_BUSY 忙 SYS_BOOK_ING预约中

作    者 :

创建时间 :18-06-07
*********************************************************/
int  Ctrl_power_on(uint8 Ch,UsrAccount *Account)
{

    //通知开始充电
    MSG_Def Msg;
    Msg.Ch =Ch;
    Msg.type = CTRL_START_CHARGE;

    if(g_Charge_Ctrl_Manager[Ch].CurState == STATE_CHARGEING
        || g_Charge_Ctrl_Manager[Ch].CurState == STATE_END_CHARGE)
    {
        //dbg_printf(DBG_INFO,"通知开机 busy");
        return SYS_BUSY;
    }
    else if(g_Charge_Ctrl_Manager[Ch].CurState == STATE_IDLE)
    {
        if(Account->Money <= MIN_ALLOW_MONEY)
        {
            return SYS_LACK_MONEY;
        }
        MsgQueue_Send(MSG_QUEUE_TASK_CHARGE_CTRL,Msg);
        g_Charge_Ctrl_Manager[Ch].CardData.Balance = Account->Money;
		g_Charge_Ctrl_Manager[Ch].ChargeMode = Account->CrgMode;        //充电策略
        g_Charge_Ctrl_Manager[Ch].ChargeModeVal = Account->CrgModeVal;//充电策略参数
		g_Charge_Ctrl_Manager[Ch].CardData.CardType = TYPE_APP_USR;//APP 用户
		g_Charge_Ctrl_Manager[Ch].ListId = Account->ListID;
        memcpy(g_Charge_Ctrl_Manager[Ch].CardData.Card_no,Account->AccountNo,sizeof(Account->AccountNo));
        dbg_printf(1,"通知开机 app list %d",Account->ListID); 
        
        return CTRL_OK;
    }
    else if(g_Charge_Ctrl_Manager[Ch].CurState == STATE_BOOK)
    {
        if(g_Charge_Ctrl_Manager[Ch].ChargeType == BY_CARD_BOOK)
        {
            return SYS_BUSY;
        }
        if(memcmp(Account->AccountNo,g_Charge_Ctrl_Manager[Ch].CardData.Card_no,sizeof(g_Charge_Ctrl_Manager[Ch].CardData.Card_no)) == 0)
        {
            MsgQueue_Send(MSG_QUEUE_TASK_CHARGE_CTRL,Msg);
            g_Charge_Ctrl_Manager[Ch].CardData.Balance = Account->Money;
            memcpy(g_Charge_Ctrl_Manager[Ch].CardData.Card_no,Account->AccountNo,sizeof(Account->AccountNo));
            //dbg_printf(DBG_INFO,"通知开机");

            return CTRL_OK;
        }
        else
        {
            return SYS_BOOK_ING;
        }
    }

    return SYS_BUSY;
}

int  Updata_UsrAccount(uint8 *CardNo,uint32 Money,int Ch)
{

    if(g_Charge_Ctrl_Manager[Ch].CurState == STATE_CHARGEING)
    {

        g_Charge_Ctrl_Manager[Ch].CardData.Balance = Money;
        memcpy(g_Charge_Ctrl_Manager[Ch].CardData.Card_no,CardNo,sizeof(g_Charge_Ctrl_Manager[Ch].CardData.Card_no));
        gChargeRecord[Ch].Balance = g_Charge_Ctrl_Manager[Ch].CardData.Balance;
        if(g_Charge_Ctrl_Manager[Ch].ChargeMode == MONEY_MODE)
        {
            g_Charge_Ctrl_Manager[Ch].ChargeModeVal = g_Charge_Ctrl_Manager[Ch].CardData.Balance;
        }

        dbg_printf(1,"Balance = %d",g_Charge_Ctrl_Manager[Ch].CardData.Balance );
        return TRUE;
    }

    return FALSE;
}

/********************************************************
描    述 : 继续充电
输入参数 :CH 通道
输出参数 :无
返    回 :

作    者 :

创建时间 :18-06-07
*********************************************************/
void Ctrl_Continu_Charge(uint8 Ch)
{

    //通知开始充电
    MSG_Def Msg;
    Msg.Ch =Ch;
    Msg.type = CTRL_CONTUNE_CHARGE;
    MsgQueue_Send(MSG_QUEUE_TASK_CHARGE_CTRL,Msg);

    dbg_printf(DBG_INFO,"通知开机");

}
/********************************************************
描    述 : 停止充电
输入参数 :CH 通道
输出参数 :无
返    回 :

作    者 :

创建时间 :18-06-07
*********************************************************/
void Ctrl_power_off(uint8 Reason,uint8 Ch)
{

    //通知停止充电
    MSG_Def Msg;
    Msg.Ch =Ch;
    Msg.type = CTRL_STOP_CHARGE;
    Msg.data[0] = Reason;

    MsgQueue_Send(MSG_QUEUE_TASK_CHARGE_CTRL,Msg);

    dbg_printf(DBG_INFO,"通知关机");

}

void Ctrl_Book_charge(uint32 Time,uint8* Usr,uint8 Ch)
{

    //
    MSG_Def Msg;
    Msg.Ch =Ch;
    Msg.type = CTRL_BOOK_CHARGE;

    g_Charge_Ctrl_Manager[Ch].BookTime = Time;
    memcpy(g_Charge_Ctrl_Manager[Ch].CardData.Card_no,Usr,16);
    MsgQueue_Send(MSG_QUEUE_TASK_CHARGE_CTRL,Msg);

    dbg_printf(DBG_INFO,"book start %d",Time);

}

void Ctrl_Cancel_Book_charge(uint8 Ch)
{

    //
    MSG_Def Msg;
    Msg.Ch =Ch;
    Msg.type = CTRL_BOOK_CANCEL;

    MsgQueue_Send(MSG_QUEUE_TASK_CHARGE_CTRL,Msg);

    dbg_printf(DBG_INFO,"book Cancel %d");

}

static void Resume_Power_Off(uint8 Ch)
{
    uint8 State = 0;
    uint16 iVal = 0;
    State = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR11);

    if(State == STATE_BOOK)
    {
        g_Charge_Ctrl_Manager[Ch].CurState = STATE_BOOK;
        g_Charge_Ctrl_Manager[Ch].LastState = STATE_BOOK;
        g_Charge_Ctrl_Manager[Ch].ChargeType = gChargeRecord[Ch].ChargeType;
        g_Charge_Ctrl_Manager[Ch].BookTime = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR9);
        iVal = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR10);
        g_Charge_Ctrl_Manager[Ch].BookTime += (iVal<<16);
        g_Charge_Ctrl_Manager[Ch].CardData.CardType = gChargeRecord[Ch].StartCardType;
        g_Charge_Ctrl_Manager[Ch].CardData.Balance = gChargeRecord[Ch].Balance;
        memcpy(g_Charge_Ctrl_Manager[Ch].CardData.Card_no,gChargeRecord[Ch].CardNo,sizeof(g_Charge_Ctrl_Manager[Ch].CardData.Card_no));
        Set_Charge_Card_No(gChargeRecord[Ch].CardNo,Ch);
        Notice_Lcd_State(BOOK_SUCCES,Ch);
        //判断充电枪？
        dbg_printf(1,"恢复到预约状态");
    }
}


void record_test(void)
{

//    int index = 0;
    ChargeRecord *pChargeRecordData = Get_ChargeRecord(0);

    static uint32 save = 0;
    static int reboot = 0;
    if(time(NULL) - save >= 1)
    {
//        Creat_Start_Charge_Record(0);
//
//        index = SPIFLASH_Method4_Get_RecordCnt(TYPE_CHARGELOG);
//        SPIFLASH_Method4_Save_Record(TYPE_CHARGELOG,index,(uint8*)pChargeRecordData);//旧数据读出来
//        dbg_printf(1,"test 存结束index = %d list %d",index,pChargeRecordData->ListID);
//        print_hex_data(pChargeRecordData->CardNo,16);
        save =  time(NULL);
        reboot++;
        if(reboot > 300)
        {
            NVIC_SystemReset();
        }
    }
}
static void Alarm_Stop_Resume(int Ch)
{
    static uint64 old_Fault = 0;
    uint64 tmp_Fault = 0;

    tmp_Fault = Get_FaultBit(Ch);

    if(tmp_Fault)
    {
        old_Fault = tmp_Fault;
    }
    else
    {
        if(old_Fault & (1LL<<ALARM_REASON_AC_INPUT_OVER_V)
            || old_Fault & (1<<ALARM_REASON_AC_INPUT_LOW_V)
            || old_Fault & (1<< ALARM_REASON_METER))//电表故障
        {
            if(Get_Car_Link(Ch) != LEVEL_12_V)
            {
                if(STATE_END_CHARGE == g_Charge_Ctrl_Manager[Ch].CurState)
                {
                    Ctrl_Continu_Charge(Ch);
                }
            }
        }
        old_Fault = tmp_Fault;
    }

}

static void Check_9V_Stop_Resume(int Ch)
{
    if(gChargeRecord[Ch].StopReason == STOP_REASON_JUMP_GUN_STOP
		&& g_Charge_Ctrl_Manager[Ch].continue_cnt++ < 3)//
    {
    	gChargeRecord[Ch].StopReason = 0;
		
        if(Get_Car_Link(Ch) != LEVEL_12_V)
        {
            if(STATE_END_CHARGE == g_Charge_Ctrl_Manager[Ch].CurState)
            {
            	dbg_printf(DBG_INFO," in  Check_9V_Stop_Resume ");
                Ctrl_Continu_Charge(Ch);
            }
        }
    }
}


static void Led_Ctrl_Fun(int Ch)
{
    #ifdef HP_HARD
    API_Led_Ctrl_Fun_HP(Ch);
    #else
    if(Get_Protocol_Type() == HP_P)
    {
        API_Led_Ctrl_Fun_HP(Ch);//LED 控制
    }
    else
    {
        API_Led_Ctrl_Fun(Ch);
    }
    #endif
}

void lcd_display_CP(void)
{
		static uint32_t tick = 0;
		
		int CP = Get_CP_Sample_2(0);
		
		//if(Tick_Out(&tick, 3000) == 1 && Tick_Out(&tick_lcd, 1000) == 1)
		if(Tick_Out(&tick_lcd, 1000) == 1)
		{  
	  	LCD_0x25_frame();
	    osDelay(300);  
	    LCD_0x26_frame(0,0x0,50,20,0,0x0);
	    osDelay(20);
	    LCD_variables_display_frame(1,3,2,0x0, 0x0,CP);
	    //Tick_Out(&tick, 0)
		}

			
}
/********************************************************
描    述 : 充电控制任务入口函数
输入参数 : void *p 参数

输出参数 :无
返    回 :无

作    者 :

创建时间 :18-06-07
*********************************************************/
void Charge_Ctrl_Thread(const void *p)
{
    Charge_Ctrl_Init();

    BSP_Pwm_Out(100,0);//满占空比认为是12V

    osDelay(5000);//等其他任务初始化完成

    Resume_Power_Off(0);


    BSP_WatchDog_Init();
    
    if(g_Sys_Config_Param.restart_num == 255)
        g_Sys_Config_Param.restart_num = 0;

    g_Sys_Config_Param.restart_num ++;
    g_Sys_Config_Param.restart_time = time(NULL);
    dbg_printf(1,"restart_num=%d ",g_Sys_Config_Param.restart_num);
    dbg_printf(1,"restart_time=%d ",g_Sys_Config_Param.restart_time);
    API_Save_Sys_Param_Direct();
    
    while(1)
    {

        uint8 State = 0;
        
        int Num = Get_DoubelGunEn() + 1;
        int Ch = 0;
        MSG_Def Msg;
        uint8 MsgFlag = 0;

				LED_Function(BOARD_LED,LED_Toggle);
				//osDelay(200);

        for(Ch = 0; Ch < Num;Ch ++)
        {

            Change_Record_Compute(Ch);//充电记录计算

            MsgFlag = MsgQueue_Get(MSG_QUEUE_TASK_CHARGE_CTRL,&Msg);

            if(MsgFlag)
            {//有消息传来,先处理消息的通道
                Ch =  Msg.Ch;
            }
            State = g_Charge_Ctrl_Manager[Ch].CurState;
            switch(State)
            {
                case STATE_IDLE:
                {
										State_Idle(Ch,MsgFlag,&Msg);         //空闲阶段处理流程
                    break;
                }

                case STATE_CHARGEING:
                {
                    State_Chargeing(Ch,MsgFlag,&Msg);    //充电阶段处理流程
                    break;
                }
                case STATE_END_CHARGE:
                {
                	Ctrl_send_charger_record(Ch);
                    State_Charge_Stop(Ch,MsgFlag,&Msg);//充电停止阶段处理流程
                    if(g_Charge_Ctrl_Manager[Ch].ChargeType == BY_CARD_STOP)
										{
											if (g_Dwin_data.control_page != bill_page)
											{
											  Dwin_Lcd_State(bill_page, Ch);
											  g_Dwin_data.control_page = bill_page;	
											}
										}
                    else 
										{	
											if(g_Charge_Ctrl_Manager[Ch].LastState != STATE_END_CHARGE)
											{
											  
												if (g_Dwin_data.control_page != finished)
												{
												  dbg_printf(1,"2937 ChargeType=%d ",g_Charge_Ctrl_Manager[Ch].ChargeType);
												  Dwin_Lcd_State(finished, Ch);
												  g_Dwin_data.control_page = finished;

												}
											}
										}
										break;
                }
                case STATE_BOOK:
                {
                    State_Charge_Book(Ch,MsgFlag,&Msg);//预约阶段处理流程
                    break;
                }
                default:
                {
                    break;

                }
            }
            //Alarm_Stop_Resume(Ch);  //告警恢复充电
						//Check_9V_Stop_Resume(Ch);
						API_Alram_Probe(Ch);    //告警监测
            API_Act_Function(Ch);   //动作类函数
            Led_Ctrl_Fun(Ch);

            BSP_WatchDog_Refresh();//喂狗
            //dbg_printf(1,"WatchDog_Refresh !!!");

              
        }
        		DHCP_Pool(NULL);
        		osDelay(5);
						lcd_display_CP();
        
    }
}

void SetupChargeCtrlApp(osPriority ThreadPriority,uint8_t StackNum)
{
	osThreadDef(ChargeCtrl, Charge_Ctrl_Thread, ThreadPriority, 0, StackNum*configMINIMAL_STACK_SIZE);
	Charge_Ctrl_Thread_Id = osThreadCreate (osThread(ChargeCtrl), NULL);
}

