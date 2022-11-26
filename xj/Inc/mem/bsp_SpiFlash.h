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

#define WriteEnable               0x06       //дʹ�ܣ�����״̬�Ĵ���
#define WriteDisable              0x04       //д��ֹ
#define ReadStatusRegister        0x05       //��״̬�Ĵ���
#define WriteStatusRegister       0x01       //д״̬�Ĵ���
#define Read_Data                 0x03       //��ȡ�洢������
#define FastReadData              0x0B       //���ٶ�ȡ�洢������
#define FastReadDualOutput        0x3B       //����˫�˿������ʽ��ȡ�洢������
#define Page_Program              0x02       //ҳ����--д����

#define BlockErace                0xD8       //�����
#define SectorErace               0x20       //��������
#define ChipErace                 0xC7       //Ƭ����
#define Power_Down                0xB9       //����ģʽ

#define ReleacePowerDown          0xAB       //�˳�����ģʽ
#define ReadDeviceID              0xAB       //��ȡ�豸ID��Ϣ

#define ReadDeviceID              0xAB       //�˳�����ģʽ���豸ID��Ϣ
#define ReadManuIDDeviceID        0x90       //��ȡ���쳧��ID��Ϣ���豸ID��Ϣ
#define ReadJedec_ID              0x9F       //JEDEC��ID��Ϣ

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