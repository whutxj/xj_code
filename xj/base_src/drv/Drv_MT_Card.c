
#ifndef __DRV_MT_CARD_C
#define __DRV_MT_CARD_C

/******************************************************************************
* Include Files
******************************************************************************/
#include <string.h>
#include "cmsis_os.h"
#include "Drv_MT_Card.h"
#include "drv_usart.h"
#include "bsp_driver_uart.h"
#include <string.h>
#include "Utility.h"
#include "sys_Config.h"
#include <time.h>



/**
 * @brief  数据发送到MT读卡器卡
 * @param  Cmd：命令、CmdPara:命令参数 、pData：数据、DataLen：数据长度
 * @retval 结果状态
 */
static DRV_MTStatusTypeDef MT_Card_Put_Data(uint8_t Cmd, uint8_t CmdPara, uint8_t *pData, uint16_t DataLen)
{
	/**> 依据MT协议 发送长度协议固定，均不超过64 bytes */
	uint8_t SendBuf[64];
    uint16_t index = 0 , Len = 0;
    uint16_t i;

    while( Drv_Uart_Read(MT_COM, SendBuf,1,1) != 0)
    {
        ;
    }
	/**> 命令头 */
    SendBuf[index++] = MT_STX;

	/**> 长度=命令字+命令参数+DataLen */
    SendBuf[index++] = (uint8_t)((DataLen+2)>>8);
    SendBuf[index++] = (uint8_t)(DataLen+2);


	/**> 命令 */
    SendBuf[index++] = (uint8_t)Cmd;

    /**> 命令参数 */
    SendBuf[index++] = (uint8_t)CmdPara;

	/**> 数据 */
    if(NULL != pData)
    {
		for(i=0;i<DataLen;i++)
		{
            SendBuf[index++] = *pData++;
		}
    }
    /**> 帧结束标识 */
    SendBuf[index++] = MT_ETX;
	/**> 异或校验 STX-ETX*/
    Len = index;
    SendBuf[index++] = DataXorCheck(SendBuf,Len);

	/**> 发送数据 */



	Drv_Uart_Write(MT_COM, SendBuf, index,0);
	return MT_DRV_OK;

}

static uint16 MT_Card_Parse(uint8 *pData,int timeout)
{
    uint16 i, rcvLen = 0, dataLen,tmpLen = 0;
    uint8 rcvBuf[64];
    uint8 buf[64];
    uint8 norData;
    uint32 currentTime;
	uint32 startTime = xTaskGetTickCount();

    memset(buf,0,sizeof(buf));

    do
    {
        if((tmpLen = Drv_Uart_Read(MT_COM , buf, sizeof(buf),200)) > 0)
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

    return FALSE;
}

/**
 * @brief  获取MT卡数据
 * @param  Cmd：命令、CmdPara ：命令参数 、pData：数据、DataLen：数据长度
 * @retval 结果状态
 */
static DRV_MTStatusTypeDef MT_Card_Get_Data(uint8_t Cmd, uint8_t CmdPara, uint8_t *StatusWord , uint8_t *pData , uint16_t DataLen)
{
	uint8_t rcvBuf[64]= {0};
	//uint8_t Check;
	uint16_t i,rcvLen;
	//uint8_t ret;
    //uint8 lenth = 0;
    int timeOut = 0;

    if(CmdPara == CMD_FIND_CARD_MT)
    {
        timeOut = 100;
    }
    else
    {
        timeOut = 600;
    }

    if(MT_Card_Parse(rcvBuf,timeOut))
    {
        *StatusWord = rcvBuf[5];
        rcvLen = rcvBuf[2] + 5;
        for(i=6;i<(rcvLen-1);i++)
        {
            *pData++ = rcvBuf[i];
        }
        return MT_DRV_OK;
    }
    return MT_DRV_TIMEOUT;

}

/**
 * @brief  寻卡
 * @param
 * @retval 结果状态
 */
DRV_MTStatusTypeDef Drv_MT_IC_FindCard()
{
    uint8_t RcvData[24], DataBuf , StatusWord;
	DRV_MTStatusTypeDef Status = MT_DRV_ERR;

    /**> 发送寻卡指令 */
    if(MT_DRV_OK == MT_Card_Put_Data(TYPE_51 , CMD_FIND_CARD_MT, &DataBuf, 0))
	{
		/**> 读寻卡返回数据 */
		Status = MT_Card_Get_Data(TYPE_51, CMD_FIND_CARD_MT, &StatusWord , RcvData , 10);
		if(MT_DRV_OK == Status)
		{
			if(CMD_SUCCESS == StatusWord)
			{
				if(SYSTEM_CARD == RcvData[0] && SYSTEM_CARD == RcvData[1])
                {
                    Status = MT_DRV_OK; /**> 系统卡*/
                }else if(NOT_SYSTEM_CARD == RcvData[0]/* && NOT_SYSTEM_CARD == RcvData[1]*/)
                {
                    Status = MT_DRV_CARD_NOT_MATCH;/**> 卡不匹配  非系统卡*/
                }else
                {
                    Status = MT_DRV_ERR;
                }
			}else
            {
                Status = MT_DRV_ERR;
            }
		}
	}
	else
	{
		Status = MT_DRV_ERR;
	}

	/**> 返回状态 */
	return Status;
}

/**
 * @brief  预处理
 * @param  buf：对应数据段
 * @retval 结果状态
 */

DRV_MTStatusTypeDef Drv_MT_IC_Preprocess( uint8_t *pPin, uint8_t *buf)
{
    uint8_t DataBuf[48] , index = 0 , StatusWord = 0;
	DRV_MTStatusTypeDef Status;

    time_t sec = 0;
    struct tm *tempDateTypeDef;
    sec = time(NULL);
    tempDateTypeDef = gmtime(&sec);

    /**> 系统时间 */
	DataBuf[index++] = 0x20;
    DataBuf[index++] =  ByteToBcd(tempDateTypeDef->tm_year - 100);
    DataBuf[index++] =  ByteToBcd(tempDateTypeDef->tm_mon + 1);
	DataBuf[index++] =  ByteToBcd(tempDateTypeDef->tm_mday);
    DataBuf[index++] =  ByteToBcd(tempDateTypeDef->tm_hour);
    DataBuf[index++] =  ByteToBcd(tempDateTypeDef->tm_min);
    DataBuf[index++] =  ByteToBcd(tempDateTypeDef->tm_sec);

    DataBuf[index++] = pPin[0];
    DataBuf[index++] = pPin[1];
    DataBuf[index++] = pPin[2];
	/**> 发送预处理数据帧 */
    if(MT_DRV_OK == MT_Card_Put_Data(TYPE_51 , CMD_PRE_PROCESS, DataBuf, 10))
	{
        memset(DataBuf , 0 , sizeof(DataBuf));
        osDelay(50);  //读卡器在收到数据后10ms后返回数据
		/**> 读返回数据 */
		Status = MT_Card_Get_Data(TYPE_51, CMD_PRE_PROCESS, &StatusWord , DataBuf , 28);
		if(MT_DRV_OK == Status)
		{
            if(CMD_SUCCESS == StatusWord)
            {
                /**> 取数据段 */
                memcpy(buf, DataBuf, 20);
                Status = MT_DRV_OK;
            }else
            {
                Status = MT_DRV_ERR;
            }
		}else
        {
            Status = MT_DRV_ERR;
        }
	}
	else
	{
		Status = MT_DRV_ERR;
	}
	/**> 返回状态 */
	return Status;
}

/**
 * @brief  开始加电
 * @param  pCardBalance：余额
 * @retval 结果状态
 */
DRV_MTStatusTypeDef Drv_MT_IC_StartCharge(uint32_t *pCardBalance)
{
    uint8_t DataBuf[20] ,StatusWord, index = 0;
    DRV_MTStatusTypeDef Status;
    time_t sec = 0;
    struct tm *tempDateTypeDef;
    sec = time(NULL);
    tempDateTypeDef = gmtime(&sec);

    /**> 系统时间 */
	DataBuf[index++] = 0x20;
    DataBuf[index++] =  ByteToBcd(tempDateTypeDef->tm_year - 100);
    DataBuf[index++] =  ByteToBcd(tempDateTypeDef->tm_mon + 1);
	DataBuf[index++] =  ByteToBcd(tempDateTypeDef->tm_mday);
    DataBuf[index++] =  ByteToBcd(tempDateTypeDef->tm_hour);
    DataBuf[index++] =  ByteToBcd(tempDateTypeDef->tm_min);
    DataBuf[index++] =  ByteToBcd(tempDateTypeDef->tm_sec);


    /**> 发送开始加电指令 */
    if(MT_DRV_OK == MT_Card_Put_Data(TYPE_51 , CMD_START_CHARGE, DataBuf, 0x07))
	{
		/**> 读返回数据 */
		memset(DataBuf , 0 , sizeof(DataBuf));
		/**> 读返回数据 */
        osDelay(50);
		Status = MT_Card_Get_Data(TYPE_51, CMD_START_CHARGE, &StatusWord , DataBuf , 12);
		if(MT_DRV_OK == Status)
		{
            if(CMD_SUCCESS == StatusWord)
            {
                /**> 取数据段 */
                *pCardBalance = (DataBuf[0] << 24) + (DataBuf[1] << 16) + (DataBuf[2] << 8) + DataBuf[3];
                Status = MT_DRV_OK;
            }else
            {
                Status = MT_DRV_ERR;
            }
		}else
        {
            Status = MT_DRV_ERR;
        }
	}
	else
	{
		Status = MT_DRV_ERR;
	}

	/**> 返回状态 */
	return Status;
}

/**
 * @brief  结束加电
* @param  sumConsumption:消费金额 pCardBalance：卡内余额
 * @retval 结果状态
 */
DRV_MTStatusTypeDef Drv_MT_IC_StopCharge(uint32_t sumConsumption , uint32_t *pCardBalance)
{
    uint8_t DataBuf[32],index = 0 , StatusWord;
    DRV_MTStatusTypeDef Status;
    time_t sec = 0;
    struct tm *tempDateTypeDef;
    sec = time(NULL);
    tempDateTypeDef = gmtime(&sec);

    /**> 数据填充 */
    DataBuf[index++] = sumConsumption >> 24;
    DataBuf[index++] = sumConsumption >> 16;
    DataBuf[index++] = sumConsumption >> 8;
    DataBuf[index++] = sumConsumption ;

	DataBuf[index++] = 0x20;
    DataBuf[index++] =  ByteToBcd(tempDateTypeDef->tm_year - 100);
    DataBuf[index++] =  ByteToBcd(tempDateTypeDef->tm_mon + 1);
	DataBuf[index++] =  ByteToBcd(tempDateTypeDef->tm_mday);
    DataBuf[index++] =  ByteToBcd(tempDateTypeDef->tm_hour);
    DataBuf[index++] =  ByteToBcd(tempDateTypeDef->tm_min);
    DataBuf[index++] =  ByteToBcd(tempDateTypeDef->tm_sec);


    /**> 发送结束加电数据帧 */
    if(MT_DRV_OK == MT_Card_Put_Data(TYPE_51 , CMD_STOP_CHARGE, DataBuf, 0x0D))
	{
		/**> 读返回数据 */
		memset(DataBuf , 0 , sizeof(DataBuf));
		/**> 读返回数据 */
        //osDelay(200);
		Status = MT_Card_Get_Data(TYPE_51, CMD_STOP_CHARGE, &StatusWord , DataBuf , 12);
		if(MT_DRV_OK == Status)
		{
            if(CMD_SUCCESS == StatusWord)
            {
                /**> 取数据段 */
                *pCardBalance = (DataBuf[0] << 24) + (DataBuf[1] << 16) + (DataBuf[2] << 8) + DataBuf[3];
                Status = MT_DRV_OK;
            }else if(CMD_MAC3_CHECK_FAIL == StatusWord)
            {
                Status = MT_MAC3_CHECK_FAIL;
            }else
            {
                Status = MT_DRV_ERR;
            }
		}else
        {
            Status = MT_DRV_ERR;
        }
	}
	else
	{
		Status = MT_DRV_ERR;
	}

	/**> 返回状态 */
	return Status;
}

/**
 * @brief  补充交易
* @param  buf:返回的数据段
 * @retval 结果状态
 */
DRV_MTStatusTypeDef Drv_MT_IC_SupplyTrade(uint8_t *buf)
{
    uint8_t DataBuf[64] , StatusWord;
    DRV_MTStatusTypeDef Status;

	/**> 发送防冲突指令 */
    if(MT_DRV_OK == MT_Card_Put_Data(TYPE_51 , CMD_SUPPLY_TRADE, DataBuf, 0))
	{
        memset(DataBuf , 0 , sizeof(DataBuf));
		/**> 读返回数据 */
		Status = MT_Card_Get_Data(TYPE_51, CMD_SUPPLY_TRADE, &StatusWord , DataBuf , 50);
		if(MT_DRV_OK == Status)
		{
            if(CMD_SUCCESS == StatusWord)
            {
                /**> 取数据段 */
                memcpy(buf , &DataBuf[6] , 42);
                Status = MT_DRV_OK;
            }else if(CMD_PROCESS_FAIL == StatusWord)
            {
                Status = MT_SUPPLY_FAIL;
            }else
            {
                Status = MT_DRV_ERR;
            }
		}
	}
	else
	{
		Status = MT_DRV_ERR;
	}

	/**> 返回状态 */
	return Status;
}

#endif /* __DRV_MT_CARD_C */

/******************************************************************************
*                            end of file
******************************************************************************/
