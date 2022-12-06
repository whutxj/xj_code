
#ifndef TSK_ALARM_C
#define TSK_ALARM_C

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
#include "Tsk_API_adc.h"
#include "Bsp_GPIO.h"
#include "bsp_Led_Function.h"
#include "Tsk_API_Alarm.h"
#include "Tsk_Charge_Ctrl.h"
#include "Drv_MMA8652FC.h"
uint64 g_Curent_Alarm_Code[2];     //��ǰ�澯λ
uint32 g_current_fault_start_time[2][48]; //��ǰ�澯��Ӧʱ��

#define VOL_ALRM_DELAY_TIME  10000
#define DELTA_RV   80  //�ز�

#define DBG_INFO  0
#define DELTA_V   20
/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/

/******************************************************************************
* Data Type Definitions
******************************************************************************/

/******************************************************************************
* Function Prototype Definitions
******************************************************************************/

uint64 Get_FaultBit(uint8 Ch)
{
    return g_Curent_Alarm_Code[Ch];;
}

/*********************************************************************
��    �� : ���β�Ӱ����Ĺ���
������� :
          Ch ͨ��
������� :��
��    �� :�� 
          
��    �� :


**********************************************************************/
uint8 Get_Alarm_State(uint8 Ch)
{
    uint64 MaskBit = 0xffffffffffffffff;
    
    uint64 Bit = Get_FaultBit(Ch);
    
//	if(Get_phaseType() == PHASE_1)
//	{ 
//	    //���װ汾�޴����� ���������ü��
//	    {
//	        MaskBit &= (~(1LL<<ALARM_REASON_THUNDER));
//	    }
//	}
//    
//    MaskBit &= (~(1LL << ALARM_REASON_TOUCH));//��������Ӱ����
//    
//    //���������ü��
//    {
//        MaskBit &= (~(1LL<<ALARM_REASON_CARD));
//    }    
    if(Bit & MaskBit)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

uint32 Get_Current_Alarm_Time(uint8 alarmID , uint8 Ch)
{
//    uint8 index = 0;
//    index = alarmID * 4;
    return g_current_fault_start_time[Ch][alarmID];
}
uint8 Create_Start_Alarm(uint8 id,uint8 Ch)
{
	MSG_Def Msg = {0};
	uint64 ror_val = 0;
	ALARM_DATA Info;
    uint64 val = 0;
		//dbg_printf(1,"g_Curent_Alarm_Code[Ch] = %d val=%d ror_val =%d id=%d ",g_Curent_Alarm_Code[Ch],val,ror_val,id);

    memset(&Info,0xff,sizeof(Info));
    
	Info.AlarmCode = id;
	Info.StartTime = time(NULL);
    
	Msg.type = MSG_CREAT_START_ALRM_INFO;

	memcpy(Msg.data,(uint8*)&Info,sizeof(Info));
	g_current_fault_start_time[Ch][id] = time(NULL);
	val = id;
	ror_val = (1LL << val);
	//g_Curent_Alarm_Code[Ch] |= ror_val;
	g_Curent_Alarm_Code[Ch] = val;
//dbg_printf(1,"g_Curent_Alarm_Code[Ch] = %d val=%d ror_val =%d id=%d ",g_Curent_Alarm_Code[Ch],val,ror_val,id);
	return MsgQueue_Send(MSG_QUEUE_TASK_FLASH,Msg);
}

uint8 Create_End_Alarm(uint8 id,uint8 Ch )
{
	MSG_Def Msg = {0};

	ALARM_DATA Info;
	uint64 ror_val = 0;

    memset(&Info,0xff,sizeof(Info));
	Info.AlarmCode = id;
	Info.EndTime = time(NULL);

	Msg.Ch = Ch;
	Msg.type = MSG_UPDATA_END_ALRM_INFO;

	memcpy(Msg.data,(uint8*)&Info,sizeof(Info));
	ror_val = (1LL<<id);
	//g_Curent_Alarm_Code[Ch] &= ~(ror_val);
	if(g_Curent_Alarm_Code[Ch] == id)
	{
		g_Curent_Alarm_Code[Ch] = 0;
		//dbg_printf(1,"Create_End_Alarm	 g_Curent_Alarm_Code[Ch] = %d ",g_Curent_Alarm_Code[Ch]);
	}
		
	return MsgQueue_Send(MSG_QUEUE_TASK_FLASH,Msg);
}

/********************************************************
��    �� : �Ӵ����澯̽��
������� :��

������� :��
��    �� :�� 
          
��    �� :


*********************************************************/
static void Alarm_Switch_State_Probe(uint8 Ch)
{
    static uint32 Tick[2] = {0};
    InPutDef Ret ;
    uint8 Alarm = 0;
    
    extern uint8 g_SwitchCtrl[2];
    
    static uint8 Recover[2] = {TRUE,TRUE};
    uint8 CurType = Get_Cur_Out_Type();

    if(CurType == CUR_16)
    {
        Ret = Read_KM_Input();

        //return;
    }
    else
    {        
        if(Ch == 0)
        {
            Ret = BSP_Read_InPut(KM1_DETECT);
        }
        else
        {
            //Ret = BSP_Read_InPut(KM2_DETECT);
        }
    }
    
    if(Ret == IN_LOW)
    {//ʵ���Ǳպ�
        if(g_SwitchCtrl[Ch] != SWITCH_CLOSE)
        {//���ƶϿ�
            Alarm = TRUE;
        }            
    }
    else
    {//ʵ���ǶϿ�
        if(g_SwitchCtrl[Ch] == SWITCH_CLOSE)
        {//���Ʊպ�
            Alarm = TRUE;
        }          
    }

    if(Alarm && (Recover[Ch] == TRUE))
    {
        if(Tick[Ch] == 0)
        {
            Tick[Ch] = HAL_GetTick();
        }
        else
        {
            if(HAL_GetTick() - Tick[Ch] > 5000)
            {
                if(Recover[Ch] == TRUE)
                {
                    Recover[Ch] = FALSE;
                    Tick[Ch] = 0;
                    Create_Start_Alarm(ALARM_REASON_OUTPUT_CONTACT,Ch);
                }
            }
        }
    }
    else if(Alarm == FALSE)
    {
        if(Recover[Ch] == FALSE)
        {
            if(Tick[Ch] == 0)
            {
                Tick[Ch] = HAL_GetTick();
            }
            else
            {
                if(HAL_GetTick() - Tick[Ch] > 5000)
                {                   
                    Recover[Ch] = TRUE;
                    Tick[Ch] = 0; 
                    Create_End_Alarm(ALARM_REASON_OUTPUT_CONTACT,Ch);
                }
            }            
        }
        else
        {
            Tick[Ch] = 0;
        }

    }

}

/********************************************************
��    �� : ��ͣ�澯̽��
������� :��

������� :��
��    �� :�� 
          
��    �� :


*********************************************************/
static void Alarm_Stop_Button_Probe(uint8 Ch)
{
    static uint32 Tick[2] = {0};
    InPutDef Ret ;
    uint8 Alarm = 0;
    static uint8 Recover[2] = {TRUE,TRUE};

    Ret = BSP_Read_InPut(STOP_DETECT);
     
    if(Ret != IN_LOW)
    {//ʵ���Ǳպ�
        Alarm = FALSE;                 
    }
    else
    {
        Alarm = TRUE;
                  
    }
    if(Alarm)
    {
        if(Recover[Ch] == TRUE)
        {
            if(Tick[Ch] == 0)
            {
                Tick[Ch] = HAL_GetTick();
            }
            else
            {
                if(HAL_GetTick() - Tick[Ch] >= 50)
                {
                    if(Recover[Ch] == TRUE)
                    {
                        Recover[Ch] = FALSE;
                        Tick[Ch] = 0;
                        Create_Start_Alarm(ALARM_REASON_EMERGENCY,Ch);
                    }
                }
            }
        }
        else
        {
            Tick[Ch] = 0;
        }
    }
    else if((Alarm == FALSE))
    {
        if((Recover[Ch] == FALSE))
        {
            if(Tick[Ch] == 0)
            {
                Tick[Ch] = HAL_GetTick();
            }
            else
            {
                if(HAL_GetTick() - Tick[Ch] >= 2000)
                {                   
                    Recover[Ch] = TRUE;
                    Tick[Ch] = 0;
                    Create_End_Alarm(ALARM_REASON_EMERGENCY,Ch);
                }
            }            
        }
        else
        {
            Tick[Ch] = 0;
        }
    }
}

/********************************************************
��    �� : ���������
������� :��

������� :��
��    �� :�� 
          
��    �� :


*********************************************************/
static void Alarm_Thunder_Probe(uint8 Ch)
{
//    static uint8 Ret=0,CheckCount[2]={0};
//    static uint8 Recover[2] = {TRUE,TRUE};
//    
//    
//    Ret = BSP_Read_InPut(SPD_DETECT1);
//    
//    if(Ret==TRUE)
//    {
//        CheckCount[Ch]++;
//    }
//    else
//    {
//        if(CheckCount[Ch]>0)
//        {
//            CheckCount[Ch]--;
//        }
//    }
//    
//    if(CheckCount[Ch]>50 && Recover[Ch] == TRUE)
//    {
//        CheckCount[Ch] = 50;
//        Recover[Ch] = FALSE;
//        Create_Start_Alarm(ALARM_REASON_THUNDER,Ch);
//    }
//    else if(Recover[Ch] == FALSE && CheckCount[Ch] < 40)
//    {
//        CheckCount[Ch] = 0;
//        Recover[Ch] = TRUE;
//        Create_End_Alarm(ALARM_REASON_THUNDER,Ch);
//    }

    
    static uint32 Tick[2] = {0};
    InPutDef Ret ;
    uint8 Alarm = 0;
    static uint8 Recover[2] = {TRUE,TRUE};

    Ret = BSP_Read_InPut(SPD_DETECT1);
     
    if(Ret == IN_LOW)
    {//ʵ���Ǳպ�
        Alarm = FALSE;                 
    }
    else
    {
        Alarm = TRUE;
                  
    }
    if(Alarm)
    {
        if(Recover[Ch] == TRUE)
        {
            if(Tick[Ch] == 0)
            {
                Tick[Ch] = HAL_GetTick();
            }
            else
            {
                if(HAL_GetTick() - Tick[Ch] >= 5000)
                {
                    if(Recover[Ch] == TRUE)
                    {
                        Recover[Ch] = FALSE;
                        Tick[Ch] = 0;
                        Create_Start_Alarm(ALARM_REASON_THUNDER,Ch);
                    }
                }
            }
        }
        else
        {
            Tick[Ch] = 0;
        }
    }
    else if((Alarm == FALSE))
    {
        if((Recover[Ch] == FALSE))
        {
            if(Tick[Ch] == 0)
            {
                Tick[Ch] = HAL_GetTick();
            }
            else
            {
                if(HAL_GetTick() - Tick[Ch] >= 2000)
                {                   
                    Recover[Ch] = TRUE;
                    Tick[Ch] = 0;
                    Create_End_Alarm(ALARM_REASON_THUNDER,Ch);
                }
            }            
        }
        else
        {
            Tick[Ch] = 0;
        }
    }
}


/********************************************************
��    �� : ��ͣ���
������� :��

������� :��
��    �� :�� 
          
��    �� :


*********************************************************/
static void Alarm_Meter_Com_Probe(uint8 Ch)
{
    static uint32 Tick[2] = {0};
    int Ret  = 0 ;
    uint8 Alarm = 0;
    static uint8 Recover[2] = {TRUE,TRUE};

    if(Get_MeterType() !=TYPE_DLT_07)
    {
        return;
    }
    
    Ret = Get_Meter_Comm(Ch);
    
    if(Ret == FALSE)
    {
        Alarm = TRUE;
    }
    else
    {
        Alarm = FALSE;
    }

    if(Alarm)
    {
        if((Recover[Ch] == TRUE))
        {
            if(Tick[Ch] == 0)
            {
                Tick[Ch] = HAL_GetTick();
            }
            else
            {
                if(HAL_GetTick() - Tick[Ch] >= 1000)
                {
                    if(Recover[Ch] == TRUE)
                    {
                        Recover[Ch] = FALSE;
                        Tick[Ch] = 0;
                        Create_Start_Alarm(ALARM_REASON_METER,Ch);
                    }
                }
            }
        }
        else
        {
            Tick[Ch] = 0;
        }
    }
    else if((Alarm == FALSE))
    {
        if(Recover[Ch] == FALSE)
        {
            if(Tick[Ch] == 0)
            {
                Tick[Ch] = HAL_GetTick();
            }
            else
            {
                if(HAL_GetTick() - Tick[Ch] >= 1000)
                {                   
                    Recover[Ch] = TRUE;
                    Tick[Ch] = 0;
                    Create_End_Alarm(ALARM_REASON_METER,Ch);
                }
            }            
        }
        else
        {
            Tick[Ch] = 0;
        }

    }
}

/********************************************************
��    �� : 
������� :��

������� :��
��    �� :�� 
          
��    �� :


*********************************************************/
static void Alarm_Car_Link_Probe(uint8 Ch)
{
    static uint32 Tick[2] = {0};

    uint8 Alarm = 0;
    static uint8 Recover[2] = {TRUE,TRUE};

    if(Get_Charge_Ctrl_State(Ch) != STATE_CHARGEING)
    {
        Alarm = FALSE;
    }
    else
    {
        if(Get_CP_Sample_2(0) < 300)//100
        {
            Alarm = TRUE;
        }else
        {
            Alarm = FALSE;
        }
    }

    if(Alarm && (Recover[Ch] == TRUE))
    {
//        if(Tick[Ch] == 0)
//        {
//            Tick[Ch] = HAL_GetTick();
//        }
//        else
//        {
//            if(HAL_GetTick() - Tick[Ch] >= 1000)
//            {
                if(Recover[Ch] == TRUE)
                {
                    Recover[Ch] = FALSE;
                    Tick[Ch] = 0;
                    Create_Start_Alarm(ALARM_REASON_CONNECT,Ch);
                }
//            }
//        }
    }
    else if((Alarm == FALSE))
    {
        if(Recover[Ch] == FALSE)
        {
            if(Tick[Ch] == 0)
            {
                Tick[Ch] = HAL_GetTick();
            }
            else
            {
                if(HAL_GetTick() - Tick[Ch] >= 15000)
                {                   
                    Recover[Ch] = TRUE;
                    Tick[Ch] = 0;
                    Create_End_Alarm(ALARM_REASON_CONNECT,Ch);
                }
            }            
        }
        else
        {
            Tick[Ch] = 0;
        }

    }
}

/********************************************************
��    �� : ������ͨѶ���
������� :��

������� :��
��    �� :�� 
          
��    �� :


*********************************************************/
static void Alarm_CardRead_Com_Probe(uint8 Ch)
{
    static uint32 Tick[2] = {0};
    int Ret  = 0 ;
    uint8 Alarm = 0;
    static uint8 Recover[2] = {TRUE,TRUE};
    uint8 type = Get_Card_Read_Type();
    if( type == CARD_NONE
        ||type == NO_CARD_START)
    {
        return;
    }
    
    Ret = Get_CardCommStatu();
    
    if(Ret == FALSE)
    {
        Alarm = TRUE;
    }
    else
    {
        Alarm = FALSE;
    }
    
    if(Alarm)
    {
        if((Recover[Ch] == TRUE))
        {
            if(Tick[Ch] == 0)
            {
                Tick[Ch] = HAL_GetTick();
            }
            else
            {
                if(HAL_GetTick() - Tick[Ch] >= 1000)
                {
                    if(Recover[Ch] == TRUE)
                    {
                        Recover[Ch] = FALSE;
                        Tick[Ch] = 0;
                        Create_Start_Alarm(ALARM_REASON_CARD,Ch);
                    }
                }
            }
        }
        else
        {
            Tick[Ch] = 0;
        }
    }
    else if(Alarm == FALSE)
    {
        if(Recover[Ch] == FALSE)
        {
            if(Tick[Ch] == 0)
            {
                Tick[Ch] = HAL_GetTick();
            }
            else
            {
                if(HAL_GetTick() - Tick[Ch] >= 1000)
                {                   
                    Recover[Ch] = TRUE;
                    Tick[Ch] = 0;
                    Create_End_Alarm(ALARM_REASON_CARD,Ch);
                }
            }            
        }
        else
        {
            Tick[Ch] = 0;
        }

    }
}

/********************************************************
��    �� : ������ͨѶ���
������� :��

������� :��
��    �� :�� 
          
��    �� :


*********************************************************/
static void Alarm_Screen_Com_Probe(uint8 Ch)
{
    static uint32 Tick[2] = {0};
    int Ret  = 0 ;
    uint8 Alarm = 0;
    static uint8 Recover[2] = {TRUE,TRUE};
    extern uint8 Get_Screen_Comm(void);
    
    if(Get_Screen_En() == FALSE)
    {
        return;
    }
    
    //Ret = Get_Screen_Comm();
    
    if(Ret == FALSE)
    {
        Alarm = TRUE;
    }
    else
    {
        Alarm = FALSE;
    }
    
    if(Alarm)
    {
        if(Recover[Ch] == TRUE)
        {
            if(Tick[Ch] == 0)
            {
                Tick[Ch] = HAL_GetTick();
            }
            else
            {
                if(HAL_GetTick() - Tick[Ch] >= 1000)
                {
                    if(Recover[Ch] == TRUE)
                    {
                        Recover[Ch] = FALSE;
                        Tick[Ch] = 0;
                        Create_Start_Alarm(ALARM_REASON_TOUCH,Ch);
                    }
                }
            }
        }
        else
        {
            Tick[Ch] = 0;
        }
    }
    else if(Alarm == FALSE)
    {
        if(Recover[Ch] == FALSE)
        {
            if(Tick[Ch] == 0)
            {
                Tick[Ch] = HAL_GetTick();
            }
            else
            {
                if(HAL_GetTick() - Tick[Ch] >= 1000)
                {                   
                    Recover[Ch] = TRUE;
                    Tick[Ch] = 0;
                    Create_End_Alarm(ALARM_REASON_TOUCH,Ch);
                }
            }            
        }
        else
        {
            Tick[Ch] = 0;
        }

    }
}


/********************************************************
��    �� : �����
������� :��

������� :��
��    �� :�� 
          
��    �� :


*********************************************************/
void Alarm_Elec_Lock_State_Probe(uint8 Ch)
{
//    static uint32 Tick[2] = {0};
//    InPutDef Ret ;
//    uint8 Alarm = 0;
//    static uint8 Recover[2] = {TRUE,TRUE};
//    
//    if(Ch == 0)
//    {
//        Ret = BSP_Read_InPut(ELECTR_DETECT1);
//    }
//    else
//    {
//        Ret = BSP_Read_InPut(ELECTR_DETECT2);
//    }
//    
//    if(Ret == IN_LOW)
//    {//ʵ��������
//        if(g_LockCtrl[Ch] == UNLOCK_GUN)
//        {//���ƶϿ�
//            Alarm = TRUE;
//        }            
//    }
//    else
//    {//ʵ���ǶϿ�
//        if(g_SwitchCtrl[Ch] == LOCK_GUN)
//        {//��������
//            Alarm = TRUE;
//        }          
//    }

//    if(Alarm && (Recover[Ch] == TRUE))
//    {
//        if(Tick[Ch] == 0)
//        {
//            Tick[Ch] = HAL_GetTick();
//        }
//        else
//        {
//            if(HAL_GetTick() - Tick[Ch] > 5000)
//            {
//                if(Recover[Ch] == TRUE)
//                {
//                    Recover[Ch] = FALSE;
//                    Tick[Ch] = 0;
//                    Create_Start_Alarm(ALARM_REASON_ELC_LOCK,Ch);
//                }
//            }
//        }
//    }
//    else if((Alarm == FALSE))
//    {
//        if( (Recover[Ch] == FALSE))
//        {
//            if(Tick[Ch] == 0)
//            {
//                Tick[Ch] = HAL_GetTick();
//            }
//            else
//            {
//                if(HAL_GetTick() - Tick[Ch] > 1000)
//                {                   
//                    Recover[Ch] = TRUE;
//                    Tick[Ch] = 0;
//                    Create_End_Alarm(ALARM_REASON_ELC_LOCK,Ch);                
//                }
//            }           
//        }
//        else
//        {
//            Tick[Ch] = 0;
//        }

//    }
}

/********************************************************
��    �� : ��ѹ���
������� :��

������� :��
��    �� :�� 
          
��    �� :


*********************************************************/
static void Alarm_Volt_Over_Probe(uint8 Ch)
{

    static uint8 Recover[2] = {TRUE,TRUE};

    MEASURE_DATA Mesure_Data;
    static uint32 Tick[2] = {0};
    if(Get_Charge_Ctrl_State(Ch) != STATE_CHARGEING)
    {
        if(Recover[Ch] == FALSE)
        {
            if(Tick[Ch] == 0)
            {
                Tick[Ch] = time(NULL);
            }
            else
            {
                if(time(NULL) - Tick[Ch] > 30)//����ʱ��
                {
                    Recover[Ch] = TRUE;
                    Tick[Ch] = 0;
                    Create_End_Alarm(ALARM_REASON_AC_INPUT_OVER_V,Ch);                                  
                }
            }
        }
        else
        {
            Tick[Ch] = 0;
        }        
        return;
    }
    Mesure_Data = Get_Measrue_Data(Ch);
         

    if(Mesure_Data.VolageA > Get_OverVolt()\
	|| Mesure_Data.VolageB > Get_OverVolt()\
	|| Mesure_Data.VolageC > Get_OverVolt())
    {
        if(Recover[Ch] == TRUE)
        {
            if(Tick[Ch] == 0)
            {
                Tick[Ch] = time(NULL);
            }
            else
            {
                if(time(NULL) - Tick[Ch] > 60)//����ʱ��
                {
                    if(Recover[Ch] == TRUE)
                    {
                        Recover[Ch] = FALSE;
                        Tick[Ch] = 0;
                        Create_Start_Alarm(ALARM_REASON_AC_INPUT_OVER_V,Ch);            
                    }
                }
            }
        }
        else
        {
            Tick[Ch] = 0;
        }
    }
    else
    {

        if((Mesure_Data.VolageA <= Get_OverVolt() - DELTA_RV)\
        &&(Mesure_Data.VolageB <= Get_OverVolt() - DELTA_RV)\
        &&(Mesure_Data.VolageC <= Get_OverVolt() - DELTA_RV))
        {
            if(Recover[Ch] == FALSE)
            {
                if(Tick[Ch] == 0)
                {
                    Tick[Ch] = time(NULL);
                }
                else
                {
                    if(time(NULL) - Tick[Ch] > 15)//����ʱ��
                    {
                        Recover[Ch] = TRUE;
                        Tick[Ch] = 0;
                        Create_End_Alarm(ALARM_REASON_AC_INPUT_OVER_V,Ch);                                  
                    }
                }
            }

        }  
        else
        {
            Tick[Ch] = 0;
        }
        
    }
}


/********************************************************
��    �� : Ƿѹ���
������� :��

������� :��
��    �� :�� 
          
��    �� :


*********************************************************/
static void Alarm_Volt_Under_Probe(uint8 Ch)
{
    
    static uint8 Recover[2] = {TRUE,TRUE};
    static uint32 Tick[2] = {0};
    MEASURE_DATA Mesure_Data;
    int Ret = Get_Meter_Comm(Ch);
    if(Get_Charge_Ctrl_State(Ch) != STATE_CHARGEING)
    {
        if(Recover[Ch] == FALSE)
        {
            if(Tick[Ch] == 0)
            {
                Tick[Ch] = time(NULL);
            }
            else
            {
                if(time(NULL) - Tick[Ch] > 30)//����ʱ��
                {       
                    if(Recover[Ch] == FALSE)
                    {
                        Tick[Ch] = 0;//���
                        Recover[Ch] = TRUE;
                        Create_End_Alarm(ALARM_REASON_AC_INPUT_LOW_V,Ch);                        
                    }                        
                }                        
            }
        }
        else
        {
            Tick[Ch] = 0;
        }
        return;
    }    
    if(Ret == FALSE && (Get_MeterType() ==TYPE_DLT_07))
    {
        Tick[Ch] = 0;//���
        return;
    }
    Mesure_Data = Get_Measrue_Data(Ch);
    
    if(Get_phaseType() == PHASE_3)
    {
        if((Mesure_Data.VolageA < Get_Undervoltage() - DELTA_V)\
        &&(Mesure_Data.VolageB < Get_Undervoltage() - DELTA_V)\
        &&(Mesure_Data.VolageC < Get_Undervoltage() - DELTA_V))
        {
            if(Recover[Ch] == TRUE)
            {
                if(Tick[Ch] == 0)
                {
                    Tick[Ch] = time(NULL);
                }
                else
                {
                    if(time(NULL) - Tick[Ch] > 60)//����ʱ��
                    {                
                        if(Recover[Ch] == TRUE)
                        {
                             Recover[Ch] = FALSE;
                             Tick[Ch] = 0;//���
                             Create_Start_Alarm(ALARM_REASON_AC_INPUT_LOW_V,Ch);
                        }            
                    }
                }
            }
            else
            {
                Tick[Ch] = 0;//���
            }
        }
        else
        {
            

            if((Mesure_Data.VolageA >= Get_Undervoltage() + DELTA_RV)\
            ||(Mesure_Data.VolageB >= Get_Undervoltage() + DELTA_RV)\
            ||(Mesure_Data.VolageC >= Get_Undervoltage() + DELTA_RV))
            {
                if(Recover[Ch] == FALSE)
                {
                    if(Tick[Ch] == 0)
                    {
                        Tick[Ch] = time(NULL);
                    }
                    else
                    {
                        if(time(NULL) - Tick[Ch] > 15)//����ʱ��
                        {              
                            Recover[Ch] = TRUE;
                            Tick[Ch] = 0;//���
                            Create_End_Alarm(ALARM_REASON_AC_INPUT_LOW_V,Ch);                              
                        }
                    }
                }
                else
                {
                    Tick[Ch] = 0;//���
                }                

            }
            else
            {
                Tick[Ch] = 0;//���
            }
            
        }
    }
    else
    {//����
        if((Mesure_Data.VolageA < Get_Undervoltage() - DELTA_V)
          && Mesure_Data.VolageA >= 700)
        {
            if(Recover[Ch] == TRUE)
            {
                if(Tick[Ch] == 0)
                {
                    dbg_printf(1,"volt =%d",Mesure_Data.VolageA);
                    Tick[Ch] = time(NULL);
                }
                else
                {
                    if(time(NULL) - Tick[Ch] > 60)//����ʱ��
                    {                
                        if(Recover[Ch] == TRUE)
                        {
                             Recover[Ch] = FALSE;
                             Tick[Ch] = 0;//���
                             Create_Start_Alarm(ALARM_REASON_AC_INPUT_LOW_V,Ch);
                        }            
                    }
                }
            }
            else
            {
                Tick[Ch] = 0;//���
            }
        }
        else
        {

            if((Mesure_Data.VolageA >= Get_Undervoltage() + DELTA_RV))
            {
                if(Recover[Ch] == FALSE)
                {
                    if(Tick[Ch] == 0)
                    {
                        Tick[Ch] = time(NULL);
                    }
                    else
                    {
                        if(time(NULL) - Tick[Ch] > 15)//����ʱ��
                        {       
                            if(Recover[Ch] == FALSE)
                            {
                                Tick[Ch] = 0;//���
                                Recover[Ch] = TRUE;
                                Create_End_Alarm(ALARM_REASON_AC_INPUT_LOW_V,Ch);                        
                            }                        
                        }                        
                    }
                }
                else
                {
                    //dbg_printf(1,"clear = %d",Mesure_Data.VolageA);
                    Tick[Ch] = 0;//���
                }                
            }
            else
            {
                //dbg_printf(1,"clear = %d",Mesure_Data.VolageA);
                Tick[Ch] = 0;//���
            }               
      
        }        
    }
}


/********************************************************
��    �� : ȱ����
������� :��

������� :��
��    �� :�� 
          
��    �� :

����ʱ�� :17-9-11
*********************************************************/
static void Alarm_Volt_Lose_Phase(uint8 Ch)
{
	static uint8 RecoverLosePH[2] = {TRUE,TRUE};
    MEASURE_DATA Mesure_Data;
    int Ret = Get_Meter_Comm(Ch);
    static uint32 Tick[2] = {0};
    
    if(Ret == FALSE)
    {
        return;
    }
    Mesure_Data = Get_Measrue_Data(Ch);

// ȱ���ж�
	if((Mesure_Data.VolageA<500 && (Mesure_Data.VolageB>1500||Mesure_Data.VolageC>1500))\
	|| (Mesure_Data.VolageB<500 && (Mesure_Data.VolageA>1500||Mesure_Data.VolageC>1500))\
	|| (Mesure_Data.VolageC<500 && (Mesure_Data.VolageA>1500||Mesure_Data.VolageB>1500)))
	{
        if(Tick[Ch] == 0)
        {
            Tick[Ch] = time(NULL);
        }
        else
        {
            if(time(NULL) - Tick[Ch] > 10)//����ʱ��
            {                
                if(RecoverLosePH[Ch] == TRUE)
                {
                    RecoverLosePH[Ch] = FALSE;
                    Create_Start_Alarm(ALARM_REASON_LOSE_PHASE,Ch);            
                }
            }
        }

	}
    else
    {
        Tick[Ch] = 0;
        if(RecoverLosePH[Ch] == FALSE)
        {
            if((Mesure_Data.VolageA > 1500)\
			&&(Mesure_Data.VolageB > 1500)\
			&&(Mesure_Data.VolageC > 1500))
            {
                RecoverLosePH[Ch] = TRUE;
                Create_End_Alarm(ALARM_REASON_LOSE_PHASE,Ch);  
            }
        }
        
    }

}

/********************************************************
��    �� : �������
������� :��

������� :��
��    �� :�� 
          
��    �� :


*********************************************************/
static void Alarm_Current_Over_Probe(uint8 Ch)
{
    static uint32 Tick[2] = {0};


    static uint8 Recover[2] = {TRUE,TRUE};
    uint8 CurType = Get_Cur_Out_Type();
    float CurValue = 0;
    float WarningCur = 0;
    
    MEASURE_DATA Mesure_Data;

    Mesure_Data = Get_Measrue_Data(Ch);

    if(CurType == CUR_16)
    {
        CurValue = 1600;
    }
    if(CurType == CUR_32)
    {
        CurValue = 3200;
    }
    if(CurType == CUR_63)
    {
        CurValue = 6300;
    }
    
    CurValue *= 1.15;
    if(Get_phaseType() == PHASE_3)
    {
        if(Mesure_Data.CurrentA > Mesure_Data.CurrentB)
        {
            WarningCur = Mesure_Data.CurrentA;
        }
        else
        {
            WarningCur = Mesure_Data.CurrentB;
        }
        //
        if(WarningCur < Mesure_Data.CurrentC)
        {
            WarningCur = Mesure_Data.CurrentC;
        }
    }
    else
    {
        WarningCur = Mesure_Data.CurrentA;  // * 1.732;
    }
    //end
    if(/*Mesure_Data.CurrentA*/WarningCur >= CurValue)
    {//
        if(Tick[Ch] == 0)
        {
            Tick[Ch] = time(NULL);
        }
        else
        {
            if(time(NULL) - Tick[Ch] >= 5)
            {
        
                if(Recover[Ch] == TRUE)
                {
                    Recover[Ch] = FALSE;
                    Tick[Ch] = 0;
                    Create_Start_Alarm(ALARM_REASON_AC_OUT_PUT_ONET_I,Ch);             
                }
            }
        }            

    }
    else
    {
        if(Recover[Ch] == FALSE)
        {
            if(Tick[Ch] == 0)
            {
                Tick[Ch] = time(NULL);
                
            }
            else
            {
                if(time(NULL) - Tick[Ch] > 30)//�����ָ�ʱ��
                {                   
                    Recover[Ch] = TRUE;
                    Tick[Ch] = 0;
                    Create_End_Alarm(ALARM_REASON_AC_OUT_PUT_ONET_I,Ch);                     
                }
            }

        }
        else
        {
            Tick[Ch] = 0;
        }            
    }
}

/********************************************************
��    �� : ©�������
������� :��

������� :��
��    �� :�� 
          
��    �� :


*********************************************************/

static void Alarm_Leakage_current_Over_Probe(uint8 Ch)
{
    static uint32 Tick[2] = {0};


    static uint8 Recover[2] = {TRUE,TRUE};

    uint32_t CurValue = 0;
    uint32_t WarningCur = 250;  //  25mA
    
    //end
    CurValue = get_Leakage_current();
    
    if(/*Mesure_Data.CurrentA*/CurValue >= WarningCur)
    //if(/*Mesure_Data.CurrentA*/CurValue >= 20)
    {//
        if(Tick[Ch] == 0)
        {
            Tick[Ch] = time(NULL);
        }
        else
        {
            if(time(NULL) - Tick[Ch] >= 2)
            {
                Tick[Ch]  = time(NULL);
                dbg_printf(1,"LeakCurValue %d",CurValue);
                if(Recover[Ch] == TRUE)
                {
                    Recover[Ch] = FALSE;
                    Tick[Ch] = 0;
                    Create_Start_Alarm(ALARM_REASON_LEAKAGE_CURRENT,Ch);             
                }
            }
        }            

    }
    else
    {
        if(Recover[Ch] == FALSE)
        {
            if(Tick[Ch] == 0)
            {
                Tick[Ch] = time(NULL);
                
            }
            else
            {
                if(time(NULL) - Tick[Ch] > 15)//�����ָ�ʱ��
                {                   
                    Recover[Ch] = TRUE;
                    Tick[Ch] = 0;
                    Create_End_Alarm(ALARM_REASON_LEAKAGE_CURRENT,Ch);                     
                }
            }

        }
        else
        {
            Tick[Ch] = 0;
        }            
    }
}

void Alrm_Check_Temp_Over(uint8 Ch)
{
    #define TEMP_OVER 115   
    static uint32 Tick[2] = {0};
    //InPutDef Ret ;
    //uint8 Alarm = 0;
    static uint8 Recover[2] = {TRUE,TRUE};

    SYS_CONFIG_PARAM  *TempSysParam = GetSys_Config_Param();
    uint32_t temp = Get_system_Temp();
    
    if((temp > TEMP_OVER))
    {
        if(Recover[Ch] == TRUE)
        {
            if(Tick[Ch] == 0)
            {
                dbg_printf(1,"temp =%d",temp);
                Tick[Ch] = time(NULL);
            }
            else
            {
                if(time(NULL) - Tick[Ch] > 15)//����ʱ��
                {                
                    if(Recover[Ch] == TRUE)
                    {
                         Recover[Ch] = FALSE;
                         Tick[Ch] = 0;//���
                         Create_Start_Alarm(ALARM_REASON_SISTEM_TEMP,Ch);
                    }            
                }
            }
        }
        else
        {
            Tick[Ch] = 0;//���
        }
    }
    else
    {

        if((temp <= TEMP_OVER - 5))
        {
            if(Recover[Ch] == FALSE)
            {
                if(Tick[Ch] == 0)
                {
                    Tick[Ch] = time(NULL);
                }
                else
                {
                    if(time(NULL) - Tick[Ch] > 15)//����ʱ��
                    {       
                        if(Recover[Ch] == FALSE)
                        {
                            Tick[Ch] = 0;//���
                            Recover[Ch] = TRUE;
                            Create_End_Alarm(ALARM_REASON_SISTEM_TEMP,Ch);                        
                        }                        
                    }                        
                }
            }
            else
            {
     
                Tick[Ch] = 0;//���
            }                
        }
        else
        {
            Tick[Ch] = 0;//���
        }               
  
    }        

}


static void print_Cur_alarm(uint64 oldAlarm_Code,uint64 Curent_Alarm_Code)
{
    static uint32 sec = 0;
    
    if(time(NULL) - sec > 60)
    {
        SYS_CONFIG_PARAM *pSysParam = GetTmp_Config_Param();
        sec = time(NULL);
        
        dbg_printf(1,"my ip %d.%d.%d.%d",pSysParam->LocalIp[0],pSysParam->LocalIp[1],pSysParam->LocalIp[2],pSysParam->LocalIp[3]);  
    }
#if 0    
    if(Curent_Alarm_Code& 1LL<< ALARM_REASON_AC_OUT_PUT_ONET_I)
    {
        dbg_printf(1,"�������");
    }
    else
    {
        if(oldAlarm_Code& 1LL<< ALARM_REASON_AC_OUT_PUT_ONET_I)
        {
            dbg_printf(1,"��������ָ�");
        }
        
    }
    if(Curent_Alarm_Code & 1LL<< ALARM_REASON_AC_INPUT_LOW_V)
    {
        dbg_printf(1,"���Ƿѹ");
    }
    else
    {
        if(oldAlarm_Code& 1LL<< ALARM_REASON_AC_INPUT_LOW_V)
        {
            dbg_printf(1,"���Ƿѹ�ָ�");
        }
    }
    if(Curent_Alarm_Code & 1LL<< ALARM_REASON_AC_INPUT_OVER_V)
    {
        dbg_printf(1,"��ѹ�澯");
    }
    else
    {
        if(oldAlarm_Code & 1LL<< ALARM_REASON_AC_INPUT_OVER_V)
        {
            dbg_printf(1,"��ѹ�澯�ָ�");
        }
    }
    
    if(Curent_Alarm_Code & 1LL<< ALARM_REASON_METER)
    {
        dbg_printf(1,"���ͨѶ�쳣");
    }
    else
    {
        if(oldAlarm_Code & 1LL<< ALARM_REASON_METER)
        {
            dbg_printf(1,"���ͨѶ�ָ�");
        }
    }    
    if(Curent_Alarm_Code & 1LL<< ALARM_REASON_THUNDER)
    {
        dbg_printf(1,"�������쳣");
    }
    else
    {
        if(oldAlarm_Code & 1LL<< ALARM_REASON_THUNDER)
        {
            dbg_printf(1,"�������ָ�");
        }
    }
    
    if(Curent_Alarm_Code & 1LL<< ALARM_REASON_EMERGENCY)
    {
        dbg_printf(1,"��ͣ����");
    }
    else
    {
        if(oldAlarm_Code & 1LL<< ALARM_REASON_EMERGENCY)
        {
            dbg_printf(1,"��ͣ�ָ�");
        }
    }    
    if(Curent_Alarm_Code & 1LL<< ALARM_REASON_OUTPUT_CONTACT)
    {
        dbg_printf(1,"�Ӵ����쳣");
    }
    else
    {
        if(oldAlarm_Code & 1LL<< ALARM_REASON_OUTPUT_CONTACT)
        {
            dbg_printf(1,"�Ӵ����쳣�ָ�");
        }
    }

    if(Curent_Alarm_Code & 1LL<< ALARM_REASON_CARD)
    {
        dbg_printf(1,"������ͨѶ����");
    }
    else
    {
        if(oldAlarm_Code & 1LL<< ALARM_REASON_CARD)
        {
            dbg_printf(1,"������ͨѶ�ָ�");
        }
    }
    if(Curent_Alarm_Code & 1LL<< ALARM_REASON_TOUCH)
    {
        dbg_printf(1,"������ͨѶ����");
    }
    else
    {
        if(oldAlarm_Code & 1LL<< ALARM_REASON_TOUCH)
        {
            dbg_printf(1,"������ͨѶ�ָ�");
        }
    }
    
    if(Curent_Alarm_Code & 1LL<< ALARM_REASON_LEAKAGE_CURRENT)
    {
        dbg_printf(1,"©��������");
    }
    else
    {
        if(oldAlarm_Code & 1LL<< ALARM_REASON_LEAKAGE_CURRENT)
        {
            dbg_printf(1,"©�����ָ�");
        }
    } 
#endif 
//dbg_printf(1,"Curent_Alarm_Code = %d ",Curent_Alarm_Code);

if(Curent_Alarm_Code == ALARM_REASON_AC_OUT_PUT_ONET_I)
	 {
			 dbg_printf(1,"�������");
	 }
	 else
	 {
			 if(oldAlarm_Code == ALARM_REASON_AC_OUT_PUT_ONET_I)
			 {
					 dbg_printf(1,"��������ָ�");
			 }
			 
	 }
	 if(Curent_Alarm_Code == ALARM_REASON_AC_INPUT_LOW_V)
	 {
			 dbg_printf(1,"���Ƿѹ");
	 }
	 else
	 {
			 if(oldAlarm_Code == ALARM_REASON_AC_INPUT_LOW_V)
			 {
					 dbg_printf(1,"���Ƿѹ�ָ�");
			 }
	 }
	 if(Curent_Alarm_Code == ALARM_REASON_AC_INPUT_OVER_V)
	 {
			 dbg_printf(1,"��ѹ�澯");
	 }
	 else
	 {
			 if(oldAlarm_Code == ALARM_REASON_AC_INPUT_OVER_V)
			 {
					 dbg_printf(1,"��ѹ�澯�ָ�");
			 }
	 }
	 
	 if(Curent_Alarm_Code == ALARM_REASON_METER)
	 {
			 dbg_printf(1,"���ͨѶ�쳣");
	 }
	 else
	 {
			 if(oldAlarm_Code == ALARM_REASON_METER)
			 {
					 dbg_printf(1,"���ͨѶ�ָ�");
			 }
	 }		
	 if(Curent_Alarm_Code == ALARM_REASON_THUNDER)
	 {
			 dbg_printf(1,"�������쳣");
	 }
	 else
	 {
			 if(oldAlarm_Code == ALARM_REASON_THUNDER)
			 {
					 dbg_printf(1,"�������ָ�");
			 }
	 }
	 
	 if(Curent_Alarm_Code == ALARM_REASON_EMERGENCY)
	 {
			 dbg_printf(1,"��ͣ����");
	 }
	 else
	 {
			 if(oldAlarm_Code == ALARM_REASON_EMERGENCY)
			 {
					 dbg_printf(1,"��ͣ�ָ�");
			 }
	 }		
	 if(Curent_Alarm_Code == ALARM_REASON_OUTPUT_CONTACT)
	 {
			 dbg_printf(1,"�Ӵ����쳣");
	 }
	 else
	 {
			 if(oldAlarm_Code == ALARM_REASON_OUTPUT_CONTACT)
			 {
					 dbg_printf(1,"�Ӵ����쳣�ָ�");
			 }
	 }

	 if(Curent_Alarm_Code == ALARM_REASON_CARD)
	 {
			 dbg_printf(1,"������ͨѶ����");
	 }
	 else
	 {
			 if(oldAlarm_Code == ALARM_REASON_CARD)
			 {
					 dbg_printf(1,"������ͨѶ�ָ�");
			 }
	 }
	 if(Curent_Alarm_Code == ALARM_REASON_TOUCH)
	 {
			 dbg_printf(1,"������ͨѶ����");
	 }
	 else
	 {
			 if(oldAlarm_Code == ALARM_REASON_TOUCH)
			 {
					 dbg_printf(1,"������ͨѶ�ָ�");
			 }
	 }
	 
	 if(Curent_Alarm_Code == ALARM_REASON_LEAKAGE_CURRENT)
	 {
			 dbg_printf(1,"©��������");
	 }
	 else
	 {
			 if(oldAlarm_Code == ALARM_REASON_LEAKAGE_CURRENT)
			 {
					 dbg_printf(1,"©�����ָ�");
			 }
	 } 

}

 /********************************************************
��    �� : �澯API ̽�⺯��
������� :��

������� :��
��    �� :�� 
          
��    �� :


*********************************************************/ 
void API_Alram_Probe(uint8 Ch)
{
    static uint32 tick[2] = {0,0};
    static uint64 old_alarm_Code[2] = {0,0};
    
//    Alarm_Switch_State_Probe(Ch);   //�Ӵ���״̬
    Alarm_Stop_Button_Probe(Ch);    //��ͣ����̽��

    Alarm_Meter_Com_Probe(Ch);      //
    Alarm_CardRead_Com_Probe(Ch);   //
    Alarm_Car_Link_Probe(Ch);       //cp�ӵع���
    //Alarm_Elec_Lock_State_Probe(Ch);//��ͣ���
    Alarm_Volt_Over_Probe(Ch);      //��ѹ���
    Alarm_Volt_Under_Probe(Ch);     //Ƿѹ���
    Alarm_Current_Over_Probe(Ch);   //�������
//    Alarm_Screen_Com_Probe(Ch);

//    Alrm_Check_Temp_Over(Ch); 	//
//    Alarm_Volt_Lose_Phase(Ch);		//ȱ����
//    Alarm_Thunder_Probe(Ch);		//���������

//    Alarm_Leakage_current_Over_Probe(Ch);  //©�������
    //�澯�������
    if(old_alarm_Code[Ch] != g_Curent_Alarm_Code[Ch])
    {
        print_Cur_alarm(old_alarm_Code[Ch],g_Curent_Alarm_Code[Ch]);
        old_alarm_Code[Ch] = g_Curent_Alarm_Code[Ch];
    }
    else
    {
        if(Tick_Out(&tick[Ch],10000))
        {
            print_Cur_alarm(old_alarm_Code[Ch],g_Curent_Alarm_Code[Ch]);
            Tick_Out(&tick[Ch],0);
        }           
        
    }
}

#endif

