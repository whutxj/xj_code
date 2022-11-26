
#ifndef __TASK_SPIFLASH_C
#define __TASK_SPIFLASH_C

/******************************************************************************
* Include Files
******************************************************************************/
#include "main.h"
#include "Task_API_SpiFlash.h"
#include "Task_SpiFlash.h"
#include "TSK_Charge_Ctrl.h"
#include <string.h>
#include "bsp_driver_uart.h"
#include <stdlib.h>
#include "Tsk_API_Alarm.h"
#include "Sys_Config.h"
#include "MsgQueue.h"
#include "CRC16.h"
#include "Drv_RN8029.h"
#include "Database.h"
#include "Tsk_Charge_Ctrl.h"
#include "Tsk_Modbus.h"
#include "lwip.h"
#define D_DBG_SAVE  1
#define TEST_LEN 10

#define CHARGE_LOG_START_INDEX    32
/******************************************************************************
* Static Variable Definitions
******************************************************************************/
typedef struct
{
    int index;              //当前待上报的索引
    ALARM_DATA alarmData;   //当前待上报的数据
    
}ReportAlarmInfo;

typedef struct
{
    int index;              //当前待上报的索引
    ChargeRecord chargeData;//当前待上报的数据
    
}ReportRecordInfo;
static ReportAlarmInfo stAlarmInfo = {0};
static ReportRecordInfo stRecordInfo = {0};
static UpdateInfo stUpdateInfo;
static uint8 g_updata_flag = 0;
/******************************************************************************
* Static Function Prototype Definitions
******************************************************************************/
//static void SpiFlashThread(void const * argument);
//static void MEM_Init_Record(void);
//static void SPIFLASH_DataErase_Record(Method2_Type_Enum RecordType);

/******************************************************************************
* Function		: Save_Real_Record 
* Parameter		:
*  
* Return			: Void 
* Description	: This function is SpiFlash Thread.
*
******************************************************************************/
static void Save_Charge_Record(uint8 Ch)
{

    static uint8 old_status[2] = {0};

    int index = 0;
    uint8 Statu = Get_Charge_Ctrl_State(Ch);
    ChargeRecord *pChargeRecordData = Get_ChargeRecord(Ch);
    ChargeRecord st_Record;
//    uint8 type = pChargeRecordData->ChargeType;//;Get_Charge_StartType(Ch);

    if(Statu == STATE_IDLE && old_status[Ch] == STATE_END_CHARGE
        && Get_Continue_Charge(Ch) == FALSE)
    {//正常结束 
        SPIFLASH_Method1_Get_Record(TYPE_REALDATA,(uint8_t*)&st_Record);

        if(st_Record.EndFlag == NOT_END)
        {
            st_Record.EndFlag = END_YET;
            SPIFLASH_Method1_Save_Record(TYPE_REALDATA,(uint8_t*)&st_Record);         
        }           
        if(pChargeRecordData->ChargeType != BY_APP_STOP)
        {         
            index = SPIFLASH_Method4_Get_RecordCnt(TYPE_CHARGELOG);
            SPIFLASH_Method4_Save_Record(TYPE_CHARGELOG,index,(uint8*)pChargeRecordData);//旧数据读出来
            dbg_printf(D_DBG_SAVE,"存结束index = %d",index);
        }
    }        
        
    if(Statu != old_status[Ch])
    {
        old_status[Ch] = Statu;
    }
}




static void Init_Power_off_Data(uint8 Ch)
{
   
    ChargeRecord *pRecord = Get_ChargeRecord(Ch);
    int index = 0;
    uint16 ival = 0;
    
    SPIFLASH_Method1_Get_Record(TYPE_REALDATA,(uint8_t*)pRecord);

    if(pRecord->EndFlag == NOT_END)
    {
       
        pRecord->EndFlag = END_YET;                                     //先变为恢复 主要是预存
        pRecord->totalFee = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR3);//保存取出来
        pRecord->EndEnerge = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR4);//保存取出来
        ival = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR5);//保存取出来
        pRecord->EndEnerge += (ival<<16); 
        if(pRecord->EndEnerge < pRecord->StartEnerge)
        {//判断值非法
            pRecord->EndEnerge = pRecord->StartEnerge;
            
            HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR12,pRecord->StartEnerge);
            HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR13,(pRecord->StartEnerge>>16));//电表数据           
        }
        pRecord->ChargeTime = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR6);  //保存取出来
        pRecord->EndTime =    HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR7);  //保存取出来
        ival = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR8);//保存取出来
        pRecord->EndTime += (ival<<16);
        if(pRecord->EndTime < pRecord->StartTime)
        {
            pRecord->EndTime = pRecord->StartTime;
        }
        pRecord->ChargeMoney = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR14);//
        //覆盖为正常
        SPIFLASH_Method1_Save_Record(TYPE_REALDATA,(uint8_t*)pRecord);
        
        //if(pRecord->ChargeTime != 0)
        {
            //保存一份到历史记录里面
            index = SPIFLASH_Method4_Get_RecordCnt(TYPE_CHARGELOG);
            SPIFLASH_Method4_Save_Record(TYPE_CHARGELOG,index,(uint8*)pRecord);//
            
            dbg_printf(1,"Power_off save index == %d Money==%d end time %d",index,pRecord->totalFee,pRecord->EndTime);    
        }

        
        //如果是卡则保存一份掉电
        if(pRecord->StartCardType == BUSINESS_CARD
            || pRecord->StartCardType == USR_CARD)
        {
            index = SPIFLASH_Method4_Get_RecordCnt(TYPE_UN_PAY);           
            SPIFLASH_Method4_Save_Record(TYPE_UN_PAY,index,(uint8_t*)pRecord);

            dbg_printf(1,"Power_on unpay index ==%d Money==%d start time %d",index,pRecord->totalFee,pRecord->StartTime);           
        }
        
        pRecord->EndFlag = NOT_END;//配置为未恢复
    } 

    

}

void Erase_spi_flash_app_data(void)
{
   int adr = SPI_FLASHE_ADDR;
   for (adr = SPI_FLASHE_ADDR;adr <  (SPI_FLASHE_ADDR + APP_SIZE);adr += 4096)
   {
        BSP_SPIFLASH_SectorErase(adr);
   }          
}

/********************************************************
描    述 :初始化升级标识

输入参数 : 

输出参数 :无
返    回 :无

创建时间 :16-05-12
*********************************************************/
static void Init_Updata_Info(void)
{
    int adr = SPI_FLASHE_ADDR;
    uint8 read[32] = {0};
    uint8 empty[32] = {0};
    BSP_SPIFLASH_BufferRead((uint8_t*)&stUpdateInfo,UPDATA_INFO_ADDR,sizeof(stUpdateInfo));
    
//    if(stUpdateInfo.UpdateMode != UPDATEMODE_NONE 
//        && stUpdateInfo.UpdateMode != FTP_IAP_MODE)
//    {
//        memset(stUpdateInfo.SoftVer,0,sizeof(stUpdateInfo.SoftVer));
//        memcpy(stUpdateInfo.SoftVer,"v1.0",sizeof("v1.0"));
//        stUpdateInfo.UpdateMode = UPDATEMODE_NONE;
//        
//        BSP_SPIFLASH_SectorErase(UPDATA_INFO_ADDR);
//        BSP_SPIFLASH_BufferWrite((uint8_t*)&stUpdateInfo,UPDATA_INFO_ADDR,sizeof(stUpdateInfo));
//    }
//    else
//    {  
//        
//    }

    if(stUpdateInfo.UpdateMode == FTP_IAP_MODE)
    {
        stUpdateInfo.UpdateMode = UPDATEMODE_NONE;
        BSP_SPIFLASH_SectorErase(UPDATA_INFO_ADDR);
        BSP_SPIFLASH_BufferWrite((uint8_t*)&stUpdateInfo,UPDATA_INFO_ADDR,sizeof(stUpdateInfo));
        g_updata_flag = 1;
    }
    
    //BSP_SPIFLASH_BufferRead((uint8_t*)&read,adr,sizeof(empty));
    
    //memset(empty,0xff,sizeof(empty));
    
        
}
/********************************************************
描    述 :初始化未恢复的告警

输入参数 : 

输出参数 :无
返    回 :无

创建时间 :16-05-12
*********************************************************/
static void Init_Un_Recover_Alarm(void)
{
    
    uint32 i,index = 0;
    
    ALARM_DATA stAlarm;
    
    index =SPIFLASH_Method4_Get_RecordCnt(TYPE_ALARMLOG);
  
    for(i = 0; i < index ;i++)
	{
        SPIFLASH_Method4_Get_Record(TYPE_ALARMLOG,i,(uint8_t*)&stAlarm);

        if(stAlarm.Recover != 0)
        {
            if(stAlarm.Recover != 0)
            {
                stAlarm.Recover = 0;
                stAlarm.EndTime = time(NULL);
                SPIFLASH_Method4_Save_Record(TYPE_ALARMLOG,i,(uint8_t*)&stAlarm);                
            }

        }
	}  
}



/*------------------------------------------------------------
操作目的：  获取没有上报的记录
初始条件：   无
操作结果：   
函数参数：type 记录类型 
			len 单个记录的长度 
				
返回值：
			
------------------------------------------------------------*/
unsigned char Get_UnReport_Data(int type,uint8* pData,int Ch)
{
    uint32 i,index = 0;

    ALARM_DATA *pAlarm;
    ChargeRecord *pCharge;

    index =SPIFLASH_Method4_Get_RecordCnt(type);
    if(type == TYPE_ALARMLOG)
    {
        for(i = stAlarmInfo.index;i <index;i++)
        {
            SPIFLASH_Method4_Get_Record(type,i,(uint8_t*)&stAlarmInfo.alarmData);

            pAlarm = (ALARM_DATA *)&stAlarmInfo.alarmData;
            
            if(pAlarm->ReportF != 0 && pAlarm->Recover == 0)
            {
                memcpy(pData,(uint8*)&stAlarmInfo.alarmData,sizeof(stAlarmInfo.alarmData));
                stAlarmInfo.index = i;
                
                return 1;
            }
            else
            {
                stAlarmInfo.index++;
            }
        }
        HistoryAlarmNum = index;
        //check full 
        if(stAlarmInfo.index > index)
        {
            stAlarmInfo.index  = index - 1;
        }

    }
    else if(type == TYPE_CHARGELOG)
    {
        for(i = stRecordInfo.index;i < index;i++)
        {
            SPIFLASH_Method4_Get_Record(type,i,(uint8_t*)&stRecordInfo.chargeData);

            pCharge = (ChargeRecord *)&stRecordInfo.chargeData;
            
            if(pCharge->Report != 0)
            {
                memcpy(pData,(uint8*)&stRecordInfo.chargeData,sizeof(stRecordInfo.chargeData));
                stRecordInfo.index = i;
                dbg_printf(D_DBG_SAVE,"准备上报历史数据 index = %d list %d",i,stRecordInfo.chargeData.ListID);   
                return 1;
            }
            else
            {
                stRecordInfo.index++;
            }
        }
        HistoryChargeNum = index;
        //check full     
        if(stRecordInfo.index > index)
        {
            stRecordInfo.index  = index - 1;
        }        
    }
    return 0;
}
/********************************************************
描    述 :更新未上报的告警标识

输入参数 : 

输出参数 :无
返    回 :无

创建时间 :16-05-12
*********************************************************/
void Resume_UpLoad_Index(int index)
{
    stAlarmInfo.index = index;
}

/********************************************************
描    述 :未上报记录初始化 主要是搜索到最新上报的记录索引

输入参数 : 

输出参数 :无
返    回 :无

创建时间 :16-05-12
*********************************************************/
void UnReport_Data_init(void)
{
	
	ALARM_DATA stAlarm;
	ChargeRecord stCharge;
	
    memset((uint8*)&stAlarmInfo,0,sizeof(stAlarmInfo));
    memset((uint8*)&stRecordInfo,0,sizeof(stAlarmInfo));
	
	Get_UnReport_Data(TYPE_ALARMLOG,(uint8*)&stAlarm,0);
	Get_UnReport_Data(TYPE_CHARGELOG,(uint8*)&stCharge,0);	
}

/*------------------------------------------------------------
操作目的：  获取没有上报的记录
初始条件：   无
操作结果：   
函数参数：type 记录类型 
			len 单个记录的长度 
				
返回值：
			
------------------------------------------------------------*/
unsigned char Updata_UnReport_Data(int type,uint8* pData,int Ch)
{


    if(type == TYPE_ALARMLOG)
    {
        stAlarmInfo.alarmData.ReportF = 0;//清除上报的标志
        SPIFLASH_Method4_Save_Record(type,stAlarmInfo.index,(uint8_t*)&stAlarmInfo.alarmData);
        return 1;
        
    }
    else if(type == TYPE_CHARGELOG )
    {
        stRecordInfo.chargeData.Report =0;
        SPIFLASH_Method4_Save_Record(type,stRecordInfo.index,(uint8_t*)&stRecordInfo.chargeData);
        return 1;   
    }
    
	return 0;
}

/********************************************************
描    述 : Get_Firmware_Ver 版本信息

输入参数 : p  版本信息

输出参数 :无
返    回 :无

创建时间 :16-05-12
*********************************************************/
void Get_Firmware_Ver(char *p)
{
    memcpy(p,stUpdateInfo.SoftVer,sizeof(stUpdateInfo.SoftVer));
}

/********************************************************
描    述 : Notice_Updata_Info 通知本任务升级

输入参数 : pUpdateInfo  升级信息

输出参数 :无
返    回 :无

创建时间 :16-05-12
*********************************************************/

void Notice_Updata_Info(UpdateInfo *pUpdateInfo)
{
    memcpy((uint8*)&stUpdateInfo,pUpdateInfo,sizeof(UpdateInfo));
}

/********************************************************
描    述 : Updata_Probe 固件版本升级

输入参数 : void

输出参数 :无
返    回 :无

创建时间 :16-05-12
*********************************************************/

static void Updata_Probe(void)
{
    int ret = 0;
    static  char cnt = 0;
    if(stUpdateInfo.UpdateMode == FTP_MODE)
    {
        uint8 step = Get_Charge_Ctrl_State(0);
        
        if(step == STATE_IDLE)
        {
            g_updata_flag = 0;
			Erase_spi_flash_app_data();
            dbg_printf(1,"start updata file %s",stUpdateInfo.ftpFile);
            ret = API_Ftp_Updata(&stUpdateInfo);
            
            if(ret)
            {    
                dbg_printf(1,"update ok\r");
                stUpdateInfo.UpdateMode = FTP_IAP_MODE;
                BSP_SPIFLASH_SectorErase(UPDATA_INFO_ADDR);
                BSP_SPIFLASH_BufferWrite((uint8_t*)&stUpdateInfo,UPDATA_INFO_ADDR,sizeof(stUpdateInfo));
                dbg_printf(1,"please reboot system\n");
                
                g_updata_flag = 1;
                //NVIC_SystemReset();               
            }
            else
            {
                g_updata_flag = 2;
                stUpdateInfo.UpdateMode = UPDATEMODE_NONE;//不要再继续升级了

                dbg_printf(1,"update err");
                
                cnt++;
                
                if(cnt >= 3)
                {
                    NVIC_SystemReset(); 
                }
            } 
        }        
    }
}
/********************************************************
描    述 : 升级成功后处理流程 写标识复位

输入参数 : void

输出参数 :无
返    回 :无

创建时间 :16-05-12
*********************************************************/
void Updata_Secces_Proc(void)
{
    save_updata_record(1);
    dbg_printf(1,"update ok\r");
    stUpdateInfo.UpdateMode = FTP_IAP_MODE;
    BSP_SPIFLASH_SectorErase(UPDATA_INFO_ADDR);
    BSP_SPIFLASH_BufferWrite((uint8_t*)&stUpdateInfo,UPDATA_INFO_ADDR,sizeof(stUpdateInfo));
    dbg_printf(1,"reboot...\n");
    
    NVIC_SystemReset();      
}

uint8 Get_Updata_Flag_And_Clear(void)
{
    uint8 ret = g_updata_flag;
    if(g_updata_flag)
        g_updata_flag = 0;
    return ret;
}
/********************************************************
描    述 : 保存系统参数

输入参数 : void

输出参数 :无
返    回 :无

创建时间 :16-05-12
*********************************************************/
void API_Save_Sys_Param(void)
{
    MSG_Def Msg = {0};
    Msg.type = MSG_SAVE_SYS_PARAM;
   
    MsgQueue_Send(MSG_QUEUE_TASK_FLASH,Msg);
}


void API_Save_Sys_Param_Direct(void)
{
    MSG_Def Msg = {0};
    Msg.type = MSG_SAVE_SYS_PARAM_DRIECT;
   
    MsgQueue_Send(MSG_QUEUE_TASK_FLASH,Msg);
}


/********************************************************
描    述 : 保存参数为出厂参数 ，恢复出厂参数时使用此参数

输入参数 : void

输出参数 :无
返    回 :无

创建时间 :16-05-12
*********************************************************/
void Save_To_Factory_Param(void)
{
    MSG_Def Msg = {0};
    Msg.type = MSG_SAVE_FACTOR_PARAM;
   
    MsgQueue_Send(MSG_QUEUE_TASK_FLASH,Msg);
}

/********************************************************
描    述 : 恢复出厂设定的参数

输入参数 : void

输出参数 :无
返    回 :无

创建时间 :16-05-12
*********************************************************/

void Restore_Factory_Param(void)
{
    MSG_Def Msg = {0};
    Msg.type = MSG_RESTORE_FACTORY_PARAM;
   
    MsgQueue_Send(MSG_QUEUE_TASK_FLASH,Msg);
}

/********************************************************
描    述 : 删除记录 包括 告警  充电 未付费

输入参数 : void

输出参数 :无
返    回 :无

创建时间 :16-05-12
*********************************************************/
void Delet_All_Record(void)
{
	SPIFLASH_Data_Erase(SPIFLASH_METHOD_MULTIRECORD_OVERWRITE,TYPE_ALARMLOG);
	SPIFLASH_Data_Erase(SPIFLASH_METHOD_MULTIRECORD_OVERWRITE,TYPE_CHARGELOG);
    SPIFLASH_Data_Erase(SPIFLASH_METHOD_MULTIRECORD_OVERWRITE,TYPE_UN_PAY);
}

/********************************************************
描    述 : 搜索未付费记录

输入参数 : CardNo 卡号

输出参数 :无
返    回 :无

创建时间 :16-05-12
*********************************************************/
uint8 Get_Un_Pay_UsrInfo(ChargeRecord *pChargeRecordData,uint8 *CardNo)
{

    int index = 0;

    int i = 0;
    ChargeRecord tmpRecordData = {0};

    index  = SPIFLASH_Method4_Get_RecordCnt(TYPE_UN_PAY);
    dbg_printf(1,"uppay index == %d ",index);
    for(i = index - 1;i >= 0;i--)
    {
        SPIFLASH_Method4_Get_Record(TYPE_UN_PAY,i,(uint8_t*)&tmpRecordData);

        if((memcmp(tmpRecordData.CardNo,CardNo,sizeof(tmpRecordData.CardNo)) == 0)
            && tmpRecordData.PayFlag == UN_PAY)
        {
            dbg_printf(1,"finde uppay ok index == %d money =%d time %d",i,tmpRecordData.ChargeMoney,tmpRecordData.StartTime);
            memcpy((uint8*)pChargeRecordData,(uint8*)&tmpRecordData,sizeof(tmpRecordData));
            return TRUE;
        }
    }

    return FALSE;
}
/********************************************************
描    述 : 从系统删除未付费的记录
输入参数 : CardNo 卡号

输出参数 :无
返    回 :无

创建时间 :16-05-12
*********************************************************/
uint8 Del_Un_Pay_UsrInfo(ChargeRecord *pChargeRecordData)
{

    int index = 0;

    int i = 0;
    ChargeRecord tmpRecordData = {0};

    index  = SPIFLASH_Method4_Get_RecordCnt(TYPE_UN_PAY);

    for(i = index - 1;i >= 0;i--)
    {
        SPIFLASH_Method4_Get_Record(TYPE_UN_PAY,i,(uint8_t*)&tmpRecordData);

        if(tmpRecordData.StartTime == pChargeRecordData->StartTime
            && tmpRecordData.PayFlag == UN_PAY)
        {

            tmpRecordData.PayFlag = PAY_YET;
            SPIFLASH_Method4_Save_Record(TYPE_UN_PAY,i,(uint8_t*)&tmpRecordData);
            
            dbg_printf(1,"del ok index == %d",i);
            return TRUE;
        }
    }

    return FALSE;
}

void Save_Start_Charge_Info(uint8 Ch)
{
    MSG_Def Msg = {0};
    Msg.type = MSG_CREAT_START_CHARGE_LOG;
    Msg.Ch = Ch;
    
    MsgQueue_Send(MSG_QUEUE_TASK_FLASH,Msg);    

}

void Save_End_Charge_Opt_Info(uint8 Ch)
{
    MSG_Def Msg = {0};
    Msg.type = MSG_UPDATA_END_CHARGE_LOG;
    Msg.Ch = Ch;
    
    MsgQueue_Send(MSG_QUEUE_TASK_FLASH,Msg);    

}
void Save_UpPay_Info(uint8 Ch)
{
    MSG_Def Msg = {0};
    Msg.type = MSG_SAVE_UNPAY_CHARGE_LOG;
    Msg.Ch = Ch;
    
    MsgQueue_Send(MSG_QUEUE_TASK_FLASH,Msg);      
}
/******************************************************************************
* Function		: static void SpiFlashThread(void const * argument)
* Parameter		:
*  
* Return			: Void 
* Description	: This function is SpiFlash Thread.
*
******************************************************************************/
static void FlashThread(void const * argument)
{
	char *p; 
    osDelay(1000);
    
    if(param_config_init() == 0)    //用于以太网修改读取参数
    {
        while(1);
    }
    Init_Power_off_Data(0);
    
    Init_Updata_Info();
    
    /* Infinite loop */
    Init_Un_Recover_Alarm();
    
    UnReport_Data_init();
    
    p = Get_ComlieTime();
    dbg_printf(1,"sys init ok ->%s",p);
    dbg_printf(1,"soft version %s",SoftVer);
	while(1)
	{   

        MSG_Def Msg = {0};
        uint32 index = 0;      
        uint8 Ret = MsgQueue_Get(MSG_QUEUE_TASK_FLASH,&Msg);
        if(Ret == TRUE)
        {
            switch(Msg.type)
            {
                case MSG_CREAT_START_ALRM_INFO:
                {//存记录
                    ALARM_DATA stAlarmData;
                    
                    index = SPIFLASH_Method4_Get_RecordCnt(TYPE_ALARMLOG);
                    memcpy(&stAlarmData,Msg.data,sizeof(ALARM_DATA));
                    SPIFLASH_Method4_Save_Record(TYPE_ALARMLOG,index,(uint8_t*)&stAlarmData);
                    dbg_printf(1,"save to alarm %d",index);                   
                    print_hex_data((uint8*)&stAlarmData,sizeof(ALARM_DATA));

                    HistoryAlarmNum = index+1;
                    break;
                }
                case MSG_UPDATA_END_ALRM_INFO:
                {//更新恢复标识
                    ALARM_DATA stAlarmTmp;
                    ALARM_DATA stAlarmData;
                    uint8_t Mask[12] = {
                                        0,//告警编码相等
                                        1,1,1,1,
                                        1,1,1,1,
                                        0,//没有恢复
                                        1,
                                        1};                     
                    memcpy(&stAlarmData,Msg.data,sizeof(ALARM_DATA));
                    stAlarmTmp = stAlarmData;
                    
                    dbg_printf(1,"seach alarm");
                    print_hex_data((uint8*)&stAlarmData,sizeof(ALARM_DATA));
                                        
                    index = SPIFLASH_Method4_Search_Record(TYPE_ALARMLOG,Mask,(uint8_t*)&stAlarmData,sizeof(ALARM_DATA));
                    if(index == 0xffff)
                    {
                        dbg_printf(1,"serch err ");
                        break;
                    }
                    else
                    {
                        SPIFLASH_Method4_Get_Record(TYPE_ALARMLOG,index,(uint8_t*)&stAlarmData);
                        stAlarmTmp.StartTime = stAlarmData.StartTime;//开始时间
                        stAlarmTmp.Recover = 0;//恢复标记

                        SPIFLASH_Method4_Save_Record(TYPE_ALARMLOG,index,(uint8_t*)&stAlarmTmp);

                        dbg_printf(1,"updata end  to alarm %d",index);
                        print_hex_data((uint8*)&stAlarmTmp,sizeof(ALARM_DATA));

                        Resume_UpLoad_Index(index);
                    }
                    break;
                }

                case MSG_UPDATA_REPORT_ALRM_INFO:
                {//更新上报标识
                    ALARM_DATA stAlarmTmp;
                    ALARM_DATA stAlarmData;
                    uint8_t Mask[12] = {
                                        0,//告警编码相等
                                        0,0,0,0,
                                        0,0,0,0,
                                        0,//
                                        0,//没有上报
                                        1}; 
                    
                    memcpy(&stAlarmData,Msg.data,sizeof(ALARM_DATA));
                    index = SPIFLASH_Method4_Search_Record(TYPE_ALARMLOG,Mask,(uint8_t*)&stAlarmData,sizeof(ALARM_DATA));
                    stAlarmTmp.ReportF = 0;
                    SPIFLASH_Method4_Save_Record(TYPE_ALARMLOG,index,(uint8_t*)&stAlarmTmp);
                    
                    dbg_printf(1,"updata report  to alarm %d time %s",index,ctime((time_t*)&stAlarmData.StartTime));
                    
                    break;
                }

                case MSG_SAVE_SYS_PARAM:            //存参数
                {
                    SYS_CONFIG_PARAM  *TmpParam = GetTmp_Config_Param();
                    TmpParam->CrcVal = CRC16((uint16_t * )TmpParam,sizeof(SYS_CONFIG_PARAM)/2 - 1);        

                    if(SPIFLASH_OK == SPIFLASH_Method1_Save_Record(TYPE_SYSPARA,(uint8*)TmpParam))//保存
                    {
                        SYS_CONFIG_PARAM  *sysParam = GetSys_Config_Param();
                        memcpy((uint8*)sysParam,(uint8*)TmpParam,sizeof(SYS_CONFIG_PARAM));
                        dbg_printf(1,"save ok");     //
                        check_opt_record();//参数修改
                        MX_LWIP_Set_IP();
                    }          
                    break;                    
                }
                case MSG_SAVE_SYS_PARAM_DRIECT:
                {
                    SYS_CONFIG_PARAM  *TmpParam = GetSys_Config_Param();
                    TmpParam->CrcVal = CRC16((uint16_t * )TmpParam,sizeof(SYS_CONFIG_PARAM)/2 - 1);        

                    if(SPIFLASH_OK == SPIFLASH_Method1_Save_Record(TYPE_SYSPARA,(uint8*)TmpParam))//保存
                    {
                        SYS_CONFIG_PARAM  *Param = GetTmp_Config_Param();
                        memcpy((uint8*)Param,(uint8*)TmpParam,sizeof(SYS_CONFIG_PARAM));
                        dbg_printf(1,"save ok");     //
                        check_opt_record();//参数修改
                    }          
                    break;                        
                }
                case MSG_SAVE_FACTOR_PARAM:
                {
                    SYS_CONFIG_PARAM  *sysParam = GetSys_Config_Param();
                    sysParam->CrcVal = CRC16((uint16_t * )sysParam,sizeof(SYS_CONFIG_PARAM)/2 - 1);        

                    SPIFLASH_Method1_Save_Record(TYPE_FACTORY_PARAM,(uint8*)sysParam);//保存
                    
                    dbg_printf(1,"save factory param %d");     //打印
                    
                    check_opt_record();//参数修改
                    break;                      
                }
                case MSG_RESTORE_FACTORY_PARAM://恢复到出厂设置
                {
                    SYS_CONFIG_PARAM  *sysParam = GetSys_Config_Param();
                    
                    SPIFLASH_Method1_Get_Record(TYPE_FACTORY_PARAM,(uint8*)sysParam);//保存
                       
                    SPIFLASH_Method1_Save_Record(TYPE_SYSPARA,(uint8*)sysParam);//保存
                    
                    //删除所有记录
                    SPIFLASH_Data_Erase(SPIFLASH_METHOD_MULTIRECORD_OVERWRITE,TYPE_ALARMLOG);
                    SPIFLASH_Data_Erase(SPIFLASH_METHOD_MULTIRECORD_OVERWRITE,TYPE_CHARGELOG);
                    SPIFLASH_Data_Erase(SPIFLASH_METHOD_MULTIRECORD_OVERWRITE,TYPE_UN_PAY);

                    SPIFLASH_Data_Erase(SPIFLASH_METHOD_MULTIRECORD,RECORD_USERLOG);
                    SPIFLASH_Data_Erase(SPIFLASH_METHOD_SINGLERECORD,TYPE_REALDATA);
                    //电表读数清除
                    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR12,0);
                    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR13,0);
                    check_opt_record();//参数修改
                    NVIC_SystemReset();   
                    break;   
                }
                case MSG_SAVE_UNPAY_CHARGE_LOG:
                {
                    ChargeRecord *pChargeRecordData = Get_ChargeRecord(Msg.Ch);
                    int index = 0;
                    index = SPIFLASH_Method4_Get_RecordCnt(TYPE_UN_PAY);
                    
                    SPIFLASH_Method4_Save_Record(TYPE_UN_PAY,index,(uint8_t*)pChargeRecordData);
                    
                    dbg_printf(1,"save UN_PAY %d",index);     //打印
                    
                    break;
                }
                case MSG_READ_ALARM_LOG:
                {
                    int index = SPIFLASH_Method4_Get_RecordCnt(TYPE_ALARMLOG);

                    if(HistoryAlarmIndex >= index)
                    {
                        HistoryAlarmIndex = index - 1;   
                    }
                    SPIFLASH_Method4_Get_Record(TYPE_ALARMLOG,HistoryAlarmIndex,(uint8_t*)&HistoryAlarm);
                    break;
                }
                case MSG_READ_CHARGE_LOG:
                {
                    int index = SPIFLASH_Method4_Get_RecordCnt(TYPE_CHARGELOG);

                    if(HistoryChargeIndex >= index)
                    {
                        HistoryChargeIndex = index - 1;                        
                    }
                    SPIFLASH_Method4_Get_Record(TYPE_CHARGELOG,HistoryChargeIndex,(uint8_t*)&HistoryCharge);
                    break;
                }
                case  MSG_CREAT_START_CHARGE_LOG:
                {
                    ChargeRecord *pChargeRecordData = Get_ChargeRecord(Msg.Ch);
                    SPIFLASH_Method1_Save_Record(TYPE_REALDATA,(uint8_t*)pChargeRecordData);
                    dbg_printf(D_DBG_SAVE,"存首次的实时数据"); 
                    break;
                }
                case MSG_UPDATA_END_CHARGE_LOG:
                {
                    ChargeRecord st_Record;
                    
                    SPIFLASH_Method1_Get_Record(TYPE_REALDATA,(uint8_t*)&st_Record);

                    if(st_Record.EndFlag == NOT_END)
                    {
                        st_Record.EndFlag = END_YET;
                        SPIFLASH_Method1_Save_Record(TYPE_REALDATA,(uint8_t*)&st_Record);         
                    }
                    break;                    
                }
                default:
                {               
                    break;               
                }
            }  
        }
        Save_Charge_Record(0);
        eth_param_cfg_proc();//网络配置参数     
        Updata_Probe();
        
        osDelay(50);
        
        board_test();
	}
}




/******************************************************************************
* Function: static void MEM_Init_Record(void)
* Parameter:void
* Return:   Void
* Description:
*   Initialize 
*
******************************************************************************/
void MEM_Init_Record(void)
{
    SPIFLASH_UserConfigDef ConfigInfo;
    
	/*记录类型初始化*/

    
	#if 0
		BSP_SPIFLASH_Init();
    int addr = 0x00;
    for(addr = 0; addr < 1024*1024;addr += 4096)
    {
        BSP_SPIFLASH_SectorErase(addr);
    }
		while(1);
	#endif
  SPIFLASH_General_Init();         
////	SPIFLASH_Data_Erase(SPIFLASH_METHOD_MULTIRECORD_OVERWRITE,TYPE_ALARMLOG);
	ConfigInfo.CommonInfo.Item.Method = SPIFLASH_METHOD_MULTIRECORD_OVERWRITE;	/*存储方法*/
	ConfigInfo.CommonInfo.Item.Type = TYPE_ALARMLOG;							/*存储类别（同一方法下的不同类型，该值必须互斥）*/
	ConfigInfo.CommonInfo.Item.OverWriteCnt = 3;								/*可重写次数*/
	ConfigInfo.CommonInfo.Item.RecordLength_User = sizeof(ALARM_DATA);		    /*记录长度*/
	ConfigInfo.CommonInfo.Item.SectorTotal = 10;									/*为该类别分配的扇区数量*/
	SPIFLASH_MEM_Init(&ConfigInfo);

	ConfigInfo.CommonInfo.Item.Method = SPIFLASH_METHOD_MULTIRECORD_OVERWRITE;	/*存储方法*/
	ConfigInfo.CommonInfo.Item.Type = TYPE_CHARGELOG;							/*存储类别（同一方法下的不同类型，该值必须互斥）*/
	ConfigInfo.CommonInfo.Item.OverWriteCnt = 3;								/*可重写次数*/
	ConfigInfo.CommonInfo.Item.RecordLength_User = sizeof(ChargeRecord);		/*记录长度*/
	ConfigInfo.CommonInfo.Item.SectorTotal = 20;       						    /*为该类别分配的扇区数量*/
	SPIFLASH_MEM_Init(&ConfigInfo);

	ConfigInfo.CommonInfo.Item.Method = SPIFLASH_METHOD_SINGLERECORD;	        /*存储方法*/
	ConfigInfo.CommonInfo.Item.Type = TYPE_SYSPARA;							    /*存储类别（同一方法下的不同类型，该值必须互斥）*/
	ConfigInfo.CommonInfo.Item.OverWriteCnt = 1;								/*可重写次数*/
	ConfigInfo.CommonInfo.Item.RecordLength_User = sizeof(SYS_CONFIG_PARAM);	/*记录长度*/
	ConfigInfo.CommonInfo.Item.SectorTotal = 1;       						    /*为该类别分配的扇区数量*/
	SPIFLASH_MEM_Init(&ConfigInfo);

	ConfigInfo.CommonInfo.Item.Method = SPIFLASH_METHOD_SINGLERECORD;	        /*存储方法*/
	ConfigInfo.CommonInfo.Item.Type = TYPE_FACTORY_PARAM;						/*存储类别（同一方法下的不同类型，该值必须互斥）*/
	ConfigInfo.CommonInfo.Item.OverWriteCnt = 1;								/*可重写次数*/
	ConfigInfo.CommonInfo.Item.RecordLength_User = sizeof(SYS_CONFIG_PARAM);	/*记录长度*/
	ConfigInfo.CommonInfo.Item.SectorTotal = 1;       						    /*为该类别分配的扇区数量*/
	SPIFLASH_MEM_Init(&ConfigInfo);    
    
	ConfigInfo.CommonInfo.Item.Method = SPIFLASH_METHOD_MULTIRECORD;	        /*存储方法*/
	ConfigInfo.CommonInfo.Item.Type = RECORD_USERLOG;							/*存储类别（同一方法下的不同类型，该值必须互斥）*/
	ConfigInfo.CommonInfo.Item.OverWriteCnt = 1;								/*可重写次数*/
	ConfigInfo.CommonInfo.Item.RecordLength_User = sizeof(opt_record_info);	            /*记录长度*/
	ConfigInfo.CommonInfo.Item.SectorTotal = 2;       						    /*为该类别分配的扇区数量*/
	SPIFLASH_MEM_Init(&ConfigInfo);

	ConfigInfo.CommonInfo.Item.Method = SPIFLASH_METHOD_SINGLERECORD;	        /*存储方法*/
	ConfigInfo.CommonInfo.Item.Type = TYPE_REALDATA;							/*存储类别（同一方法下的不同类型，该值必须互斥）*/
	ConfigInfo.CommonInfo.Item.OverWriteCnt = 1;								/*可重写次数*/
	ConfigInfo.CommonInfo.Item.RecordLength_User = sizeof(ChargeRecord);	    /*记录长度*/
	ConfigInfo.CommonInfo.Item.SectorTotal = 1;       						    /*为该类别分配的扇区数量*/
	SPIFLASH_MEM_Init(&ConfigInfo); 

	ConfigInfo.CommonInfo.Item.Method = SPIFLASH_METHOD_SINGLERECORD;	        /*存储方法*/
	ConfigInfo.CommonInfo.Item.Type = TYPE_METERPARA;							/*存储类别（同一方法下的不同类型，该值必须互斥）*/
	ConfigInfo.CommonInfo.Item.OverWriteCnt = 1;								/*可重写次数*/
	ConfigInfo.CommonInfo.Item.RecordLength_User = sizeof(SAVEPARA);	    /*记录长度*/
	ConfigInfo.CommonInfo.Item.SectorTotal = 1;       						    /*为该类别分配的扇区数量*/
	SPIFLASH_MEM_Init(&ConfigInfo); 

	ConfigInfo.CommonInfo.Item.Method = SPIFLASH_METHOD_MULTIRECORD_OVERWRITE;	/*存储方法*/
	ConfigInfo.CommonInfo.Item.Type = TYPE_UN_PAY;							    /*存储类别（同一方法下的不同类型，该值必须互斥）*/
	ConfigInfo.CommonInfo.Item.OverWriteCnt = 2;								/*可重写次数*/
	ConfigInfo.CommonInfo.Item.RecordLength_User = sizeof(ChargeRecord);		/*记录长度*/
	ConfigInfo.CommonInfo.Item.SectorTotal = 2;       						    /*为该类别分配的扇区数量*/
	SPIFLASH_MEM_Init(&ConfigInfo);
}

///******************************************************************************
//* Function: static void SPIFLASH_DataSave_Record(void)
//* Parameter:void
//* Return:   Void
//* Description:
//*   Initialize 
//*
//******************************************************************************/
//static void SPIFLASH_DataErase_Record(Method2_Type_Enum RecordType)
//{
//	SPIFLASH_Data_Erase(SPIFLASH_METHOD_MULTIRECORD,RecordType);
//}


/******************************************************************************
* Function: void SetupSpiFlashApp(osPriority ThreadPriority,uint8_t StackNum)
* Parameter:
*   1. ThreadPriority: Specifies the Thread Priority.
			@arg enum Type by osPriority,Refer to this enum.
		2. StackNum: Specifies Thread Stack.
			@arg The Block Stack Number is 128 Bytes.
* Return:   Void
* Description:
*   Setup SpiFlash Application to Main.
*
******************************************************************************/
void SetupSpiFlashApp(osPriority ThreadPriority,uint8_t StackNum)
{
	osThreadDef(SpiFlash_Thread, FlashThread, ThreadPriority, 0, StackNum*configMINIMAL_STACK_SIZE);
	osThreadCreate (osThread(SpiFlash_Thread), NULL);
}


#endif /* __TASK_SPIFLASH_C */
/******************************************************************************
*                            end of file
******************************************************************************/
