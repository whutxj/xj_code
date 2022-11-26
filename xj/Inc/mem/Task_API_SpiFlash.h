
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
#define SPIFLASH_SECTOR_AMOUNT		(256 - 50)	    /* 可用扇区总数 */
#define SPIFLASH_SECTOR_START		(0x00032000)	/* 起始扇区 */


#define SPIFLASH_ADDR_ALARMLOG_RECORD_START  0x7b4000   //60 sectors
#define SPIFLASH_ADDR_ALARMLOG_RECORD_END    0x7effff
#define SPIFLASH_ADDR_ALARMLOG_SNAP_START    0x7aa000   //10 sectors
#define SPIFLASH_ADDR_ALARMLOG_SNAP_END      0x7b3fff
#define SPIFLASH_ADDR_CHARGELOG_START        0x7a0000   //10 sectors
#define SPIFLASH_ADDR_CHARGELOG_END          0x7a9fff
#define SPIFLASH_ADDR_UNPAY_START            0x79e000   //2 sectors
#define SPIFLASH_ADDR_UNPAY_END              0x79ffff
#define SPIFLASH_ADDR_USERLOG_START          0x79c000   //2 sectors
#define SPIFLASH_ADDR_USERLOG_END            0x79dfff

#define SPIFLASH_ADDR_REALDATA_START          0x79b000   //1 sectors
#define SPIFLASH_ADDR_REALDATA_END            0x79bfff
#define SPIFLASH_ADDR_METERPARA_START         0x79a000   //1 sectors
#define SPIFLASH_ADDR_METERPARA_END           0x79afff
#define SPIFLASH_ADDR_SYSPARA_START           0x799000   //1 sectors
#define SPIFLASH_ADDR_SYSPARA_END             0x799fff
#define SPIFLASH_ADDR_FACTORY_PARAM_START     0x798000   //1 sectors
#define SPIFLASH_ADDR_FACTORY_PARAM_END       0x798fff




/******************************************************************************
* Data Type Definitions
******************************************************************************/
/** 
  * @brief   
  */
typedef enum
{
	SPIFLASH_METHOD_SINGLERECORD 			= (uint8_t)0,		/* 方法1 单条记录有效 */
	SPIFLASH_METHOD_MULTIRECORD 			= (uint8_t)1,		/* 方法2 多条记录有效 */
	SPIFLASH_METHOD_MULTIRECORD_WITHPARA1	= (uint8_t)2,		/* 方法3 多条记录有效，带一个用户自定义参数 */
	SPIFLASH_METHOD_MULTIRECORD_OVERWRITE	= (uint8_t)3,		/* 方法4 多条记录有效，记录可重写 */
	
}SPIFLASH_METHOD_ENUM;


/** 
  * @brief   SpiFlash Sector HeadInfo structure definition  
  */ 
typedef struct
{
	union{
		uint8_t Para[6];
		struct {
			uint8_t Method;       			/* i存储方法 */

			uint8_t Type;      				/* i存储类别 */

			uint8_t SectorTotal;    		/* i该类别分配的扇区总数 */
						
			uint8_t OverWriteCnt;			/* i可写次数 */

			uint16_t RecordLength_User;  	/* i记录用户数据长度 */
									
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
