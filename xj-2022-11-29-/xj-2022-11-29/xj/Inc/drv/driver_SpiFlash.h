/******************************************************************************
*  
*  File name:     driver_SpiFlash.h
*  
*  Version:       V1.00
*  Created on:    2016-12-28
*  Description:   
*  History:
*      Date        Author        Modification
*    1.2014-12-15  
*    2. ...
******************************************************************************/
#ifndef __DRIVER_SPIFLASH_H
#define __DRIVER_SPIFLASH_H

/*****************************************************************************/
#ifdef __cplusplus
 extern "C" {
#endif
	 
#ifndef __DRIVER_SPIFLASH_C
#define DRIVER_SPIFLASH_EXTERN  extern
#else
#define DRIVER_SPIFLASH_EXTERN
#endif

/******************************************************************************
* Includes
******************************************************************************/
#include "main.h"
#include "bsp_SpiFlash.h"

/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/
#define SECTOR_HEADSIZE			(128)
#define SECTOR_HEADSIZE_COMMON	(64)
#define SECTOR_HEADSIZE_USER	(64)

#define MaxCntOfRecord(Length)	((SPI_FLASH_SectorSize-SECTOR_HEADSIZE)/Length)
/******************************************************************************
* Data Type Definitions
******************************************************************************/
/** 
  * @brief   
  */
typedef enum
{
  SPIFLASH_OK 	= (uint8_t)0,
  SPIFLASH_ERR 	= (uint8_t)1,
}SPIFLASH_OperateState;

/** 
  * @brief   
  */
typedef enum
{
  DATA_VALID 	= (uint8_t)0,
  DATA_INVALID 	= (uint8_t)1,
}SPIFLASH_RecordState;

/** 
  * @brief   SpiFlash Sector HeadInfo structure definition  
  */ 
typedef struct
{
	union{
		uint8_t Para[SECTOR_HEADSIZE_COMMON];
		struct {
			uint8_t Method;       			/* i�洢���� */

			uint8_t Type;      				/* i�洢��� */

			uint8_t SectorTotal;    		/* i����������������� */

			uint8_t SectorCurrent;			/* �������ڸ������������������ţ���0��ʼ�� */
			
			uint8_t	StoreFlag;				/* �洢��ǣ����ڼ����������� */
			
			uint8_t OverWriteCnt;			/* i��д���� */

			uint16_t RecordLength_User;  	/* i��¼�û����ݳ��� */
			
			uint16_t RecordLength_Total;	/* ��¼ʵ�ʳ��� */
			
			uint8_t Reserve[52];			/* Ԥ�� */
			
			uint16_t CrcVal;				/* CRCУ���� */
			
		}Item;
			
	}CommonInfo;
	
	struct {
		uint8_t Parameter[64];
		
	}UserInfo;
	
}SPIFLASH_SECTOR_HeadInfoDef;

/******************************************************************************
* Global Variable Definitions
******************************************************************************/


/******************************************************************************
* Function Prototype Definitions
******************************************************************************/
DRIVER_SPIFLASH_EXTERN SPIFLASH_OperateState SPIFLASH_SECTOR_Get_HeadInfo(uint32_t SectorAddr, SPIFLASH_SECTOR_HeadInfoDef* HeadInfo);
DRIVER_SPIFLASH_EXTERN uint16_t SPIFLASH_SECTOR_Get_RecordCnt(uint32_t SectorAddr);
DRIVER_SPIFLASH_EXTERN SPIFLASH_OperateState SPIFLASH_SECTOR_Read_Record(uint32_t SectorAddr, uint16_t RecordIndex, uint8_t* buffRead);
DRIVER_SPIFLASH_EXTERN uint16_t SPIFLASH_SECTOR_Search_Record(uint32_t SectorAddr, uint8_t* Mask, uint8_t* MaskBuff);
DRIVER_SPIFLASH_EXTERN void SPIFLASH_SECTOR_CheckResume(uint32_t SectorAddr);
DRIVER_SPIFLASH_EXTERN SPIFLASH_OperateState SPIFLASH_SECTOR_Overwrite_Record(uint32_t SectorAddr, uint16_t RecordIndex, uint8_t* buffWrite);
DRIVER_SPIFLASH_EXTERN SPIFLASH_OperateState SPIFLASH_SECTOR_Write_Record(uint32_t SectorAddr, uint16_t RecordIndex, uint8_t* buffWrite);
DRIVER_SPIFLASH_EXTERN void SPIFLASH_SECTOR_Create(uint32_t SectorAddr, SPIFLASH_SECTOR_HeadInfoDef* HeadInfo);

#ifdef __cplusplus
}
#endif

#endif /* __DRIVER_SPIFLASH_H */

/******************************************************************************
*                            end of file
******************************************************************************/
