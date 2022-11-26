#ifndef _BSP_SPI_FLASH
#define _BSP_SPI_FLASH
#include "main.h"


#ifdef __cplusplus
 extern "C" {
#endif
	 
#ifndef __BSP_SPIFLASH_C
#define BSP_SPIFLASH_EXTERN  extern
#else
#define BSP_SPIFLASH_EXTERN
#endif

#define countof(a)                	(sizeof(a) / sizeof(*(a)))
#define SPI_FLASH_PageSize        	256
#define SPI_FLASH_SectorSize		0x1000
#define Dummy_Byte                	0xA5

#define WriteEnable               0x06       //写使能，设置状态寄存器
#define WriteDisable              0x04       //写禁止
#define ReadStatusRegister        0x05       //读状态寄存器
#define WriteStatusRegister       0x01       //写状态寄存器
#define Read_Data                 0x03       //读取存储器数据
#define FastReadData              0x0B       //快速读取存储器数据
#define FastReadDualOutput        0x3B       //快速双端口输出方式读取存储器数据
#define Page_Program              0x02       //页面编程--写数据

#define BlockErace                0xD8       //块擦除
#define SectorErace               0x20       //扇区擦除
#define ChipErace                 0xC7       //片擦除
#define Power_Down                0xB9       //掉电模式

#define ReleacePowerDown          0xAB       //退出掉电模式
#define ReadDeviceID              0xAB       //获取设备ID信息

#define ReadDeviceID              0xAB       //退出掉电模式、设备ID信息
#define ReadManuIDDeviceID        0x90       //读取制造厂商ID信息和设备ID信息
#define ReadJedec_ID              0x9F       //JEDEC的ID信息

#define OP_NORP	0
#define GET_ID	1
#define WRITE_DATA	2
#define READ_DATA	3
#define SECTOR_ERASE	4
#define BULK_ERASE	5
#define CHIP_ERASE	6
#define ReStatusRegister	7
#define WrStatusRegister	8

/*******************************************************************************/
BSP_SPIFLASH_EXTERN void BSP_SPIFLASH_Init(void);
BSP_SPIFLASH_EXTERN uint8_t BSP_SPIFLASH_ByteRead(uint32_t ReadAddr);
BSP_SPIFLASH_EXTERN uint8_t BSP_SPIFLASH_FastRead(uint32_t ReadAddr);
BSP_SPIFLASH_EXTERN void BSP_SPIFLASH_BufferRead(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);
BSP_SPIFLASH_EXTERN void BSP_SPIFLASH_BufferWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
BSP_SPIFLASH_EXTERN void BSP_SPIFLASH_SectorErase(uint32_t SectorAddr);
BSP_SPIFLASH_EXTERN void BSP_SPIFLASH_BulkErase(uint32_t BlockAddr);
BSP_SPIFLASH_EXTERN void BSP_SPIFLASH_ChipErase(void);

BSP_SPIFLASH_EXTERN uint8_t BSP_SPIFLASH_ReadStatusRegister(void);
BSP_SPIFLASH_EXTERN void BSP_SPIFLASH_WriteStatusRegister(uint8_t Byte);
BSP_SPIFLASH_EXTERN void BSP_SPIFLASH_PowerDown(void);
BSP_SPIFLASH_EXTERN void BSP_SPIFLASH_ReleasePowerDown(void);
BSP_SPIFLASH_EXTERN uint8_t BSP_SPIFLASH_ReadDeviceID(void);
BSP_SPIFLASH_EXTERN uint16_t BSP_SPIFLASH_ReadManuID_DeviceID(uint32_t ReadManu_DeviceID_Addr);
BSP_SPIFLASH_EXTERN uint32_t BSP_SPIFLASH_ReadJedecID(void);
#endif