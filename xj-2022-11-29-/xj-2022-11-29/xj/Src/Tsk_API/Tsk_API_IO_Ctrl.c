
#ifndef TSK_IO_CTRL_C
#define TSK_IO_CTRL_C

/******************************************************************************
* Includes
******************************************************************************/

#include "MsgQueue.h"
#include "string.h"
#include "cmsis_os.h"
#include "time.h"
#include "Sys_Config.h"
#include "Global.h"
#include "Tsk_API_Meter.h"
#include "Utility.h"

#include "Bsp_GPIO.h"
#include "Bsp_ADC.h"
#include "bsp_Led_Function.h"
#include "Tsk_API_IO_Ctrl.h"
#include "Tsk_Charge_Ctrl.h"
#include "Bsp_Pwm.h"
#include "Bsp_Buzzer.h"
static uint8 g_BeepCtrl[2];
static uint8 g_LockCtrl[2];
uint8 g_SwitchCtrl[2];

#define DBG_INFO  0

/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/

/******************************************************************************
* Data Type Definitions
******************************************************************************/

/******************************************************************************
* Function Prototype Definitions
******************************************************************************/


#define LIMIT_9V   800


uint8 Get_Car_Link(uint8 Ch)
{
	    
	int adSamp = Get_CP_Sample(Ch);
	//dbg_printf(1,"CP = %d ",adSamp);

	if(adSamp > 1000)
    {
        return LEVEL_12_V;
    }
    else if(adSamp <= 1000 && adSamp > LIMIT_9V)
    {
        return LEVEL_9_V;
    }
    else if(adSamp <= LIMIT_9V && adSamp > 300)//100
    {
        return LEVEL_6_V;
    }
    
    return LEVEL_12_V;
    
}
//判断枪是否在位
uint8 Get_Lock_Seat(uint8 Ch)
{
//    if(Ch == 0)
//        return BSP_Read_InPut(GUN_LOCK_DETECT1);
//    else
//        return BSP_Read_InPut(GUN_LOCK_DETECT2);
	return 0;
}

/********************************************************
描    述 : 通知蜂鸣器动作
输入参数 :无

输出参数 :无
返    回 :无 
          
作    者 :

创建时间 :18-06-07
*********************************************************/

void Notice_Beep(uint8 BeepState,uint8 Ch)
{
    g_BeepCtrl[Ch] = BeepState;
}

/********************************************************
描    述 : 通知接触器动作
输入参数 :无

输出参数 :无
返    回 :无 
          
作    者 :

创建时间 :18-06-07
*********************************************************/

void Notice_Switch(uint8 SwitchState,uint8 Ch)
{//接触器和继电器控制
    g_SwitchCtrl[Ch] = SwitchState;
    //dbg_printf(1,"动作接触器 %d",SwitchState);
}
/********************************************************
描    述 : 通知电磁锁动作
输入参数 :无

输出参数 :无
返    回 :无 
          
作    者 :

创建时间 :18-06-07
*********************************************************/

void Notice_Ctrl_Lock(uint8 LockState,uint8 Ch)
{
    g_LockCtrl[Ch] = LockState;
}
/********************************************************
描    述 : 充电锁控制函数
输入参数 :无

输出参数 :无
返    回 :无 
          
作    者 :

创建时间 :18-06-07
*********************************************************/

static void Switch_Ctrl_Fun(uint8 Ch)
{

    static uint32 tick[2] = {0};
//    MEASURE_DATA MeterData;
//    MeterData = Get_Measrue_Data(Ch);
    uint8_t CP = Get_Car_Link(Ch);
    if(g_SwitchCtrl[Ch] == SWITCH_CLOSE)
    {
        if(Ch == 0)
        {
            BSP_GPIO_OutPut(MK1_OUT,OUT_HIGHE);
            BSP_GPIO_OutPut(1,OUT_HIGHE);
            BSP_GPIO_OutPut(2,OUT_HIGHE);
            //dbg_printf(1,"close Switch");
        }
        else
        {
            //BSP_GPIO_OutPut(MK2_OUT,OUT_HIGHE); 
        }
        
    }
    if(g_SwitchCtrl[Ch] == SWITCH_DISCON_IM)
    {
        if(Ch == 0)
        {
            BSP_GPIO_OutPut(MK1_OUT,OUT_LOW);
            BSP_GPIO_OutPut(1,OUT_LOW);
            BSP_GPIO_OutPut(2,OUT_LOW);
            //dbg_printf(1,"open switch imd");
        }
        else
        {
            //BSP_GPIO_OutPut(MK2_OUT,OUT_LOW); 
        }
        //dbg_printf(DBG_INFO,"time out switch Disconnect");        
    }
    if(g_SwitchCtrl[Ch] == SWITCH_DISCONNECT)
    {
        if(tick[Ch] == 0)
        {
            Tick_Out(&tick[Ch],0);
        }
        else
        {
            if(Tick_Out(&tick[Ch],3000))
            {//超时断开
                tick[Ch] = 0;
                if(Ch == 0)
                {
                    BSP_GPIO_OutPut(MK1_OUT,OUT_LOW);
                    BSP_GPIO_OutPut(1,OUT_LOW);
                    BSP_GPIO_OutPut(2,OUT_LOW);
                }
                else
                {
                   //BSP_GPIO_OutPut(MK2_OUT,OUT_LOW); 
                }
                
                dbg_printf(DBG_INFO,"3S delay open switch");
            }
            else
            {
                if( CP == LEVEL_12_V
                    || CP == LEVEL_9_V)
                {//允许断开的电流值
                                      
                    if(Ch == 0)
                    {
                        BSP_GPIO_OutPut(MK1_OUT,OUT_LOW);
                        BSP_GPIO_OutPut(1,OUT_LOW);
                        BSP_GPIO_OutPut(2,OUT_LOW);
                    }
                    else
                    {
                       //BSP_GPIO_OutPut(MK2_OUT,OUT_LOW); 
                    }
                    dbg_printf(DBG_INFO,"9V delay open switch");
                                       
                    tick[Ch] = 0;               
                }
            }
        }    
    }
    else
    {
        tick[Ch] = 0;//clear
        
    }
}

void HAL_TIM4_PeriodElapsedCallback(void)
{
    
    uint8 CurType = Get_Cur_Out_Type();
    //切断在此控制
    char Ch = 0;
    //闭合也在此控制
    
    if(CurType != CUR_16)
    {
        //return;
    }        

    for(Ch = 0;Ch < 2;Ch++)
    {
        
        if(g_SwitchCtrl[Ch] == SWITCH_CLOSE)
        {
            if(Ch == 0)
            {
                BSP_GPIO_OutPut(MK1_OUT,OUT_HIGHE);
            }
            else
            {
                //BSP_GPIO_OutPut(MK2_OUT,OUT_HIGHE); 
            }
            
        }
        if(g_SwitchCtrl[Ch] == SWITCH_DISCON_IM)
        {
            if(Ch == 0)
            {
                BSP_GPIO_OutPut(MK1_OUT,OUT_LOW);
            }
            else
            {
                //BSP_GPIO_OutPut(MK2_OUT,OUT_LOW); 
            }
                  
        }
        if(g_SwitchCtrl[Ch] == SWITCH_DISCONNECT)
        {

            if(Ch == 0)
            {
                BSP_GPIO_OutPut(MK1_OUT,OUT_LOW);
            }
            else
            {
               //BSP_GPIO_OutPut(MK2_OUT,OUT_LOW); 
            }     
        }
    }
}

/********************************************************
描    述 : 接触器控制函数
输入参数 :无

输出参数 :无
返    回 :无 
          
作    者 :

创建时间 :18-06-07
*********************************************************/

static void Lock_Ctrl_Fun(uint8 Ch)
{
    static uint32 tick[2] = {1,1};    //动作脉冲定时器
    static uint32 Cltr_Sec[2] = {0,0};//锁定定时器  
    static uint32 DelayTick[2] = {0}; //延时锁定定时器
//    static uint32 TackGunTick[2] = {0};//取走充电枪定时器
    InPutDef Ret;
//    BSP_GPIO_Init();
    if(Ch == 0)
    {
        Ret = BSP_Read_InPut(ELECTR_DETECT1);
    }
    else
    {
        //Ret = BSP_Read_InPut(ELECTR_DETECT2);
    }
    
    if(Get_LocKEn() == FALSE)
    {
        return;
    }
    
    if(g_LockCtrl[Ch] == LOCK_GUN)
    {
        if(Ret != IN_LOW && (Get_Lock_Seat(Ch) == TRUE))
        {//不是锁定的 并且枪在的时候
            if(DelayTick[Ch] == 0)
            {
                DelayTick[Ch] = 1;
                Tick_Out(&Cltr_Sec[Ch],0);
            }
            else
            {
                if(Tick_Out(&Cltr_Sec[Ch],5000))
                {
                    
                    if(tick[Ch] == 0)
                    {
                        Tick_Out(&tick[Ch],0);
                        if(Ch == 0)
                        {
                            BSP_GPIO_OutPut(ELECTR1_LOCK1,OUT_HIGHE);
                            BSP_GPIO_OutPut(ELECTR1_LOCK2,OUT_LOW);                
                        }
                        else
                        {
                            //BSP_GPIO_OutPut(ELECTR2_LOCK1,OUT_HIGHE);
                            //BSP_GPIO_OutPut(ELECTR2_LOCK2,OUT_LOW);
                        }
                        dbg_printf(1,"动作锁定");
                    }

                }
            }            
                         
        }
        if(tick[Ch] != 0 )
        {            
            if(Tick_Out(&tick[Ch],200))
            {
                if(Ch == 0)
                {
                    BSP_GPIO_OutPut(ELECTR1_LOCK1,OUT_LOW);
                    BSP_GPIO_OutPut(ELECTR1_LOCK2,OUT_LOW);                
                }
                else
                {
                    //BSP_GPIO_OutPut(ELECTR2_LOCK1,OUT_LOW);
                    //BSP_GPIO_OutPut(ELECTR2_LOCK2,OUT_LOW);
                }          
                tick[Ch] = 0;
                Tick_Out(&Cltr_Sec[Ch],0);
                DelayTick[Ch] = 0;
                dbg_printf(1,"动作锁定结束");
            }
        }        
    }
    else if(g_LockCtrl[Ch] == UNLOCK_GUN)
    {
        if(Ret == IN_LOW)
        {          
 
            if(Tick_Out(&Cltr_Sec[Ch],5000))
            {
             
                if(tick[Ch] == 0)
                {
                    Tick_Out(&tick[Ch],0);
                    if(Ch == 0)
                    {
                        BSP_GPIO_OutPut(ELECTR1_LOCK1,OUT_LOW);
                        BSP_GPIO_OutPut(ELECTR1_LOCK2,OUT_HIGHE);                
                    }
                    else
                    {
                        //BSP_GPIO_OutPut(ELECTR2_LOCK1,OUT_LOW);
                        //BSP_GPIO_OutPut(ELECTR2_LOCK2,OUT_HIGHE);
                    }
                    dbg_printf(1,"动作解锁");                    

                }
            }            
        }
        if(tick[Ch] != 0)
        {
            if(Tick_Out(&tick[Ch],200))
            {
                if(Ch == 0)
                {
                    BSP_GPIO_OutPut(ELECTR1_LOCK1,OUT_LOW);
                    BSP_GPIO_OutPut(ELECTR1_LOCK2,OUT_LOW);                
                }
                else
                {
                    //BSP_GPIO_OutPut(ELECTR2_LOCK1,OUT_LOW);
                    //BSP_GPIO_OutPut(ELECTR2_LOCK2,OUT_LOW);
                }          
                tick[Ch] = 0;
                Tick_Out(&Cltr_Sec[Ch],0);
                dbg_printf(1,"动作解锁完毕");   
            }
        }
         
    }
   

    
}

/********************************************************
描    述 : 蜂鸣器控制函数
输入参数 :无

输出参数 :无
返    回 :无 
          
作    者 :

创建时间 :18-06-07
*********************************************************/

static void Beep_Ctrl_Fun(void)
{

    static uint32 tick = 0;
    
    if(g_BeepCtrl[0] == LAST_RING || g_BeepCtrl[1] == LAST_RING
        || Get_Alarm_State(0) != 0 ||  Get_Alarm_State(1) != 0)
    {
        static uint32 lastTick = 0;
        static char Xflag = 0;
        if(Tick_Out(&lastTick,500))
        {
            Xflag ^= 1;
            Tick_Out(&lastTick,0);
        }
        if(Xflag)
        {
            BSP_Buzzer_on();
        }
        else
        {
            BSP_Buzzer_off();
        }
    }
    else if(g_BeepCtrl[0] == ONCE_RING ||  g_BeepCtrl[1] == ONCE_RING)
    {

        if(tick == 0)
        {
            Tick_Out(&tick,0);
            BSP_Buzzer_on();
        }
        else
        {
            if(Tick_Out(&tick,500))
            {
                if(g_BeepCtrl[0] == ONCE_RING)
                {
                     g_BeepCtrl[0] = BEEP_OFF;
                }
                
                if(g_BeepCtrl[1] == ONCE_RING)
                {
                     g_BeepCtrl[1] = BEEP_OFF;
                }
                
                BSP_Buzzer_off();
                
                tick = 0;
            }
        }  
    }
    else
    {
        BSP_Buzzer_off();
        tick = 0;
    }
}

/********************************************************
描    述 : 动作函数
输入参数 :无

输出参数 :无
返    回 :无 
          
作    者 :

创建时间 :18-06-07
*********************************************************/ 
void API_Act_Function(uint8 Ch)
{
    uint8 CurType = Get_Cur_Out_Type();


    Switch_Ctrl_Fun(Ch);            //接触器动作

    
    Lock_Ctrl_Fun(Ch);              //电磁锁控制
    
    Beep_Ctrl_Fun();                //蜂鸣器动作
             
}
/********************************************************
描    述 : 指示灯控制函数
输入参数 :无

输出参数 :无
返    回 :无 
          
作    者 :l

创建时间 :18-06-07
*********************************************************/

void API_Led_Ctrl_Fun(uint8 Ch)
{
    static uint32 tick[2] = {0};
    extern u8_t loginStatus;
//    uint8 led_bit = 0;
    static char flash;
    
    if(Tick_Out(&tick[Ch],500) == TRUE)
    {
        Tick_Out(&tick[Ch],0); 
        flash ^= 1;
    }
    else
    {
        return;
    }
    
    if(Get_FaultBit(Ch))
    { 
        if(Get_Alarm_State(Ch))
        {//故障灯亮
            LED_Function(RED_LED1,LED_On);
            LED_Function(GRERN_LED1,LED_Off);
            LED_Function(YALLOW_LED1,LED_Off);    
        }
        else
        {// 告警
            if(flash)
            {
                LED_Function(RED_LED1,LED_On);
            }
            else
            {
                LED_Function(RED_LED1,LED_Off);
            }
            LED_Function(GRERN_LED1,LED_Off);
            LED_Function(YALLOW_LED1,LED_Off);
        }
        if(Get_Charge_Ctrl_State(Ch) != STATE_END_CHARGE)
        {
			  if (g_Dwin_data.control_page != fault)
				{
					Dwin_Lcd_State(fault, Ch);
					g_Dwin_data.control_page = fault;
				}
        }
        if(loginStatus == 1)
        {
            LED_Function(NET_LED, LED_Off);
        }        
    }
    else if(Get_Charge_Ctrl_State(Ch) == STATE_IDLE)
    {                 
        
        LED_Function(RED_LED1,LED_Off); 
        if(Get_Car_Link(Ch) != LEVEL_12_V)
        {
            LED_Function(YALLOW_LED1,LED_On);
            LED_Function(GRERN_LED1,LED_Off);
            #if 0
            if(g_Dwin_data.control_page != connected)
            {
                Dwin_Lcd_State(connected, Ch);
                g_Dwin_data.control_page = connected;
            }
            #endif

		}
        else
        {
            LED_Function(GRERN_LED1,LED_On);
            LED_Function(YALLOW_LED1,LED_Off);
            #if 0
            if (g_Dwin_data.control_page != disconnected)
            {
                Dwin_Lcd_State(disconnected, Ch);
                g_Dwin_data.control_page = disconnected;
            }
            #endif
		}
        if(loginStatus == 1)
        {
            LED_Function(NET_LED, LED_On);
        }
        else
        {
            LED_Function(NET_LED, LED_Off);
        }            
    }
    else if(Get_Charge_Ctrl_State(Ch) == STATE_CHARGEING
        ||Get_Charge_Ctrl_State(Ch) == STATE_END_CHARGE)
    {
        if(flash||Get_Charge_Ctrl_State(Ch) == STATE_END_CHARGE)
        {
            LED_Function(YALLOW_LED1,LED_On);         
        }
        else
        {
            LED_Function(YALLOW_LED1,LED_Off);           
        }
        
        LED_Function(RED_LED1,LED_Off);
        if(loginStatus == 1)
        {
            LED_Function(NET_LED, LED_On);
        }
        else
        {
            LED_Function(NET_LED, LED_Off);
        }       
    }
}

void API_Led_Ctrl_Fun_HP(uint8 Ch)
{
    
}

#endif

