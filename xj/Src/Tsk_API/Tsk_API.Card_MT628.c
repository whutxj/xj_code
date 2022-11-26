
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

static uint8 g_Chargeing_CardNo[2][8];//�������еĿ���
//static uint8 g_CommTick = 0;
static uint8 g_CommFault = 0;
static char Last = 0;//���ڼ��һֱˢ���Ͳ�����

#define INTERVAL 6

#define CARD_NUM_LENGTH			8			/**< ���ų��� ��λbyte */ 
#define COMM_RETRY_NUM			6			/**< ��ͨ�����Դ��� */ 

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
        {//�������
            
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
��    �� : MTd��һ�γ��ˢ������
������� : Ch ,�Ǹ�ͨ��

������� :PCardData  ������
��    �� : START_CRG_OPT_CARD����
    
��    �� :lx

����ʱ�� :17-1-4
*********************************************************/

START_CRG_OPT_CARD MT628_Start_Charge_Card(CARD_DATA *PCardData,uint8 Ch)
{
    uint8 CardType = 0;
    uint8_t CardID[4],PIN[3],LockStatus,LockStatusConform;
    int Balance = 0;//�ָ���������
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
            {//һ���û���

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
                            
                            /**< ��ʼ�ӵ� */
                            if(MT_DRV_OK == Drv_MT_IC_StartCharge(&Balance))    //������ȷ�Ͽ���״̬   ���δ�������������  2017-12-12 10:13
                            {   
                                /**< start Ԥ����   2017-12-12 10:13*/

                                dbg_printf(1,"card MT628 money %d",Balance);

                                ret =   OPT_OK; 
                                break;                                
 
                                /**< end Ԥ����   2017-12-12 10:13*/
                            }
                            else
                            {
                                ret = OPT_ERR;   //������ӵ�
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
            {//��������
                ret =   OPT_OK;
                PCardData->CardType = SUPER_MANAGE;
            }
            else if(CardType == ONLINE_CARD)
            {//���߿�
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
//��    �� : MTЭ�鲹�佻��Ѱ������
//������� : ��

//������� :cardType ������
//��    �� :TRUE:���ս�����ȷ 
//          FALSE :����
//��    �� :lx

//����ʱ�� :17-1-4
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
//            {//�������
//                memcpy(cardType,RcvData + 1,2);
//            }

//            return FIND_CARD_OK;
//        }

//        return FIND_CARD_NONE;
//    }   
//    return FIND_CARD_ERR;  
//}

/********************************************************
��    �� : MT�ڶ��ν������ˢ������
������� : Ch ,�Ǹ�ͨ��
            Record д�뵽������ļ�¼����
������� :��
��    �� : END_OPT_CARD����
    
��    �� :lx

����ʱ�� :17-1-4
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
            {//��������
                *Balance = 0;
                ret = END_OK;
            }
            else if(Record.type == CardType && CardType == ONLINE_CARD)
            {
                ret = END_OK;
            }
            else if(CardType  == BUSINESS_CARD)
            {//һ�����
                
                if(memcmp(g_Chargeing_CardNo[Ch],CardNo,8) == 0)
                {//����ƥ��
                    
                    if(CardStatus == 0x30)
                    {
                        return END_NOT_MATCH;
                    }
                    else if(CardStatus == 0x31 || CardStatus == 0x32)
                    {
                        uint32 balance = 0;
                        DRV_MTStatusTypeDef opt = Drv_MT_IC_StopCharge(Record.Money, &balance);   //�˴����µ�ʣ������Ч   //for test 2017-12-12 11��13
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
��    �� : MT�ڶ��β��۳��ˢ������
������� : Ch ,�Ǹ�ͨ��
            Record д�뵽������ļ�¼����
������� :��
��    �� : END_OPT_CARD����
    
��    �� :lx

����ʱ�� :17-1-4
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
            {//��������
                ret = END_OK;
            }
            else if(Record.type == CardType && CardType == ONLINE_CARD)
            {
                ret = END_OK;
            }
            else if(CardType  == BUSINESS_CARD)
            {//һ�����
                
                //if(memcmp(g_Chargeing_CardNo[Ch],CardNo,8) == 0)
                {//����ƥ��
                    
                    if(CardStatus == 0x30)
                    {
                        return END_NOT_MATCH;
                    }
                    else if(CardStatus == 0x31 || CardStatus == 0x32)
                    {
                        uint32 balance = 0;
                        DRV_MTStatusTypeDef opt = Drv_MT_IC_StopCharge(Record.Money, &balance);   //�˴����µ�ʣ������Ч   //for test 2017-12-12 11��13
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
