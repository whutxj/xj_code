#include "main.h"
#include "bsp_spiflash.h"
void BSP_SPIFLASH_Init(void)
{
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
	return ;
}
void BSP_SPIFLASH_BufferWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
	return ;
}
void BSP_SPIFLASH_SectorErase(uint32_t SectorAddr)
{
	return;
}
void BSP_SPIFLASH_BulkErase(uint32_t BlockAddr)
{
	return ;
}
void BSP_SPIFLASH_ChipErase(void)
{
	return;
}

uint8_t BSP_SPIFLASH_ReadStatusRegister(void)
{
	return 1;
}
void BSP_SPIFLASH_WriteStatusRegister(uint8_t Byte)
{
	return;
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
	return 1;
}
uint16_t BSP_SPIFLASH_ReadManuID_DeviceID(uint32_t ReadManu_DeviceID_Addr)
{
	return 1;
}
uint32_t BSP_SPIFLASH_ReadJedecID(void)
{
	return 1;
}