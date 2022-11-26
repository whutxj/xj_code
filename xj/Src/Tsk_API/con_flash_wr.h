#ifndef _CON_FLASH_WR
#define _CON_FLASH_WR
#include "main.h"



#define SPIFLASH_ADDR_INFO_START  0x000000   //1 sectors updata_info
#define SPIFLASH_ADDR_INFO_END    0x000fff
#define SPIFLASH_ADDR_START    0x001000   //continue write
#define SPIFLASH_ADDR_END      0x3fffff   //4M

/*******************************************************************************/
void SPIFLASH_Erase(uint32_t EraseAddr);
uint32_t SPIFLASH_ContinWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
#endif