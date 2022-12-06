
#ifndef _TSK_API_CARD_C
#define _TSK_API_CARD_C

/******************************************************************************
* Include Files
******************************************************************************/
#include "Tsk_API_Card.h"
#include "drv_usart.h"
#include "bsp_driver_uart.h"
#include <string.h>
#include "Utility.h"
#include "sys_Config.h"
#include <time.h>
#include "Tsk_JWT.h"

#define CARD_PORT  COM5

#define SEC_INDEX   1

#define BLOCK_INDEX_CARD_NO  0
#define BLOCK_INDEX_BANLANCE 1
#define BLOCK_INDEX_STATU    2

#define MAX_FAULT_TIME     20

#define FIND_M1 0x3430
#define GET_NO  0x3431
#define AUTH_KEYA   0x3432
#define AUTH_KEYB  0x3439
#define READ_BLOCK 0x3433
#define WRITE_BLOCK 0x3434
#define DEC_VAL 0x3438
#define INIT_VAL 0x3436
#define ADD_VAL 0x3435

#define M1_DBG 1
#define  INTERVAL  8
/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/

static uint8 g_Chargeing_CardNo[2][8];//充电过程中的卡号
static uint8 g_CommTick = 0;
static uint8 g_CommFault = 0;
static char Last = 0;//用于检测一直刷卡就不动作
CARD_DATA card_data[2];

/******************************************************************************
* Static Function Prototype Definitions
******************************************************************************/
extern uint8 M2_Search_Card(CARD_DATA *PCardData);
extern START_CRG_OPT_CARD M2_Start_Charge_Card(CARD_DATA *PCardData,uint8 Ch);
extern END_OPT_CARD M2_End_Charge_Card(CARD_RECORD Record,int Ch);
extern uint8 Get_CardCommStatu_M2(void);
extern uint8 M2_Set_ReadWrite_Mode(void);
extern END_OPT_CARD M2_Add_Charge_Card(CARD_RECORD Record,int Ch);
extern void M2_Card_Read_init(void);
extern uint8 Set_Charge_Card_No_M2(uint8 *CardNo,uint8 Ch);

/********************************************************
描    述 : MT协议封装发送数据帧函数

输入参数 : SendBuf:包缓冲去
            Cmd 操作命令
            pData 数据域
            Len 数据域 长度

输出参数 :无
返    回 :-1 发送出错  1 发送成功

作    者 :

创建时间 :18-06-07
*********************************************************/

static uint32 MT_Card_Pack(uint8 *Send,uint16 CMD,uint8 *data,uint8 len)
{
    int index = 0;
    uint8 crc = 0;

    Send[index++] = 0x02;
    Send[index++] = ((len + 2) >> 8);
    Send[index++] = ((len + 2));

    Send[index++] = (CMD >> 8);
    Send[index++] = (CMD & 0xff);

    if(len > 0)
    {
        memcpy(Send+index,data,len);
        index += len;
    }
    Send[index++] = 0x03;

    crc = DataXorCheck(Send,index);
    Send[index++] = crc;

    return index;
}

/********************************************************
描    述 : MT协议收数据解析
输入参数 : 无

输出参数 :pData 数据域
返    回 :TRUE:接收解析正确
          FALSE :错误
作    者 :

创建时间 :18-06-07
*********************************************************/

static uint16 MT_Card_Parse(uint8 *pData,int timeout)
{
    uint16 i, rcvLen = 0, dataLen,tmpLen = 0;
    uint8 rcvBuf[32];
    uint8 buf[32];
    uint8 norData;
    uint32 currentTime;
	uint32 startTime = xTaskGetTickCount();

    memset(buf,0,sizeof(buf));

    do
    {
        if((tmpLen = Drv_Uart_Read(CARD_PORT , buf, sizeof(buf),100)) > 0)
        {
        	dbg_printf(0,"card rcv %d",tmpLen);

            for(i = 0; i < tmpLen; i++)
            {

                if(rcvLen == 0)
                {
                    if(buf[i] == 0X02)
                    {
                        rcvBuf[rcvLen++] = buf[i];
                        continue;
                    }
                }
                if((rcvLen >= 1)&&(rcvLen<=2))
                {
                    rcvBuf[rcvLen++] = buf[i];
                    continue;
                }
                if(rcvLen >= 3)
                {
                    dataLen = rcvBuf[1]*256+rcvBuf[2];
                    rcvBuf[rcvLen++] = buf[i];
                    if(rcvLen >= (dataLen+5))
                    {
                        norData = DataXorCheck(rcvBuf,dataLen+4);
                        if((norData == rcvBuf[dataLen+4])&&(rcvBuf[dataLen+3]==0X03))
                        {
                            if(rcvLen < sizeof(buf))
                            {
                                memcpy(pData, &rcvBuf[0], rcvLen);
                                g_CommTick = 0;     //clear
                                g_CommFault = FALSE;//clear
                                return TRUE;
                            }
                        }
                        dbg_printf(1,"crc err  %d",rcvLen);
                        return FALSE;
                    }
                }
            }

		}
        osDelay(10);
        currentTime= xTaskGetTickCount();
    }while((currentTime - startTime)<=timeout);

    dbg_printf(1,"card err rcv %d",rcvLen);

    if(g_CommTick > MAX_FAULT_TIME)
    {
        g_CommTick = MAX_FAULT_TIME;
        g_CommFault = TRUE;
    }
    else
    {
        g_CommFault = FALSE;
    }
    g_CommTick++;
    return FALSE;
}

/********************************************************
描    述 : MT协议寻卡操作
输入参数 : 无

输出参数 :cardType 卡类型
返    回 :TRUE:接收解析正确
          FALSE :错误
作    者 :

创建时间 :18-06-07
*********************************************************/

static uint8 MT_Find_Card(uint16 *cardType)
{
    uint8 SendBuf[32] = {0};
    uint8 Data[16] = {0};
    uint8 RcvData[16] = {0};
    int len = 0;

	static uint32 tick = 0;
    static uint32 last_tick = 0;
    if(Tick_Out(&tick,1000))
    {
        Tick_Out(&tick,0);
    }
    else
    {
        return FIND_CARD_ERR;
    }

    len = MT_Card_Pack(SendBuf,FIND_M1,Data,0);
    Drv_Uart_Write(CARD_PORT,(const uint8 *)SendBuf,len,1);

    if(MT_Card_Parse(RcvData,40) == TRUE)
    {
        if(RcvData[5] == 0x59 && Last == 0)
        {
            if(cardType != NULL)
            {//参数检测
                memcpy(cardType,RcvData + 1,2);
            }
			//Last = 1;
            last_tick = time(NULL);
            return FIND_CARD_OK;
        }
		else
		{
            if(RcvData[5] != 0x59 )
            {
                Last = 0;
            }
            if(time(NULL) - last_tick > INTERVAL)
            {
                Last = 0;
            }
		}
        return FIND_CARD_NONE;
    }
    else
    {
        dbg_printf(M1_DBG,"find card err");
    }
    return FIND_CARD_ERR;
}

/********************************************************
描    述 : MT协议验证密码
输入参数 : key 密码
           block 块号

输出参数 :cardID 卡ID号
返    回 :TRUE:接收解析正确
          FALSE :错误
作    者 :

创建时间 :18-06-07
*********************************************************/

static uint8 MT_Auth_Opt(uint8 *key,int block)
{
    uint8 SendBuf[32] = {0};
    uint8 Data[16] = {0};
    uint8 RcvData[16] = {0};
    int len = 0;

    Data[0] = block;

    memcpy(Data + 1,key,6);

    len = MT_Card_Pack(SendBuf,AUTH_KEYA,Data,7);
    Drv_Uart_Write(CARD_PORT,(const uint8 *)SendBuf,len,1);
    if(MT_Card_Parse(RcvData,400) == TRUE)
    {
        if(RcvData[6] == 0X59)
        {
            return TRUE;
        }

    }
    else
    {
        dbg_printf(M1_DBG,"Auth card err");
    }
    return FALSE;
}


/********************************************************
描    述 : MT协议读卡操作
输入参数 : key 密码  sec 扇区
           block 块号

输出参数 :cardID 卡ID号
返    回 :TRUE:接收解析正确
          FALSE :错误
作    者 :

创建时间 :18-06-07
*********************************************************/

static uint8 MT_Read_Opt(uint8 *pData,int sec,int block)
{
    uint8 SendBuf[32] = {0};
    uint8 Data[16] = {0};
    uint8 RcvData[32] = {0};
    int len = 0;

    Data[0] = sec;
    Data[1] = block;
    len = MT_Card_Pack(SendBuf,READ_BLOCK,Data,2);

    Drv_Uart_Write(CARD_PORT,(const uint8 *)SendBuf,len,1);
    if(MT_Card_Parse(RcvData,400) == TRUE)
    {
        if(RcvData[7] == 0x59)
        {
            memcpy(pData,RcvData + 8,16);

            return TRUE;
        }

    }
    else
    {
        dbg_printf(M1_DBG,"read card err");
    }
    return FALSE;
}


/********************************************************
描    述 : MT协议读卡操作
输入参数 : key 密码
           block 块号

输出参数 :cardID 卡ID号
返    回 :TRUE:接收解析正确
          FALSE :错误
作    者 :

创建时间 :18-06-07
*********************************************************/

static uint8 MT_Write_Opt(uint8 *pData,int sec,int block)
{
    uint8 SendBuf[32] = {0};
    uint8 Data[20] = {0};
    uint8 RcvData[32] = {0};
    int len = 0;

    Drv_Uart_Read(CARD_PORT , RcvData, sizeof(RcvData),100);

    Data[0] = sec;
    Data[1] = block;
    memcpy(Data + 2,pData,16);
    len = MT_Card_Pack(SendBuf,WRITE_BLOCK,Data,18);

    Drv_Uart_Write(CARD_PORT,(const uint8 *)SendBuf,len,1);

    if(MT_Card_Parse(RcvData,500) == TRUE)
    {
        if(RcvData[7] == 0x59)
        {
            if(MT_Read_Opt(RcvData,sec,block))
            {
                if(memcmp(pData,RcvData,16) == 0)
                {
                    return TRUE;
                }
            }
            return FALSE;
        }

    }
    else
    {
        dbg_printf(M1_DBG,"write card err");
    }
    return FALSE;
}

/********************************************************
描    述 : MT协议初始化值操作
输入参数 : key 密码
           block 块号

输出参数 :cardID 卡ID号
返    回 :TRUE:接收解析正确
          FALSE :错误
作    者 :

创建时间 :18-06-07
*********************************************************/

uint8 MT_Init_Block(uint32 iData,int sec,int block)
{
    uint8 SendBuf[32] = {0};
    uint8 Data[16] = {0};
    uint8 RcvData[16] = {0};
    int len = 0;
    Data[0] = sec;
    Data[1] = block;
    memcpy(Data + 2,(uint8*)&iData,4);
    len = MT_Card_Pack(SendBuf,INIT_VAL,Data,6);
    Drv_Uart_Write(CARD_PORT,(const uint8 *)SendBuf,len,1);
    if(MT_Card_Parse(RcvData,400) == TRUE)
    {
        if(RcvData[0] == 0x59)
        {
            return TRUE;
        }

    }
    return FALSE;
}

/********************************************************
描    述 : MT协议减值操作
输入参数 : key 密码
           block 块号

输出参数 :cardID 卡ID号
返    回 :TRUE:接收解析正确
          FALSE :错误
作    者 :

创建时间 :18-06-07
*********************************************************/

static uint8 MT_Dec_Value(uint32 iData,int sec,int block)
{
    uint8 SendBuf[32] = {0};
    uint8 Data[16] = {0};
    uint8 RcvData[16] = {0};
    int len = 0;
    co_dword dword;

    Data[0] = sec;
    Data[1] = block;
    //memcpy(Data + 2,(uint8*)&iData,4);
    dword.v = card_data[0].Balance - iData;
    Data[2] = dword.b[0];
    Data[3] = dword.b[1];
    Data[4] = dword.b[2];
    Data[5] = dword.b[3];
    len = MT_Card_Pack(SendBuf,WRITE_BLOCK,Data,6);
    Drv_Uart_Write(CARD_PORT,(const uint8 *)SendBuf,len,1);
    if(MT_Card_Parse(RcvData,400) == TRUE)
    {
        if(RcvData[7] == 0x59)
        {

            return TRUE;
        }

    }
    else
    {
        dbg_printf(1,"dec val err");
    }
    return FALSE;
}


/********************************************************
描    述 : MT协议减值操作
输入参数 : key 密码
           block 块号

输出参数 :cardID 卡ID号
返    回 :TRUE:接收解析正确
          FALSE :错误
作    者 :

创建时间 :18-06-07
*********************************************************/

uint8 MT_Add_Value(uint32 iData,int sec,int block)
{
    uint8 SendBuf[32] = {0};
    uint8 Data[16] = {0};
    uint8 RcvData[16] = {0};
    int len = 0;

    Data[0] = sec;
    Data[1] = block;
    memcpy(Data + 1,(uint8*)&iData,4);
    len = MT_Card_Pack(SendBuf,ADD_VAL,Data,5);
    Drv_Uart_Write(CARD_PORT,(const uint8 *)SendBuf,len,1);
    if(MT_Card_Parse(RcvData,400) == TRUE)
    {
        if(RcvData[7] == 0x59)
        {
            return TRUE;
        }

    }
    return FALSE;
}

/********************************************************
描    述 : MT协议减值操作
输入参数 : key 密码
           block 块号

输出参数 :cardID 卡ID号
返    回 :TRUE:接收解析正确
          FALSE :错误
作    者 :

创建时间 :18-06-07
*********************************************************/

static uint8 MT_Read_Value(int *iData,int sec,int block)
{

    uint8 RcvData[16] = {0};

    int ret = MT_Read_Opt(RcvData,sec,block);

    if(ret == TRUE)
    {
        //if(RcvData[7] == 0x59)
        {
            memcpy((uint8*)iData,RcvData,4);
            return TRUE;
        }

    }
    else
    {
        dbg_printf(M1_DBG,"read val err");
    }
    return FALSE;
}


/********************************************************
描    述 : MT协议读卡号
输入参数 :

输出参数 :PCardData  卡数据
返    回 :TRUE:接收解析正确
          FALSE :错误
作    者 :

创建时间 :18-06-07
*********************************************************/

uint8 MT_Search_Card(CARD_DATA *PCardData)
{

    uint8 Key[6] = {0};

    Get_Card_Key(Key);

    if(MT_Find_Card(NULL) == FIND_CARD_OK)
    {
         return TRUE;
    }

    return FALSE;
}
/********************************************************
描    述 : MTd第一次充电刷卡操作
输入参数 : Ch ,那个通道

输出参数 :PCardData  卡数据
返    回 : START_CRG_OPT_CARD类型

作    者 :

创建时间 :18-06-07
*********************************************************/

START_CRG_OPT_CARD MT_Start_Charge_Card(CARD_DATA *PCardData,uint8 Ch)
{
    uint8 CardType = 0;
    uint8 Key[6] = {0};
    int Balance = 0;//分负数和正数
    START_CRG_OPT_CARD ret = (START_CRG_OPT_CARD)0;

    uint8 RcvData[16] = {0};

    ret = OPT_ERR;
    Get_Card_Key(Key);

    if(MT_Find_Card(NULL) == FIND_CARD_OK)
    {

        if(MT_Auth_Opt(Key,BLOCK_INDEX_BANLANCE))
        {

            if(MT_Read_Opt(RcvData,SEC_INDEX,BLOCK_INDEX_CARD_NO) == TRUE)
            {
                memcpy(PCardData->Card_no,(uint8 *)RcvData,8);
                CardType = RcvData[8];
                PCardData->CardType = CardType;
                if(CardType  == BUSINESS_CARD)
                {//一般用户卡
                    if(MT_Read_Opt(RcvData,SEC_INDEX,BLOCK_INDEX_STATU) == TRUE)
                    {//读卡状态
                        if(RcvData[0] == STATUS_LOCK)
                        {
                            ret =  OPT_LOCK;
                        }
                        else
                        {

                            if(MT_Read_Value(&Balance,SEC_INDEX,BLOCK_INDEX_BANLANCE) == TRUE)
                            {
                                if(Balance >= MIN_ALLOW_MONEY)
                                {
                                    RcvData[0] = STATUS_LOCK;

                                    MT_Write_Opt(RcvData,SEC_INDEX,BLOCK_INDEX_STATU);

                                    dbg_printf(1,"card M1 money %d",Balance);

                                    card_data[0].Balance = Balance;
                                    PCardData->Balance = Balance;
                                    ret =   OPT_OK;


                                }
                                else
                                {
                                    ret =   OPT_BALANCE_LACK;
                                }
                            }

                        }
                    }
                }
                else if(CardType == SUPER_MANAGE)
                {//超级管理卡
                    ret =   OPT_OK;
                }
                else if(CardType == USR_CARD)
                {//一般管理卡

                    if(MT_Read_Value(&Balance,SEC_INDEX,BLOCK_INDEX_BANLANCE) == TRUE)
                    {
                        if(Balance >= MIN_ALLOW_MONEY)
                        {
                            dbg_printf(1,"card M1 money %d",Balance);

														card_data[0].Balance = Balance;
                            PCardData->Balance = Balance;

                            ret =   OPT_OK;
                        }
                        else
                        {
                            return OPT_BALANCE_LACK;
                        }

                    }
                }
                else if(CardType == ONLINE_CARD)
                {//在线卡
                    ret =   OPT_OK;
                }
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



/********************************************************
描    述 : MT协议补充交易寻卡操作
输入参数 : 无

输出参数 :cardType 卡类型
返    回 :TRUE:接收解析正确
          FALSE :错误
作    者 :

创建时间 :18-06-07
*********************************************************/

static uint8 MT_Find_Card_Add(uint16 *cardType)
{
    uint8 SendBuf[32] = {0};
    uint8 Data[16] = {0};
    uint8 RcvData[16] = {0};
    int len = 0;
    len = MT_Card_Pack(SendBuf,FIND_M1,Data,0);
    Drv_Uart_Write(CARD_PORT,(const uint8 *)SendBuf,len,1);
    if(MT_Card_Parse(RcvData,40) == TRUE)
    {
        if(RcvData[5] == 0x59)
        {
            if(cardType != NULL)
            {//参数检测
                memcpy(cardType,RcvData + 1,2);
            }

            return FIND_CARD_OK;
        }

        return FIND_CARD_NONE;
    }
    return FIND_CARD_ERR;
}

/********************************************************
描    述 : MT第二次结束充电刷卡操作
输入参数 : Ch ,那个通道
            Record 写入到卡里面的记录数据
输出参数 :无
返    回 : END_OPT_CARD类型

作    者 :

创建时间 :18-06-07
*********************************************************/

END_OPT_CARD MT_End_Charge_Card(CARD_RECORD Record,int *Balance,int Ch)
{
    uint8 CardType = 0;
    uint8 Key[6] = {0};

    uint8 CardNo[8] = {0};//bcd 码 8字节
    uint8 RcvData[16] = {0};
    END_OPT_CARD ret = END_ERR;

    Get_Card_Key(Key);

    if(MT_Find_Card(NULL) == FIND_CARD_OK)
    {
        if(MT_Auth_Opt(Key,BLOCK_INDEX_BANLANCE))
        {

            if(MT_Read_Opt(RcvData,SEC_INDEX,BLOCK_INDEX_CARD_NO) == TRUE)
            {
                memcpy(CardNo,RcvData,8);
                CardType = RcvData[8];
                if(CardType == SUPER_MANAGE)
                {//超级管理卡

                    ret = END_OK;
                }
                else if(Record.type == CardType && CardType == ONLINE_CARD)
                {
                    ret = END_OK;
                }
                else if(CardType == USR_CARD || CardType  == BUSINESS_CARD)
                {//一般管理卡

                    if(memcmp(g_Chargeing_CardNo[Ch],CardNo,8) == 0)
                    {//卡号匹配

                        //add
                        RcvData[0] = STATUS_UNLOCK;
                        memcpy(&RcvData[1],(uint8*)&Record,sizeof(CARD_RECORD));

                        if(MT_Write_Opt(RcvData,SEC_INDEX,BLOCK_INDEX_STATU) == TRUE)
                        {
                            if(Record.Money <= 0)
                            {
                                ret = END_OK;
                            }
                            else if(MT_Dec_Value(Record.Money,SEC_INDEX,BLOCK_INDEX_BANLANCE) == TRUE)
                            {
                                ret = END_OK;

                            }
                            else
                            {
                                ret = END_ERR;
                            }
                        }
                        else
                        {
                            dbg_printf(1,"Write err");
                            ret = END_ERR;
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
    }
    if(END_OK == ret)
    {
        Last = 1;
        dbg_printf(1,"end card ok");
        Ctrl_card_charge(Ch, JWT_card_end, CardNo);
        MT_Read_Value(Balance,SEC_INDEX,BLOCK_INDEX_BANLANCE);
    }

    return ret;
}

/********************************************************
描    述 : MT第二次补扣充电刷卡操作
输入参数 : Ch ,那个通道
            Record 写入到卡里面的记录数据
输出参数 :无
返    回 : END_OPT_CARD类型

作    者 :

创建时间 :18-06-07
*********************************************************/

END_OPT_CARD MT_Add_Charge_Card(CARD_RECORD Record,int Ch)
{
    uint8 CardType = 0;
    uint8 Key[6] = {0};

    uint8 CardNo[8] = {0};
    uint8 RcvData[16] = {0};
    END_OPT_CARD ret = END_ERR;
		int Balance = 0;//分负数和正数
    Get_Card_Key(Key);
    if(MT_Find_Card_Add(NULL) == FIND_CARD_OK)
    {
        if(MT_Auth_Opt(Key,BLOCK_INDEX_BANLANCE))
        {

            if(MT_Read_Opt(RcvData,SEC_INDEX,BLOCK_INDEX_CARD_NO) == TRUE)
            {
                memcpy(CardNo,RcvData,8);
                CardType = RcvData[8];
							if(MT_Read_Value(&Balance,SEC_INDEX,BLOCK_INDEX_BANLANCE) == TRUE)
                {
                  dbg_printf(1," card Balance %d",Balance);
                  card_data[0].Balance = Balance; 
									
									if(CardType == USR_CARD || CardType  == BUSINESS_CARD)
	                {//一般管理卡
	                    RcvData[0] = STATUS_UNLOCK;
	                    memcpy(RcvData + 1,(uint8*)&Record,sizeof(CARD_RECORD));
										
	                    if(MT_Write_Opt(RcvData,SEC_INDEX,BLOCK_INDEX_STATU))
	                    {//卡号匹配
	                    		
	                        if(Record.Money == 0)
	                        {
	                            ret = END_OK;
	                        }
	                        else if(MT_Dec_Value(Record.Money,SEC_INDEX,BLOCK_INDEX_BANLANCE) == TRUE)
	                        {
	                            ret = END_OK;

	                        }
	                    }
	                }
                }
                
            }
        }
    }

    return ret;
}

uint8 Get_CardCommStatu_MT(void)
{
    return (g_CommFault == FALSE )? TRUE:FALSE;
}

uint8 Set_Charge_Card_No_MT(uint8 *CardNo,uint8 Ch)
{

    asc2hex(g_Chargeing_CardNo[Ch],CardNo,16);

    return 0;
}
void MT_Card_Read_init(void)
{
    //
}


uint8 API_Search_Card(CARD_DATA *PCardData)
{
    uint8 type = Get_Card_Read_Type();
    uint8 ret = 0;
    switch(type)
    {
        case CARD_M2:
        {
            ret =  M2_Search_Card(PCardData);
            break;
        }
        case CARD_MT:
        {
            ret =  MT_Search_Card(PCardData);
            break;
        }
        case CARD_MT628:
        {
            ret = MT628_Search_Card(PCardData);
        }
        default:
            break;
    }

    return ret;
}

START_CRG_OPT_CARD API_Start_Charge_Card(CARD_DATA *PCardData,uint8 Ch)
{
    uint8 type = Get_Card_Read_Type();
    START_CRG_OPT_CARD ret = (START_CRG_OPT_CARD )OPT_ERR;
    switch(type)
    {
        case CARD_M2:
        {
            ret =  M2_Start_Charge_Card(PCardData,Ch);
            break;
        }
        case CARD_MT:
        {
            ret =  MT_Start_Charge_Card(PCardData,Ch);
            break;
        }
        case CARD_MT628:
        {
            ret = MT628_Start_Charge_Card(PCardData,Ch);
            break;
        }
        default:
            ret = (START_CRG_OPT_CARD )OPT_ERR;
            break;
    }

    return ret;
}

END_OPT_CARD API_End_Charge_Card(CARD_RECORD Record,int *pBalence,int Ch)
{
    uint8 type = Get_Card_Read_Type();
    END_OPT_CARD ret = (END_OPT_CARD )OPT_ERR;
    switch(type)
    {
        case CARD_M2:
        {
            ret =  M2_End_Charge_Card(Record,Ch);
            break;
        }
        case CARD_MT:
        {
            ret =  MT_End_Charge_Card(Record,pBalence,Ch);
            break;
        }
        case CARD_MT628:
        {
            ret = MT628_End_Charge_Card(Record,pBalence,Ch);
            break;
        }
        default:
            ret = (END_OPT_CARD )OPT_ERR;
            break;
    }

    return ret;
}

uint8 Get_CardCommStatu(void)
{
    uint8 type = Get_Card_Read_Type();
    uint8 ret = 0;
    switch(type)
    {
        case CARD_M2:
        {
            ret =  Get_CardCommStatu_M2();
            break;
        }
        case CARD_MT:
        {
            ret =  Get_CardCommStatu_MT();
            break;
        }
        case CARD_MT628:
        {
            ret = Get_CardCommStatu_MT628();
            break;
        }
        default:
            break;
    }

    return ret;
}

END_OPT_CARD API_Add_Charge_Card(CARD_RECORD Record,int Ch)
{
    uint8 type = Get_Card_Read_Type();
    END_OPT_CARD ret = (END_OPT_CARD )0;;
    switch(type)
    {
        case CARD_M2:
        {
            ret =  M2_Add_Charge_Card(Record,Ch);
            break;
        }
        case CARD_MT:
        {
            ret =  MT_Add_Charge_Card(Record,Ch);
            break;
        }
        case CARD_MT628:
        {
            ret = MT628_Add_Charge_Card(Record,Ch);
            break;
        }
        default:
            break;
    }
    return ret;
}

void Card_Read_init(void)
{
    uint8 type = Get_Card_Read_Type();

    switch(type)
    {
        case CARD_M2:
        {
            M2_Card_Read_init();
            break;
        }
        case CARD_MT:
        {
            MT_Card_Read_init();
            break;
        }
        default:
            break;
    }

}
uint8 Set_Charge_Card_No(uint8 *CardNo,uint8 Ch)
{
    uint8 type = Get_Card_Read_Type();
    uint8 ret = 0;
    switch(type)
    {
        case CARD_M2:
        {
            ret =  Set_Charge_Card_No_M2(CardNo,Ch);
            break;
        }
        case CARD_MT:
        {
            ret =  Set_Charge_Card_No_MT(CardNo,Ch);
            break;
        }
        case CARD_MT628:
        {
            ret =  Set_Charge_Card_No_MT628(CardNo,Ch);
            break;
        }
        default:
            break;
    }
    return ret;
}
#endif
