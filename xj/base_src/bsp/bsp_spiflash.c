#include "main.h"
#include "bsp_spiflash.h"
#include "string.h"
#include "stdlib.h"

int g_Flash_op=0;

//FLASH芯片 SPI2的处理  SPI2  W25Q64JVSSIQ
uint8_t FLASH_RxData[200];//FLASH 接收数据的buf
uint8_t FLASH_TxData[200];//FLASH发送数据的buf
//int FLASH_Recv_Len;

//void FLASH_SendData(uint8_t *pTxData, uint16_t Size)
//{
//	HAL_SPI_Transmit_DMA(&hspi2,pTxData,Size);
//}

//void FLASH_RecvData( uint16_t Size)
//{
//	HAL_SPI_Receive_IT(&hspi2,FLASH_RxData,Size);
//}


/*
对接收到的FLASH芯片的数据进行解析，判断当前的状态和值
这里就是读取到的数据
*/
//void FLASH_DataUpdate(uint8_t *pRxData,int CmdType,int Size)
//{
//	uint16_t device_id = 0;
//	//有可能是返回的指令，也可能是FLASH的数据
//	switch(CmdType)
//	{
//		case GET_ID:
//			device_id = (pRxData[0]<<8) | pRxData[1];
//			break;
//		case WRITE_DATA:
//			break;
//		case READ_DATA:
//			//pRxData  
//			break;
//		case SECTOR_ERASE:
//			break;
//		case BULK_ERASE:
//			break;
//		case CHIP_ERASE:
//			break;
//		case ReStatusRegister:
//			break;
//		case WrStatusRegister:
//			break;
//	}
//}

void BSP_SPIFLASH_Init(void)
{
	uint8_t ldata[20];
	memset(ldata,0,20);
	ldata[0] = 0x66;   //Enable Reset
	ldata[1] = 0x99;   //Reset device
	//W25Q64_CS_0;
	g_Flash_op = OP_NORP;
	FLASH_SendData((uint8_t *)ldata,2);
	
	HAL_Delay(1);//1ms
	//W25Q64_CS_1;
}


uint8_t BSP_SPIFLASH_ByteRead(uint32_t ReadAddr)
{
	return 1;
}
uint8_t BSP_SPIFLASH_FastRead(uint32_t ReadAddr)
{
	return 1;
}
void BSP_SPIFLASH_BufferRead(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
	uint8_t pReadCmd[10];
	
	//W25Q64_CS_0;
	pReadCmd[0] = Read_Data;
	FLASH_SendData((uint8_t *)pReadCmd,1);
	
	pReadCmd[0] = (uint8_t)(ReadAddr & 0x000000ff);
	pReadCmd[1] = (uint8_t)((ReadAddr & 0x0000ff00)>>8);
	pReadCmd[2] = (uint8_t)((ReadAddr & 0x00ff0000)>>16);
	//pWriteCmd[3] = (uint8_t)((pAddr[0] && 0xff000000)>>24);
	FLASH_SendData((uint8_t *)pReadCmd,3);
	
	pReadCmd[0] = (uint8_t)(NumByteToRead & 0x000000ff);
	pReadCmd[1] = (uint8_t)((NumByteToRead & 0x0000ff00)>>8);
	FLASH_SendData(pReadCmd,2);
	
	g_Flash_op = READ_DATA;
	pBuffer = FLASH_RecvData(NumByteToRead);
	HAL_Delay(1);//1ms
}
void BSP_SPIFLASH_BufferWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
	uint8_t pWriteCmd[10];
	
	//W25Q64_CS_0;
	pWriteCmd[0] = WriteEnable;
	pWriteCmd[1] = Page_Program;
	FLASH_SendData((uint8_t *)pWriteCmd,2);
	
	pWriteCmd[0] = (uint8_t)(WriteAddr & 0x000000ff);
	pWriteCmd[1] = (uint8_t)((WriteAddr & 0x0000ff00)>>8);
	pWriteCmd[2] = (uint8_t)((WriteAddr & 0x00ff0000)>>16);
	//pWriteCmd[3] = (uint8_t)((pAddr[0] && 0xff000000)>>24);
	FLASH_SendData((uint8_t *)pWriteCmd,3);
	
	FLASH_SendData((uint8_t *)pBuffer,NumByteToWrite);
	
	g_Flash_op = WRITE_DATA;
	HAL_Delay(1);//1ms
}
void BSP_SPIFLASH_SectorErase(uint32_t SectorAddr)
{
	uint8_t SeCmd[10];
  FLASH_SendData((uint8_t *)WriteEnable,1);
	FLASH_SendData((uint8_t *)SectorErace,1);
	SeCmd[0] = (uint8_t)(SectorAddr & 0x000000ff);
	SeCmd[1] = (uint8_t)((SectorAddr & 0x0000ff00)>>8);
	SeCmd[2] = (uint8_t)((SectorAddr & 0x00ff0000)>>16);
	FLASH_SendData((uint8_t *)SeCmd,3);
	g_Flash_op = SECTOR_ERASE;
	HAL_Delay(1);//1ms
}
void BSP_SPIFLASH_BulkErase(uint32_t BlockAddr)
{
	uint8_t BeCmd[10];
  FLASH_SendData((uint8_t *)WriteEnable,1);
	FLASH_SendData((uint8_t *)BlockErace,1);
	BeCmd[0] = (uint8_t)(BlockAddr & 0x000000ff);
	BeCmd[1] = (uint8_t)((BlockAddr & 0x0000ff00)>>8);
	BeCmd[2] = (uint8_t)((BlockAddr & 0x00ff0000)>>16);
	FLASH_SendData((uint8_t *)BeCmd,3);
	g_Flash_op = BULK_ERASE;
	HAL_Delay(1);//1ms
}
void BSP_SPIFLASH_ChipErase(void)
{
	//uint8_t CeCmd[10];
  FLASH_SendData((uint8_t *)WriteEnable,1);
	FLASH_SendData((uint8_t *)ChipErace,1);
	g_Flash_op = CHIP_ERASE;
	HAL_Delay(1);//1ms
}

uint8_t BSP_SPIFLASH_ReadStatusRegister(void)
{
	uint8_t pReadRegisterCmd[10];
	
	//W25Q64_CS_0;
	pReadRegisterCmd[0] = ReadStatusRegister;
	FLASH_SendData((uint8_t *)pReadRegisterCmd,1);
	
	g_Flash_op = ReStatusRegister;
	FLASH_RecvData(1);
	HAL_Delay(1);//1ms
}
void BSP_SPIFLASH_WriteStatusRegister(uint8_t Byte)
{
		uint8_t pWriteRegisterCmd[10];
	
	//W25Q64_CS_0;
	pWriteRegisterCmd[0] = WriteEnable;
	pWriteRegisterCmd[1] = WriteStatusRegister;
	FLASH_SendData((uint8_t *)pWriteRegisterCmd,2);
	
	//FLASH_SendData((uint8_t *)Byte,1);
	
	g_Flash_op = WrStatusRegister;
	HAL_Delay(1);//1ms
}
void BSP_SPIFLASH_PowerDown(void)
{
	return ;
}
void BSP_SPIFLASH_ReleasePowerDown(void)
{
	return ;
}
uint8_t BSP_SPIFLASH_ReadDeviceID(void)
{
	uint8_t ID[4];
	//W25Q64_CS_0;
	ID[0] = ReadManuIDDeviceID;
	ID[1] = 0x00;
	ID[2] = 0x00;
	ID[3] = 0x00;
	FLASH_SendData((uint8_t *)ID,4);
	g_Flash_op = GET_ID;
	FLASH_RecvData(2);
	HAL_Delay(1);//1ms
	//W25Q64_CS_1;   //reset cs
}
uint16_t BSP_SPIFLASH_ReadManuID_DeviceID(uint32_t ReadManu_DeviceID_Addr)
{
	return 1;
}
uint32_t BSP_SPIFLASH_ReadJedecID(void)
{
	return 1;
}

//void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
//{
//	uint8_t *RN8209D_RXData;
//	int RN8209D_Recv_Len;
//	int Recv_Len;
//	if(hspi == &hspi3)//RN8209D
//	{
//		RN8209D_DataUpdate(RN8209D_RXData,RN8209D_Recv_Len);
//	}
//	if(hspi == &hspi2)//FLASH
//	{
//		Recv_Len = hspi->RxXferCount;
//		FLASH_DataUpdate(FLASH_RxData,g_Flash_op,Recv_Len);
//		g_Flash_op = OP_NORP;
//	}	
//}
