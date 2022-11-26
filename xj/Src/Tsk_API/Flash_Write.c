#include "main.h"
#include "string.h"
#include "stdlib.h"

void Flash_Write(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)
{
	uint32_t secpos;
	uint16_t secoff;
	uint16_t secremain;
	uint16_t i;
 
	secpos=WriteAddr/4096;	
	secoff=WriteAddr%4096;	
	secremain=4096-secoff;	
 
	if(NumByteToWrite<=secremain)secremain=NumByteToWrite;
	while(1)
	{
		BSP_SPIFLASH_BufferRead(Readbuff,secpos*4096,4096);
		for(i=0;i<secremain;i++)
		{
			if(Readbuff[secoff+i]!=0XFF)break;
		}
		if(i<secremain)
		{
			BSP_SPIFLASH_SectorErase(WriteAddr);	
			for(i=0;i<secremain;i++)		
			{
				Readbuff[i+secoff]=pBuffer[i];
			}
			BSP_SPIFLASH_BufferWrite(Readbuff,secpos*4096,4096);
		}else BSP_SPIFLASH_BufferWrite(pBuffer,WriteAddr,secremain);
		if(NumByteToWrite==secremain)break;
		else
		{
			secpos++;					
			secoff=0;					
 
			pBuffer+=secremain; 		
			WriteAddr+=secremain;		
			NumByteToWrite-=secremain;	
			if(NumByteToWrite>4096)secremain=4096;	
			else secremain=NumByteToWrite;			
		}
	};
}