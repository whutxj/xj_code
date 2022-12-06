
#ifndef _TSK_API_CARD_C
#define _TSK_API_CARD_C

/******************************************************************************
* Include Files
******************************************************************************/
#include "main.h"
#include "drv_usart.h"
#include "bsp_driver_uart.h"
#include <string.h>
#include "Utility.h"
#include "sys_Config.h"
#include <time.h>
#include "Tsk_API.CARD_MT628.h"
#define CARD_PORT  COM5

/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/

static uint8 g_Chargeing_CardNo[2][8];//充电过程中的卡号
//static uint8 g_CommTick = 0;
static uint8 g_CommFault = 0;
static char Last = 0;//用于检测一直刷卡就不动作

#define INTERVAL 6

#define CARD_NUM_LENGTH			8			/**< 卡号长度 单位byte */ 
#define COMM_RETRY_NUM			6			/**< 卡通信重试次数 */ 

/******************************************************************************
* Static Function Prototype Definitions
******************************************************************************/
uint8 MT628_Search_Card(CARD_DATA *PCardData)
{
   
    DRV_MTStatusTypeDef MTStatusType = Drv_MT_IC_FindCard();

    if(MTStatusType == MT_DRV_OK)
    {    
    	 g_CommFault = 0;
         return TRUE;     
    }

    return FALSE;
}

static uint8 MT_Find_Card(uint16 *cardType)
{
    static uint32_t last_tick = 0;
    static uint32_t time_tick = 0;
    
	static uint32 tick = 0;
 
    if(Tick_Out(&tick,300))
    {
        Tick_Out(&tick,0);
    }
    else
    {
        return FIND_CARD_NONE;
    }    
    DRV_MTStatusTypeDef MTStatusType = Drv_MT_IC_FindCard();

    if(MTStatusType == MT_DRV_OK && Last == 0)
    {
        if(cardType != NULL)
        {//参数检测
            
        }
        time_tick = 0;
        g_CommFault = 0;
        last_tick = time(NULL);
        return FIND_CARD_OK;
        
    }
    else
    {
        if(MTStatusType != MT_DRV_OK )
        {
            Last = 0;
        }
        if(time(NULL) - last_tick > INTERVAL)
        {
            Last = 0;
        }
        if(MTStatusType ==  MT_DRV_TIMEOUT)
        {
            time_tick++;
            if(time_tick > 3)
            {
                g_CommFault = 1;
            }
            
        }
        else
        {
            g_CommFault = 0;            
        }            
    }
    return FIND_CARD_NONE;  
}


/********************************************************
描    述 : MTd第一次充电刷卡操作
输入参数 : Ch ,那个通道

输出参数 :PCardData  卡数据
返    回 : START_CRG_OPT_CARD类型
    
作    者 :lx

创建时间 :17-1-4
*********************************************************/

START_CRG_OPT_CARD MT628_Start_Charge_Card(CARD_DATA *PCardData,uint8 Ch)
{
    uint8 CardType = 0;
    uint8_t CardID[4],PIN[3],LockStatus,LockStatusConform;
    int Balance = 0;//分负数和正数
    START_CRG_OPT_CARD ret = (START_CRG_OPT_CARD)0;
    
    uint8 RcvData[16] = {0};
    uint8 RetryCnt = 0;
    ret = OPT_ERR;
    
    uint8 Key[6] = {0};
    Get_Card_Key(Key);
    
    asc2hex(PIN,Key,6);
		   
    
    if(MT_Find_Card(NULL) == FIND_CARD_OK)
    {
                                    
       if(MT_DRV_OK == Drv_MT_IC_Preprocess( PIN, RcvData))
       {
            uint8_t i , index = 0;
            
            PCardData->CardType = RcvData[index++];
            PCardData->CardStatus = RcvData[index++];
            for(i = 0; i < 8;i++)
            {
                PCardData->Card_no[i] =  RcvData[index++];
            }
            PCardData->Balance = (RcvData[index]<<24) + (RcvData[index+1]<<16) + (RcvData[index+2]<<8) + RcvData[index+3];
            index += 4;              
            //
            CardType = PCardData->CardType;
            if(CardType  == BUSINESS_CARD)
            {//一般用户卡

                if(PCardData->CardStatus == 0x32 || PCardData->CardStatus == 0x31)
                {
                    ret =  OPT_LOCK;
                }
                else
                {                                         
                    if(PCardData->Balance >= MIN_ALLOW_MONEY)
                    {
                        while(COMM_RETRY_NUM > RetryCnt)
                        {
                            
                            /**< 开始加电 */
                            if(MT_DRV_OK == Drv_MT_IC_StartCharge(&Balance))    //启动后确认卡锁状态   如果未上锁则不启动充电  2017-12-12 10:13
                            {   
                                /**< start 预处理   2017-12-12 10:13*/

                                dbg_printf(1,"card MT628 money %d",Balance);

                                ret =   OPT_OK; 
                                break;                                
 
                                /**< end 预处理   2017-12-12 10:13*/
                            }
                            else
                            {
                                ret = OPT_ERR;   //不允许加电
                                //osDelay(500);
                                RetryCnt++;
                                dbg_printf(1,"card MT628 start %d",RetryCnt);
                                
                            }
                        }
                    }
                    else
                    {
                        ret =   OPT_BALANCE_LACK;
                    }                                                                
                }     
            }
            else if(CardType == 0x51)
            {//超级管理卡
                ret =   OPT_OK;
                PCardData->CardType = SUPER_MANAGE;
            }
            else if(CardType == ONLINE_CARD)
            {//在线卡
                ret =   OPT_OK;
            }          
        }       
    }
    if(OPT_OK == ret)
    {
        memcpy(g_Chargeing_CardNo[Ch],PCardData->Card_no,8);
        Last = 1;
    }
    
    return ret;
}



///********************************************************
//描    述 : MT协议补充交易寻卡操作
//输入参数 : 无

//输出参数 :cardType 卡类型
//返    回 :TRUE:接收解析正确 
//          FALSE :错误
//作    者 :lx

//创建时间 :17-1-4
//*********************************************************/

//static uint8 MT_Find_Card_Add(uint16 *cardType)
//{
//    uint8 SendBuf[32] = {0};
//    uint8 Data[16] = {0};
//    uint8 RcvData[16] = {0};
//    int len = 0;
//    len = MT_Card_Pack(SendBuf,FIND_M1,Data,0);
//    Drv_Uart_Write(CARD_PORT,(const uint8 *)SendBuf,len,1);
//    if(MT_Card_Parse(RcvData,40) == TRUE)
//    {
//        if(RcvData[5] == 0x59)
//        {
//            if(cardType != NULL)
//            {//参数检测
//                memcpy(cardType,RcvData + 1,2);
//            }

//            return FIND_CARD_OK;
//        }

//        return FIND_CARD_NONE;
//    }   
//    return FIND_CARD_ERR;  
//}

/********************************************************
描    述 : MT第二次结束充电刷卡操作
输入参数 : Ch ,那个通道
            Record 写入到卡里面的记录数据
输出参数 :无
返    回 : END_OPT_CARD类型
    
作    者 :lx

创建时间 :17-1-4
*********************************************************/

END_OPT_CARD MT628_End_Charge_Card(CARD_RECORD Record,int *Balance,int Ch)
{
    uint8 CardType,CardStatus = 0;
    uint8_t PIN[3]; //,LockStatus,LockStatusConform;
    
    END_OPT_CARD ret = (END_OPT_CARD)0;
    
    uint8 RcvData[16] = {0};
//    uint8 RetryCnt = 0;
    ret = END_ERR;
    uint8 CardNo[8] = {0};
//    int tmpBalance = 0;                                
    uint8 Key[6] = {0};
    Get_Card_Key(Key);
    
    asc2hex(PIN,Key,6);   
    if(MT_Find_Card(NULL) == FIND_CARD_OK)
    {         
        if(MT_DRV_OK == Drv_MT_IC_Preprocess( PIN, RcvData))
        {     
            uint8_t i , index = 0;

            CardType = RcvData[index++];
            CardStatus = RcvData[index++];
            
            for(i = 0; i < 8;i++)
            {
                CardNo[i] =  RcvData[index++];
            }
               
            if(CardType == 0x51)
            {//超级管理卡
                *Balance = 0;
                ret = END_OK;
            }
            else if(Record.type == CardType && CardType == ONLINE_CARD)
            {
                ret = END_OK;
            }
            else if(CardType  == BUSINESS_CARD)
            {//一般管理卡
                
                if(memcmp(g_Chargeing_CardNo[Ch],CardNo,8) == 0)
                {//卡号匹配
                    
                    if(CardStatus == 0x30)
                    {
                        return END_NOT_MATCH;
                    }
                    else if(CardStatus == 0x31 || CardStatus == 0x32)
                    {
                        uint32 balance = 0;
                        DRV_MTStatusTypeDef opt = Drv_MT_IC_StopCharge(Record.Money, &balance);   //此处更新的剩余金额有效   //for test 2017-12-12 11：13
                        if(MT_DRV_OK == opt)
                        {
                             ret = END_OK;
                             *Balance = balance;
                             dbg_printf(1,"end MT528 0k %d",balance);
                        }
                        else
                        {
                            ret = END_ERR;
                            dbg_printf(1,"Write err");
                        }                                
                    }                           
                }
                else
                {
                    return END_NOT_MATCH;
                }               
            }
            else if(Record.type != CardType)
            {
                return END_NOT_MATCH;
            }
        }           
    }
    if(END_OK == ret)
    {
        Last = 1;
        dbg_printf(1,"end card ok");
    }
    return ret;
}


static uint8 MT628_Find_Card_Add(uint16 *cardType)
{
    //static uint32_t last_tick = 0;
    
    DRV_MTStatusTypeDef MTStatusType = Drv_MT_IC_FindCard();
    
    if(MTStatusType == MT_DRV_OK)
    {
        return FIND_CARD_OK;
    }
    else
    {
         return FIND_CARD_NONE;  
    }
 
}

/********************************************************
描    述 : MT第二次补扣充电刷卡操作
输入参数 : Ch ,那个通道
            Record 写入到卡里面的记录数据
输出参数 :无
返    回 : END_OPT_CARD类型
    
作    者 :lx

创建时间 :17-1-4
*********************************************************/

END_OPT_CARD MT628_Add_Charge_Card(CARD_RECORD Record,int Ch)
{
    uint8 CardType,CardStatus = 0;
    uint8_t PIN[3];
    
    END_OPT_CARD ret = (END_OPT_CARD)0;
    
    uint8 RcvData[16] = {0};
//    uint8 RetryCnt = 0;
    ret = END_ERR;
    uint8 CardNo[8] = {0};
//    int tmpBalance = 0;                                
    uint8 Key[6] = {0};
    Get_Card_Key(Key);
    
    asc2hex(PIN,Key,6);
    
    if(MT628_Find_Card_Add(NULL) == FIND_CARD_OK)
    {         
        if(MT_DRV_OK == Drv_MT_IC_Preprocess( PIN, RcvData))
        {     
            uint8_t i , index = 0;

            CardType = RcvData[index++];
            CardStatus = RcvData[index++];
            
            for(i = 0; i < 8;i++)
            {
                CardNo[i] =  RcvData[index++];
            }
//            tmpBalance = (RcvData[index]<<24) + (RcvData[index+1]<<16) + (RcvData[index+2]<<8) + RcvData[index+3];
//            index += 4;                   
            
            if(CardType == 0x51)
            {//超级管理卡
                ret = END_OK;
            }
            else if(Record.type == CardType && CardType == ONLINE_CARD)
            {
                ret = END_OK;
            }
            else if(CardType  == BUSINESS_CARD)
            {//一般管理卡
                
                //if(memcmp(g_Chargeing_CardNo[Ch],CardNo,8) == 0)
                {//卡号匹配
                    
                    if(CardStatus == 0x30)
                    {
                        return END_NOT_MATCH;
                    }
                    else if(CardStatus == 0x31 || CardStatus == 0x32)
                    {
                        uint32 balance = 0;
                        DRV_MTStatusTypeDef opt = Drv_MT_IC_StopCharge(Record.Money, &balance);   //此处更新的剩余金额有效   //for test 2017-12-12 11：13
                        if(MT_DRV_OK == opt)
                        {
                             ret = END_OK;
                             dbg_printf(1,"end MT528 0k %d",balance);
                        }
                        else
                        {
                            ret = END_ERR;
                            dbg_printf(1,"Write err");
                        }                                
                    }                           
                }
//                else
//                {
//                    return END_NOT_MATCH;
//                }               
            }
            else if(Record.type != CardType)
            {
                return END_NOT_MATCH;
            }
        }           
    }
    if(END_OK == ret)
    {
        Last = 1;
        dbg_printf(1,"end card ok");
    } 

    return ret;
}

uint8 Get_CardCommStatu_MT628(void)
{
    return (g_CommFault == FALSE )? TRUE:FALSE;
}

uint8 Set_Charge_Card_No_MT628(uint8 *CardNo,uint8 Ch)
{
    
    asc2hex(g_Chargeing_CardNo[Ch],CardNo,16);

    return 0;
}




#endif
