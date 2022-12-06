#include "task_api_spiflash.h"
/******************************************************************************
* Function Prototype Definitions
******************************************************************************/
 void SPIFLASH_General_Init(void)
 {
	 ;
 }
 SPIFLASH_OperateState SPIFLASH_MEM_Init(SPIFLASH_UserConfigDef* ConfigInfo)
 {
	 SPIFLASH_OperateState temp;
	 return temp;
 }
 void SPIFLASH_Data_Erase(uint8_t Method, uint8_t Type)
 {
	 ;
 }

 SPIFLASH_OperateState SPIFLASH_Method1_Get_Record(uint8_t RecordType, uint8_t* buffRead)
 {
	 SPIFLASH_OperateState temp;
	 return temp;
 }
 
 SPIFLASH_OperateState SPIFLASH_Method1_Save_Record(uint8_t RecordType, uint8_t* buffWrite)
 {
	 SPIFLASH_OperateState  temp;
	 return temp;
 }
 
 uint16_t SPIFLASH_Method2_Get_RecordCnt(uint8_t RecordType)
 {
	 return 1;
 }
 
 SPIFLASH_OperateState SPIFLASH_Method2_Get_Record(uint8_t RecordType, uint16_t RecordIndex, uint8_t* buffRead)
 {
	 SPIFLASH_OperateState temp;
	 return temp;
 }
 
 SPIFLASH_OperateState SPIFLASH_Method2_Save_Record(uint8_t RecordType, uint8_t* buffWrite)
 {
	 SPIFLASH_OperateState temp;
	 return temp;
 }
 
 SPIFLASH_OperateState SPIFLASH_Method3_Get_Record(uint8_t RecordType, uint16_t SearchPara, uint16_t RecordIndex, uint8_t* buffRead)
 {
	 SPIFLASH_OperateState temp;
	 return temp;
 }
 
 SPIFLASH_OperateState SPIFLASH_Method3_Save_Record(uint8_t RecordType, uint16_t SearchPara, uint16_t RecordIndex, uint8_t* buffWrite)
 {
	 SPIFLASH_OperateState temp;
	 return temp;
 }
 
 SPIFLASH_OperateState SPIFLASH_Method3_Search_Sector(uint8_t Method, uint8_t RecordType, uint16_t Parameter1)
 {
	 SPIFLASH_OperateState temp;
	 return temp;
 }

 uint16_t SPIFLASH_Method4_Get_RecordCnt(uint8_t RecordType)
 {
	 return 1;
 }
 SPIFLASH_OperateState SPIFLASH_Method4_Get_Record(uint8_t RecordType, uint16_t RecordIndex, uint8_t* buffRead)
 {
	 SPIFLASH_OperateState  temp;
	 return temp;
 }
 SPIFLASH_OperateState SPIFLASH_Method4_Save_Record(uint8_t RecordType, uint16_t RecordIndex, uint8_t* buffWrite)
  {
	 SPIFLASH_OperateState  temp;
	 return temp;
 }
 uint16_t SPIFLASH_Method4_Search_Record(uint8_t RecordType,uint8_t* Mask, uint8_t*SearchValue, uint16_t DataLength)
  {
	 return 1;
 }

