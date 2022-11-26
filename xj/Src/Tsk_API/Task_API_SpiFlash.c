#include "task_api_spiflash.h"
#include "Task_SpiFlash.h"
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
 { uint32_t addr_alarm;
	 uint32_t addr_charge;
	 uint32_t addr_unpay;
	 uint32_t addr_userlog;
	 switch (Method) {
		 case SPIFLASH_METHOD_MULTIRECORD_OVERWRITE:
			 switch (Type) 
			 {
				 case TYPE_ALARMLOG:
					 for (addr_alarm=SPIFLASH_ADDR_ALARMLOG_RECORD_START;addr_alarm<=SPIFLASH_ADDR_ALARMLOG_RECORD_END;addr_alarm+=4096){
						 BSP_SPIFLASH_SectorErase(addr_alarm);}
					 for (addr_alarm=SPIFLASH_ADDR_ALARMLOG_SNAP_START;addr_alarm<=SPIFLASH_ADDR_ALARMLOG_SNAP_END;addr_alarm+=4096){
						 BSP_SPIFLASH_SectorErase(addr_alarm); }
					 break;
				 case TYPE_CHARGELOG:
					 for (addr_charge=SPIFLASH_ADDR_CHARGELOG_START;addr_charge<=SPIFLASH_ADDR_CHARGELOG_END;addr_charge+=4096){
						 BSP_SPIFLASH_SectorErase(addr_charge);}
					 break;
				 case TYPE_UN_PAY:
					 for (addr_unpay=SPIFLASH_ADDR_UNPAY_START;addr_unpay<=SPIFLASH_ADDR_UNPAY_END;addr_unpay+=4096){
						 BSP_SPIFLASH_SectorErase(addr_unpay);}
					 break;		 
				 }
			 break;
		 case SPIFLASH_METHOD_MULTIRECORD:
			 switch (Type)
			 {
				 case RECORD_USERLOG:
					 for (addr_userlog=SPIFLASH_ADDR_USERLOG_START;addr_userlog<=SPIFLASH_ADDR_USERLOG_END;addr_userlog+=4096){
				     BSP_SPIFLASH_SectorErase(addr_userlog);}
					 break;
			 }
			 break;
		 case SPIFLASH_METHOD_SINGLERECORD:
			 switch (Type)
			 {
				 case TYPE_REALDATA:
					 BSP_SPIFLASH_SectorErase(SPIFLASH_ADDR_REALDATA_START);
				   break;
				 case TYPE_METERPARA:
					 BSP_SPIFLASH_SectorErase(SPIFLASH_ADDR_METERPARA_START);
				   break;
				 case TYPE_SYSPARA:
					 BSP_SPIFLASH_SectorErase(SPIFLASH_ADDR_SYSPARA_START);
				   break;
				 case TYPE_FACTORY_PARAM:
					 BSP_SPIFLASH_SectorErase(SPIFLASH_ADDR_FACTORY_PARAM_START);
				   break;
			 }
			 break; 
    }
 }

 SPIFLASH_OperateState SPIFLASH_Method1_Get_Record(uint8_t RecordType, uint8_t* buffRead)
 {
	 uint32_t addr;
	 switch(RecordType)
	 {
		 case TYPE_REALDATA:
			 addr = SPIFLASH_ADDR_REALDATA_START;
			 break;
		 case TYPE_METERPARA:
			 addr = SPIFLASH_ADDR_METERPARA_START;
			 break;
		 case TYPE_SYSPARA:
			 addr = SPIFLASH_ADDR_SYSPARA_START;
			 break;
		 case TYPE_FACTORY_PARAM:
			 addr = SPIFLASH_ADDR_FACTORY_PARAM_START;
			 break; 		 
	 }
	 BSP_SPIFLASH_BufferRead(buffRead,addr,sizeof(buffRead));
 }
 
 SPIFLASH_OperateState SPIFLASH_Method1_Save_Record(uint8_t RecordType, uint8_t* buffWrite)
 {
//	 SPIFLASH_OperateState  temp;
	 uint32_t addr;
	 switch(RecordType)
	 {
		 case TYPE_REALDATA:
			 addr = SPIFLASH_ADDR_REALDATA_START;
			 break;
		 case TYPE_METERPARA:
			 addr = SPIFLASH_ADDR_METERPARA_START;
			 break;
		 case TYPE_SYSPARA:
			 addr = SPIFLASH_ADDR_SYSPARA_START;
			 break;
		 case TYPE_FACTORY_PARAM:
			 addr = SPIFLASH_ADDR_FACTORY_PARAM_START;
			 break; 		 
	 }
	 BSP_SPIFLASH_SectorErase(addr);
	 BSP_SPIFLASH_BufferWrite(buffWrite,addr,sizeof(buffWrite));
	 
	 return SPIFLASH_OK;
//	 return temp;
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
//	 SPIFLASH_OperateState temp;
//	 return temp;
	 uint32_t addr;
	 if (RecordType == RECORD_USERLOG){
		 addr = SPIFLASH_ADDR_USERLOG_START;
	 }
	 for (;addr<=SPIFLASH_ADDR_USERLOG_END;addr+=4096){
		BSP_SPIFLASH_SectorErase(addr);
		BSP_SPIFLASH_BufferWrite(buffWrite,addr,sizeof(buffWrite));
	}
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
	 	uint32_t addr,addr_start,addr_end;
		uint16_t index=0;
	  uint16_t cnt_index = 0;
		uint8_t* pBuffer;
	  uint16_t i;
	  switch(RecordType)
	 {
		 case TYPE_ALARMLOG:
			 addr_start = SPIFLASH_ADDR_ALARMLOG_RECORD_START;
		   addr_end = SPIFLASH_ADDR_ALARMLOG_RECORD_END;
			 break;
		 case TYPE_CHARGELOG:
			 addr_start = SPIFLASH_ADDR_CHARGELOG_START;
		   addr_end = SPIFLASH_ADDR_CHARGELOG_END;
			 break;
		 case TYPE_UN_PAY:
			 addr_start = SPIFLASH_ADDR_UNPAY_START;
		   addr_end = SPIFLASH_ADDR_UNPAY_END;
			 break;		 
	 }
	 for (addr=addr_start;addr<=addr_end;addr+=4096)
	 {
		 BSP_SPIFLASH_BufferRead(pBuffer,addr_start,4096);
		 for (i=0;i<4096;i++)
		 {
			if(pBuffer[i] != 0xff)
			{
				cnt_index +=1;
				break;
			}
		 }
	 }
	 if (cnt_index == 0)
	 {
		 index = 0;
	 }
	 else
	 {
		 index = cnt_index -1;
	 }
		return index;
}
 SPIFLASH_OperateState SPIFLASH_Method4_Get_Record(uint8_t RecordType, uint16_t RecordIndex, uint8_t* buffRead)
 {
//	 SPIFLASH_OperateState  temp;
//	 return temp;
	 uint32_t addr,addr_index;
	 	 switch(RecordType)
	 {
		 case TYPE_ALARMLOG:
			 addr = SPIFLASH_ADDR_ALARMLOG_RECORD_START;
			 break;
		 case TYPE_CHARGELOG:
			 addr = SPIFLASH_ADDR_CHARGELOG_START;
			 break;
		 case TYPE_UN_PAY:
			 addr = SPIFLASH_ADDR_UNPAY_START;
			 break;		 
	 }
	 addr_index = addr + RecordIndex*4096;
	 BSP_SPIFLASH_BufferRead(buffRead,addr_index,sizeof(buffRead));
 }
 
 
 SPIFLASH_OperateState SPIFLASH_Method4_Save_Record(uint8_t RecordType, uint16_t RecordIndex, uint8_t* buffWrite)
  {
//	 SPIFLASH_OperateState  temp;
//	 return temp;
		uint32_t addr,addr_index;
	 	 switch(RecordType)
	 {
		 case TYPE_ALARMLOG:
			 addr = SPIFLASH_ADDR_ALARMLOG_RECORD_START;
			 break;
		 case TYPE_CHARGELOG:
			 addr = SPIFLASH_ADDR_CHARGELOG_START;
			 break;
		 case TYPE_UN_PAY:
			 addr = SPIFLASH_ADDR_UNPAY_START;
			 break;		 
	 }
	 addr_index = addr + RecordIndex*4096;
	 BSP_SPIFLASH_SectorErase(addr_index);
	 BSP_SPIFLASH_BufferWrite(buffWrite,addr_index,sizeof(buffWrite));
	 
 }
	
 
 uint16_t SPIFLASH_Method4_Search_Record(uint8_t RecordType,uint8_t* Mask, uint8_t*SearchValue, uint16_t DataLength)
  {
		uint32_t addr,addr_start,addr_end;
		uint16_t index;
		uint8_t* pBuffer;
	 	 switch(RecordType)
	 {
		 case TYPE_ALARMLOG:
			 addr_start = SPIFLASH_ADDR_ALARMLOG_RECORD_START;
		   addr_end = SPIFLASH_ADDR_ALARMLOG_RECORD_END;
			 break;
		 case TYPE_CHARGELOG:
			 addr_start = SPIFLASH_ADDR_CHARGELOG_START;
		   addr_end = SPIFLASH_ADDR_CHARGELOG_END;
			 break;
		 case TYPE_UN_PAY:
			 addr_start = SPIFLASH_ADDR_UNPAY_START;
		   addr_end = SPIFLASH_ADDR_UNPAY_END;
			 break;		 
	 }
	 for (addr=addr_start;addr<=addr_end;addr+=4096)
	 {
		 BSP_SPIFLASH_BufferRead(pBuffer,addr_start,DataLength);
		 if(pBuffer == SearchValue)
		 {
			 index = (addr - addr_start)/4096;
			 break;
		 }
		 else
		 {
			 index = 0xffff;
			 break;
		 }
	 }
	 
	 return index;
 }

