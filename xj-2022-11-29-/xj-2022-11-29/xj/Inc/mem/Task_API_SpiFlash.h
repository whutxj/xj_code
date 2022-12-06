
#ifndef __TASK_API_SPIFLASH_H
#define __TASK_API_SPIFLASH_H

///*****************************************************************************/
//#ifdef __cplusplus
// extern "C" {
//#endif
	 
//#ifndef __TASK_API_SPIFLASH_C
//#define TASK_API_SPIFLASH_EXTERN  extern
//#else
//#define TASK_API_SPIFLASH_EXTERN
//#endif

/******************************************************************************
* Includes
******************************************************************************/
#include "Tsk_JWT.h"
#include "driver_SpiFlash.h"

/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/
#define SPIFLASH_SECTORSIZE_DOUBLE	0x2000
#define SPIFLASH_SECTOR_AMOUNT		(256 - 50)	    /* ������������ */
#define SPIFLASH_SECTOR_START		(0x00032000)	/* ��ʼ���� */


/******************************************************************************
* Data Type Definitions
******************************************************************************/
/** 
  * @brief   
  */
typedef enum
{
	SPIFLASH_METHOD_SINGLERECORD 			= (uint8_t)0,		/* ����1 ������¼��Ч */
	SPIFLASH_METHOD_MULTIRECORD 			= (uint8_t)1,		/* ����2 ������¼��Ч */
	SPIFLASH_METHOD_MULTIRECORD_WITHPARA1	= (uint8_t)2,		/* ����3 ������¼��Ч����һ���û��Զ������ */
	SPIFLASH_METHOD_MULTIRECORD_OVERWRITE	= (uint8_t)3,		/* ����4 ������¼��Ч����¼����д */
	
}SPIFLASH_METHOD_ENUM;


/** 
  * @brief   SpiFlash Sector HeadInfo structure definition  
  */ 
typedef struct
{
	union{
		uint8_t Para[6];
		struct {
			uint8_t Method;       			/* i�洢���� */

			uint8_t Type;      				/* i�洢��� */

			uint8_t SectorTotal;    		/* i����������������� */
						
			uint8_t OverWriteCnt;			/* i��д���� */

			uint16_t RecordLength_User;  	/* i��¼�û����ݳ��� */
									
		}Item;
			
	}CommonInfo;
	
	struct {
		uint8_t Parameter[64];
		
	}UserInfo;
	
}SPIFLASH_UserConfigDef;

/******************************************************************************
* Global Variable Definitions
******************************************************************************/


/******************************************************************************
* Function Prototype Definitions
******************************************************************************/
 void SPIFLASH_General_Init(void);
 SPIFLASH_OperateState SPIFLASH_MEM_Init(SPIFLASH_UserConfigDef* ConfigInfo);
 void SPIFLASH_Data_Erase(uint8_t Method, uint8_t Type);

 SPIFLASH_OperateState SPIFLASH_Method1_Get_Record(uint8_t RecordType, uint8_t* buffRead);
 SPIFLASH_OperateState SPIFLASH_Method1_Save_Record(uint8_t RecordType, uint8_t* buffWrite);

 uint16_t SPIFLASH_Method2_Get_RecordCnt(uint8_t RecordType);
 SPIFLASH_OperateState SPIFLASH_Method2_Get_Record(uint8_t RecordType, uint16_t RecordIndex, uint8_t* buffRead);
 SPIFLASH_OperateState SPIFLASH_Method2_Save_Record(uint8_t RecordType, uint8_t* buffWrite);

 SPIFLASH_OperateState SPIFLASH_Method3_Get_Record(uint8_t RecordType, uint16_t SearchPara, uint16_t RecordIndex, uint8_t* buffRead);
 SPIFLASH_OperateState SPIFLASH_Method3_Save_Record(uint8_t RecordType, uint16_t SearchPara, uint16_t RecordIndex, uint8_t* buffWrite);
 SPIFLASH_OperateState SPIFLASH_Method3_Search_Sector(uint8_t Method, uint8_t RecordType, uint16_t Parameter1);

 uint16_t SPIFLASH_Method4_Get_RecordCnt(uint8_t RecordType);
 SPIFLASH_OperateState SPIFLASH_Method4_Get_Record(uint8_t RecordType, uint16_t RecordIndex, uint8_t* buffRead);
 SPIFLASH_OperateState SPIFLASH_Method4_Save_Record(uint8_t RecordType, uint16_t RecordIndex, uint8_t* buffWrite);
 uint16_t SPIFLASH_Method4_Search_Record(uint8_t RecordType,uint8_t* Mask, uint8_t*SearchValue, uint16_t DataLength);


//#ifdef __cplusplus
//}
//#endif

#endif /* __TASK_API_SPIFLASH_H */

/******************************************************************************
*                            end of file
******************************************************************************/
