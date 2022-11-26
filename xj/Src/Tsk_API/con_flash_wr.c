#include "main.h"
#include "con_flash_wr.h"
#include "string.h"
#include "stdlib.h"

uint32_t SPIFLASH_ContinWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
	uint32_t sec_addr;
	uint16_t sec_shift;
	uint16_t sec_remain;
	uint32_t final_addr;
	
	sec_addr = WriteAddr/4096;   # sector addr
	sec_shift = WriteAddr%4096;
	sec_remain = 4096-sec_shift;
	if (NumByteToWrite<=sec_remain){
		sec_remain = NumByteToWrite;
	}
	while(1)
	{
		BSP_SPIFLASH_BufferWrite(pBuffer,WriteAddr,sec_remain);
		if(NumByteToWrite == sec_remain){
			final_addr = sec_addr*4096+sec_shift+sec_remain-1;
			break;
		}
		else {
			sec_addr += 1;
			sec_shift = 0;
			pBuffer += sec_remain;
			WriteAddr += sec_remain;
			NumByteToWrite -= sec_remain;
			if (NumByteToWrite>4096){
				sec_remain = 4096;
			}
		  else {
				sec_remain = NumByteToWrite;
			}
		}	
	}
	return final_addr;
}

void SPIFLASH_Erase(uint32_t EraseAddr){
	uint32_t sec_end_addr;
	uint32_t addr_erase;

	sec_end_addr = SPIFLASH_ADDR_END;
	for (addr_erase=EraseAddr;addr_erase<=SPIFLASH_ADDR_END;addr_erase+=4096){
		BSP_SPIFLASH_SectorErase(addr_erase);
	}
}
