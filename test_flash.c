#include "main.h"
#include "bsp_spiflash.h"

#define ADDR_FLASH_SECTORE_1 0x00001000
#define ADDR_FLASH_SECTORE_2 0x00002000

int main(void){
	uint32_t tempData[200];
	uint32_t temp_Data[500];
	uint32_t VerifyData[200];
	uint32_t Verify_Data[500];
	int i;
	int j;

	BSP_SPIFLASH_Init();
	BSP_SPIFLASH_SectorErase(ADDR_FLASH_SECTORE_1);
	BSP_SPIFLASH_SectorErase(ADDR_FLASH_SECTORE_2);
	for(i=0; i<200; i++){
		tempData[i] = i+1;	
	}
	for(j=0; j<500; j++){
		temp_Data[j] = j+1;	
	}
	BSP_SPIFLASH_BufferWrite(tempData,ADDR_FLASH_SECTORE_1,200);
	BSP_SPIFLASH_BufferRead(VerifyData,ADDR_FLASH_SECTORE_1,200);
	BSP_SPIFLASH_BufferWrite(temp_Data,ADDR_FLASH_SECTORE_2,500);
	BSP_SPIFLASH_BufferRead(Verify_Data,ADDR_FLASH_SECTORE_2,500);
	while(1){
	}
}