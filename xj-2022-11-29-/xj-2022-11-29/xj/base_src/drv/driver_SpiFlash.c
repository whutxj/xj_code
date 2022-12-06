
#ifndef __DRIVER_SPIFLASH_C
#define __DRIVER_SPIFLASH_C

/******************************************************************************
* Include Files
******************************************************************************/
#include "main.h"

#include "driver_SpiFlash.h"
#include "CRC16.h"
#include <string.h>
#include <stdlib.h>
#include "Utility.h"
#define DBG_DRV_SPI_FLASH   1
/******************************************************************************
* Variable Definitions
******************************************************************************/


/******************************************************************************
* Static Function Prototype Definitions
******************************************************************************/
static SPIFLASH_RecordState IS_Record_Valid(uint32_t ReadAddr, uint16_t TotalLength, uint16_t UserLength );
static void SPIFLASH_SECTOR_BufferWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);


static void *SPIFLASH_SECTOR_Malloc(uint32 size)
{
    void *p = NULL;
    taskENTER_CRITICAL();
    p = malloc(size);
    
    taskEXIT_CRITICAL();
    return p;
    
}

static void SPIFLASH_SECTOR_Free(void *p)
{
    taskENTER_CRITICAL();
    if(p != NULL)
    {
        free(p);
        p = NULL;
    }
    taskEXIT_CRITICAL();
}
/******************************************************************************
* Function		: 	SPIFLASH_OperateState SPIFLASH_SECTOR_Get_HeadInfo(uint32_t SectorAddr, SPIFLASH_SECTOR_HeadInfoDef* HeadInfo)
*
* Parameter		:	SectorAddr	-扇区首地址
*					HeadInfo	-头部信息指针
*
* Return		:	获取结果：	SPIFLASH_ERR-失败；SPIFLASH_OK-成功
*
* Description	:	Get Head Infomation of the Sector.
*
******************************************************************************/
SPIFLASH_OperateState SPIFLASH_SECTOR_Get_HeadInfo(uint32_t SectorAddr, SPIFLASH_SECTOR_HeadInfoDef* HeadInfo)
{
	BSP_SPIFLASH_BufferRead(HeadInfo->CommonInfo.Para,SectorAddr,SECTOR_HEADSIZE_COMMON);
	if(CRC16LUT(HeadInfo->CommonInfo.Para,SECTOR_HEADSIZE_COMMON))
	{
		memset(HeadInfo->CommonInfo.Para,0xFF,sizeof(HeadInfo->CommonInfo.Para));
		return SPIFLASH_ERR;
	}
	else
	{
		BSP_SPIFLASH_BufferRead(HeadInfo->UserInfo.Parameter,SectorAddr+SECTOR_HEADSIZE_USER,SECTOR_HEADSIZE_USER);
		return SPIFLASH_OK;
	}
}

/******************************************************************************
* Function		: 	SPIFLASH_RecordState IS_Record_Valid(uint32_t ReadAddr, uint16_t TotalLength, uint16_t UserLength )
*
* Parameter		:	ReadAddr   	- 记录地址
					TotalLength	- 记录总长（包含重写CRC码）
					UserLength 	- 用户数据长度
*
* Return		:	校验结果
*
* Description	:	校验记录是否有效：从指定地址中获取某条记录，根据重写标志字，
*					计算当前记录长度，返回CRC16校验结果
*
******************************************************************************/
static SPIFLASH_RecordState IS_Record_Valid(uint32_t ReadAddr, uint16_t TotalLength, uint16_t UserLength )
{
	//uint8_t	buf[TotalLength];
    uint8_t	*buf = SPIFLASH_SECTOR_Malloc(TotalLength);
    SPIFLASH_RecordState ret;
	uint16_t OverWriteWord,i,LengthNow;
    if(buf == NULL)
    {
        dbg_printf(DBG_DRV_SPI_FLASH,"IS_Record_Valid SPIFLASH_SECTOR_Malloc err");
        return DATA_INVALID;
    }
	BSP_SPIFLASH_BufferRead(buf,ReadAddr,TotalLength);
	OverWriteWord = ((uint16_t)buf[0])+(((uint16_t)buf[1])<<8);		/*重写标志字*/
	for(i=0;i<16;i++)
	{
		if((OverWriteWord>>i)&0x0001)
		{
			break;
		}
	}
	LengthNow = 2+UserLength+i*2;									/*当前已写的记录长度*/
	if(CRC16LUT(buf,LengthNow))
	{
		ret = DATA_INVALID;										/*无效*/
	}
	else
	{
		ret =  DATA_VALID;											/*有效*/
	}
    
    SPIFLASH_SECTOR_Free(buf);
    
    return ret;
}

/******************************************************************************
* Function		: 	uint16_t SPIFLASH_SECTOR_Get_RecordCnt(uint32_t SectorAddr)
*
* Parameter		:	SectorAddr	- 扇区首地址
*
* Return		:	记录数量
*
* Description	:	获取指定扇区的记录数量
*
******************************************************************************/
uint16_t SPIFLASH_SECTOR_Get_RecordCnt(uint32_t SectorAddr)
{
	SPIFLASH_SECTOR_HeadInfoDef HeadInfo;
	uint32_t ReadAddr,startAddr,startIndex,endIndex,midIndex;
	uint16_t LengthComplete,Cnt;
	SPIFLASH_RecordState ValidFlag1,ValidFlag2;
	
	/*获取扇区头部信息*/
	BSP_SPIFLASH_BufferRead(HeadInfo.CommonInfo.Para,SectorAddr,SECTOR_HEADSIZE_COMMON);
	if(0 == CRC16LUT(HeadInfo.CommonInfo.Para,SECTOR_HEADSIZE_COMMON))
	{
//		/* 计算完整记录长度 */
//		LengthComplete = 2+HeadInfo.CommonInfo.Item.RecordLength+2*HeadInfo.CommonInfo.Item.OverWriteCnt;
		LengthComplete = HeadInfo.CommonInfo.Item.RecordLength_Total;
		
		startAddr = SectorAddr+SECTOR_HEADSIZE;
		startIndex = 0;
		endIndex = MaxCntOfRecord(LengthComplete)-1;
		
		/*查找最末尾的记录索引（折半查找）*/
		while((startIndex+1)<endIndex)
		{
			midIndex = (startIndex+endIndex)/2;
			ReadAddr = startAddr+midIndex*LengthComplete;

			if(DATA_INVALID == IS_Record_Valid(ReadAddr,LengthComplete,HeadInfo.CommonInfo.Item.RecordLength_User))
			{
				endIndex = midIndex;
			}
			else
			{
				startIndex = midIndex;
			}
		}
		
		/*校验最终的两条记录，得到记录条数*/
		ReadAddr = startAddr+startIndex*LengthComplete;
		ValidFlag1 = IS_Record_Valid(ReadAddr,LengthComplete,HeadInfo.CommonInfo.Item.RecordLength_User);
		ReadAddr = startAddr+endIndex*LengthComplete;
		ValidFlag2 = IS_Record_Valid(ReadAddr,LengthComplete,HeadInfo.CommonInfo.Item.RecordLength_User);
		if(ValidFlag1==DATA_INVALID)
		{
			Cnt = startIndex;
		}
		else if(ValidFlag2==DATA_VALID)
		{
			Cnt = endIndex+1;
		}
		else
		{
			Cnt = startIndex+1;
		}
		return Cnt;
	}
	else
	{
		return 0;
	}
}

/******************************************************************************
* Function		: 	uint16_t SPIFLASH_SECTOR_Read_Record
*
* Parameter		:	SectorAddr	- 扇区首地址
*					RecordIndex	- 记录索引
*					buffRead	- 待读数组指针
*
* Return		:	读取状态：	SPIFLASH_ERR-失败；SPIFLASH_OK-成功
*
* Description	:	获取指定扇区的第RecordIndex条记录
*
******************************************************************************/
SPIFLASH_OperateState SPIFLASH_SECTOR_Read_Record(uint32_t SectorAddr, uint16_t RecordIndex, uint8_t* buffRead)
{
	SPIFLASH_SECTOR_HeadInfoDef HeadInfo;
	uint32_t ReadAddr;
	
	if(SPIFLASH_OK == SPIFLASH_SECTOR_Get_HeadInfo(SectorAddr,&HeadInfo))
	{
		ReadAddr = SectorAddr+SECTOR_HEADSIZE + HeadInfo.CommonInfo.Item.RecordLength_Total*RecordIndex;
		if(DATA_VALID == IS_Record_Valid(ReadAddr,HeadInfo.CommonInfo.Item.RecordLength_Total,HeadInfo.CommonInfo.Item.RecordLength_User))
		{
			ReadAddr = ReadAddr+2;
			BSP_SPIFLASH_BufferRead(buffRead,ReadAddr,HeadInfo.CommonInfo.Item.RecordLength_User);
			return SPIFLASH_OK;
		}
		else
		{
			return SPIFLASH_ERR;
		}
	}
	else
	{
		return SPIFLASH_ERR;
	}
	
}

/******************************************************************************
* Function		: 	uint16_t SPIFLASH_SECTOR_Search_Record
*
* Parameter		:	SectorAddr	- 扇区首地址
*					Mask		- 匹配过滤器数组指针：指向数据0-该字节加入搜索范围；1-不加入搜索范围
*					MaskBuff	- 匹配字数组指针：指向搜索的具体数值
*
* Return		:	记录索引值
*
* Description	:	在指定扇区中,根据*Mask中设置的搜索范围，检索与*MaskBuff中数据据匹配的记录，并返回该记录索引
*
******************************************************************************/
uint16_t SPIFLASH_SECTOR_Search_Record(uint32_t SectorAddr, uint8_t* Mask, uint8_t* MaskBuff)
{
	uint16_t RecordCnt,i,j;
	SPIFLASH_SECTOR_HeadInfoDef HeadInfo;
	
	SPIFLASH_SECTOR_Get_HeadInfo(SectorAddr,&HeadInfo);
	uint8_t buff[HeadInfo.CommonInfo.Item.RecordLength_User];
	
	RecordCnt = SPIFLASH_SECTOR_Get_RecordCnt(SectorAddr);
	for(i=0;i<RecordCnt;i++)
	{
		SPIFLASH_SECTOR_Read_Record(SectorAddr,i,buff);
		for(j=0;j<HeadInfo.CommonInfo.Item.RecordLength_User;j++)
		{
			if((*(Mask+j))==0)
			{
				if((*(MaskBuff+j))!=buff[j])
				{
					break;
				}
			}
		}
		if(j==HeadInfo.CommonInfo.Item.RecordLength_User)
		{
			return i;
		}
	}
	return 0xFFFF;
}

/******************************************************************************
* Function		: 	void SPIFLASH_SECTOR_BufferWrite
*
* Parameter		:	pBuffer			- 待写数组指针
*					WriteAddr		- 待写地址
*					NumByteToWrite	- 待写字节数
*
* Return		:	void
*
* Description	:	往主区、备份区中分时写入数据
*
******************************************************************************/
static void SPIFLASH_SECTOR_BufferWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
	BSP_SPIFLASH_BufferWrite(pBuffer, WriteAddr, NumByteToWrite);
	BSP_SPIFLASH_BufferWrite(pBuffer, WriteAddr+SPI_FLASH_SectorSize, NumByteToWrite);
}

/******************************************************************************
* Function		: 	void SPIFLASH_SECTOR_CheckResume
*
* Parameter		:	SectorAddr	- 主扇区首地址
*
* Return		:	void
*
* Description	:	校验主区、备份区中记录数量，进行数据相互恢复操作
*
******************************************************************************/
void SPIFLASH_SECTOR_CheckResume(uint32_t SectorAddr)
{
	uint16_t  Cnt1,Cnt2,i;
//	uint8_t	  Buffer[128];
	uint8_t* Buffer;
	uint32_t  ReadAddr,WriteAddr;
	
	Cnt1 = SPIFLASH_SECTOR_Get_RecordCnt(SectorAddr);
	Cnt2 = SPIFLASH_SECTOR_Get_RecordCnt(SectorAddr+SPI_FLASH_SectorSize);
	
	//if(Cnt1 != Cnt2)
	{
		Buffer = SPIFLASH_SECTOR_Malloc(128);
		if(Cnt1>Cnt2)
		{
			ReadAddr = SectorAddr;
			WriteAddr = SectorAddr+SPI_FLASH_SectorSize;
		}
		else
		{
			ReadAddr = SectorAddr+SPI_FLASH_SectorSize;
			WriteAddr = SectorAddr;
		}
        //参数检测
        if(Buffer == NULL)
        {
            dbg_printf(DBG_DRV_SPI_FLASH,"CheckResume SPIFLASH_SECTOR_Malloc err");
            return;
        }
		BSP_SPIFLASH_SectorErase(WriteAddr);
		for(i=0;i<(SPI_FLASH_SectorSize/128);i++)
		{
			BSP_SPIFLASH_BufferRead(Buffer,ReadAddr,128);
			BSP_SPIFLASH_BufferWrite(Buffer,WriteAddr,128);
			ReadAddr += 128;
			WriteAddr += 128;
		}
		SPIFLASH_SECTOR_Free(Buffer);
	}
}

/******************************************************************************
* Function		: 	SPIFLASH_OperateState SPIFLASH_SECTOR_Write_Record
*
* Parameter		:	SectorAddr	- 主扇区首地址
*					RecordIndex	- 记录索引
*					buffWrite	- 待写记录数组指针
*
* Return		:	存储结果
*
* Description	:	将记录根据重写标志加CRC校验码后写入指定地址。适用于可重写的记录类型。
*
******************************************************************************/
SPIFLASH_OperateState SPIFLASH_SECTOR_Overwrite_Record(uint32_t SectorAddr, uint16_t RecordIndex, uint8_t* buffWrite)
{
	SPIFLASH_SECTOR_HeadInfoDef HeadInfo;
	uint32_t ReadAddr,WriteAddr;
	uint16_t OverWriteWord,i,LengthNow,crcVal,Cnt;
	
	if(SPIFLASH_OK == SPIFLASH_SECTOR_Get_HeadInfo(SectorAddr,&HeadInfo))
	{
		uint8_t	*buffTemp1 = SPIFLASH_SECTOR_Malloc(HeadInfo.CommonInfo.Item.RecordLength_Total);
        if(buffTemp1 == NULL)
        {
            dbg_printf(DBG_DRV_SPI_FLASH,"Overwrite_Record SPIFLASH_SECTOR_Malloc err");
            return SPIFLASH_ERR;
        }
        
		ReadAddr = SectorAddr+SECTOR_HEADSIZE+HeadInfo.CommonInfo.Item.RecordLength_Total*RecordIndex;
		WriteAddr = ReadAddr;
		BSP_SPIFLASH_BufferRead(buffTemp1,ReadAddr,HeadInfo.CommonInfo.Item.RecordLength_Total);
		OverWriteWord = ((uint16_t)buffTemp1[0])+(((uint16_t)buffTemp1[1])<<8);
		Cnt = 0;
		for(i=0;i<16;i++)
		{
			Cnt++;
			if(((OverWriteWord>>i)&0x0001)==1)
			{
				OverWriteWord &= ~(0x0001<<i);
				break;
			}
		}
		if(Cnt<=HeadInfo.CommonInfo.Item.OverWriteCnt)
		{
			for(i=0;i<HeadInfo.CommonInfo.Item.RecordLength_User;i++)
			{
				if((buffTemp1[2+i]&buffWrite[i])!=buffWrite[i])
				{
                    SPIFLASH_SECTOR_Free(buffTemp1);
					return SPIFLASH_ERR;
				}
			}
			LengthNow = 2+HeadInfo.CommonInfo.Item.RecordLength_User+2*Cnt;
			buffTemp1[0] = OverWriteWord;
			buffTemp1[1] = (OverWriteWord>>8);
			for(i=0;i<HeadInfo.CommonInfo.Item.RecordLength_User;i++)
			{
				buffTemp1[2+i] = buffWrite[i];
			}
			crcVal = CRC16LUT(buffTemp1,LengthNow-2);
			buffTemp1[LengthNow-2] = crcVal;
			buffTemp1[LengthNow-1] = (crcVal>>8);
			SPIFLASH_SECTOR_BufferWrite(buffTemp1,WriteAddr,HeadInfo.CommonInfo.Item.RecordLength_Total);
			SPIFLASH_SECTOR_Free(buffTemp1);
			return SPIFLASH_OK;
		}
		else
		{
			return SPIFLASH_ERR;
		}
	}
	else
	{
		return SPIFLASH_ERR;
	}
}
/******************************************************************************
* Function		: 	SPIFLASH_OperateState SPIFLASH_SECTOR_Write_Record
*
* Parameter		:	SectorAddr	- 主扇区首地址
*					RecordIndex	- 记录索引
*					buffWrite	- 待写记录数组指针
*
* Return		:	存储结果
*
* Description	:	将记录写入相应地址，跨越区间写0，后续地址清空。适用于不可重写的记录类型。
*
******************************************************************************/
SPIFLASH_OperateState SPIFLASH_SECTOR_Write_Record(uint32_t SectorAddr, uint16_t RecordIndex, uint8_t* buffWrite)
{
	SPIFLASH_SECTOR_HeadInfoDef HeadInfo;
	uint16_t i,Cnt,CopySize,LastSize,CopyTime;	
//	uint8_t buffTemp[128];
	uint8_t* buffTemp;
	uint32_t BufferLength;
	
	if(SPIFLASH_OK == SPIFLASH_SECTOR_Get_HeadInfo(SectorAddr,&HeadInfo))
	{
//		uint8_t buffTemp[HeadInfo.CommonInfo.Item.RecordLength_User];
		if(128 < HeadInfo.CommonInfo.Item.RecordLength_User)
		{
			buffTemp = SPIFLASH_SECTOR_Malloc(HeadInfo.CommonInfo.Item.RecordLength_User);
		}
		else
		{
			buffTemp = SPIFLASH_SECTOR_Malloc(128);
		}
        //参数检测
        if(buffTemp == NULL)
        {
            dbg_printf(DBG_DRV_SPI_FLASH,"write_Record SPIFLASH_SECTOR_Malloc err");
            return SPIFLASH_ERR;
        }
		memset(buffTemp,0x00,HeadInfo.CommonInfo.Item.RecordLength_User);
		Cnt = SPIFLASH_SECTOR_Get_RecordCnt(SectorAddr);
		if(Cnt<RecordIndex)
		{
			for(i=Cnt;i<RecordIndex;i++)
			{
				if(SPIFLASH_ERR == SPIFLASH_SECTOR_Overwrite_Record(SectorAddr,i,buffTemp))
				{
				    SPIFLASH_SECTOR_Free(buffTemp);
					return SPIFLASH_ERR;
				}
			}
		}
		else if(Cnt>RecordIndex)
		{
			CopySize = SECTOR_HEADSIZE+RecordIndex*HeadInfo.CommonInfo.Item.RecordLength_Total;		
			LastSize = CopySize%128;
			if(LastSize == 0)
			{
				LastSize = 128;
				CopyTime = CopySize/128; 
			}
			else
			{
				CopyTime = CopySize/128+1; 
			}
			/*从备份区拷出前RecordIndex条记录，写入主区*/
			BSP_SPIFLASH_SectorErase(SectorAddr);
			for(i=0;i<CopyTime;i++)
			{
				if(i==(CopyTime-1))
				{
					BufferLength = LastSize;
				}
				else
				{
					BufferLength = 128;
				}
				BSP_SPIFLASH_BufferRead(buffTemp,SectorAddr+SPI_FLASH_SectorSize+128*i,BufferLength);
				BSP_SPIFLASH_BufferWrite(buffTemp,SectorAddr+128*i,BufferLength);
			}
			/*从主区拷出前RecordIndex条记录，写入备份区*/
			BSP_SPIFLASH_SectorErase(SectorAddr+SPI_FLASH_SectorSize);
			for(i=0;i<CopyTime;i++)
			{
				if(i==(CopyTime-1))
				{
					BufferLength = LastSize;
				}
				else
				{
					BufferLength = 128;
				}
				BSP_SPIFLASH_BufferRead(buffTemp,SectorAddr+128*i,BufferLength);
				BSP_SPIFLASH_BufferWrite(buffTemp,SectorAddr+SPI_FLASH_SectorSize+128*i,BufferLength);
			}

		}
		SPIFLASH_SECTOR_Free(buffTemp);
		//SPIFLASH_SECTOR_Free(HeadInfo);
		if(SPIFLASH_OK == SPIFLASH_SECTOR_Overwrite_Record(SectorAddr,RecordIndex,buffWrite))
		{
			return SPIFLASH_OK;
		}
		else
		{
			return SPIFLASH_ERR;
		}
	}
	else
	{
		return SPIFLASH_ERR;
	}
}
/******************************************************************************
* Function		: 	void SPIFLASH_SECTOR_Create
*
* Parameter		:	SectorAddr	- 主扇区首地址
*					HeadInfo	- 扇区头部信息指针
* Return		:	
* Description	:	擦除主扇区及对应的备份区，写入头部信息
*
******************************************************************************/
void SPIFLASH_SECTOR_Create(uint32_t SectorAddr, SPIFLASH_SECTOR_HeadInfoDef* HeadInfo)
{
	BSP_SPIFLASH_SectorErase(SectorAddr);
	BSP_SPIFLASH_SectorErase(SectorAddr+SPI_FLASH_SectorSize);
	SPIFLASH_SECTOR_BufferWrite(HeadInfo->CommonInfo.Para,SectorAddr,SECTOR_HEADSIZE_COMMON);
	SPIFLASH_SECTOR_BufferWrite(HeadInfo->UserInfo.Parameter,SectorAddr+SECTOR_HEADSIZE_COMMON,SECTOR_HEADSIZE_USER);
}
#endif /* __DRIVER_SPIFLASH_C */
/******************************************************************************
*                            end of file
******************************************************************************/
