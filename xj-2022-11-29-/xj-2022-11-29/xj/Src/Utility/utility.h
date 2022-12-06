
/******************************************************************************
*  
*  File name:     Utility.h
*  Author:        
*  Version:       V1.00
*  Created on:    2017-01-03
*  Description:   
*  History:
*      Date        Author        Modification
*    1.2014-12-15  
*    2. ...
******************************************************************************/
#ifndef UTILITY_H
#define UTILITY_H

/******************************************************************************
* Includes
******************************************************************************/
#include "main.h"
#include "cmsis_os.h"
#include "Global.h"
#include "stdio.h"
/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/

/******************************************************************************
* Data Type Definitions
******************************************************************************/
typedef union TDWORD
{
  unsigned long           v;
  long                    l;
  unsigned short int      w[2];
  signed int              s[2];
  unsigned char           b[4];
  char                    c[4];
  float                   f;
}
co_dword;


/******************************************************************************
* Global Variable Definitions
******************************************************************************/


#ifdef __cplusplus
 extern "C" {
#endif
/******************************************************************************
* Function Prototype Definitions
******************************************************************************/
uint8_t Checkout_Sum(uint8_t *pBuf, uint16_t uLen);
int Bcd2int(uint8_t *bcd, uint16_t uLen);
uint8_t Checkout_Sum(uint8_t *pBuf, uint16_t uLen);
uint8_t BcdToHex8(uint8_t bcd);
int16_t Bcd2ToHex16(uint8_t *bcd);
I32 Bcd3ToHex32(uint8_t *bcd);
I32 Bcd4ToHexInt32(uint8_t *bcd);
U32 Bcd4ToHexUint32(uint8_t *bcd);
void HextoBCD(int Dec, uint8_t *Bcd, int length);
uint8_t HexToBcd(uint8_t hex);
uint8_t ByteToBcd(uint8_t Byte);
void dbg_printf(uint8 leve,uint8 *pFormat,...);
void print_hex_data(const uint8 *hex,int len);
void dbg_init(void);
void dbg_printf(uint8 leve,uint8 *pFormat,...);

void shift_hex_data(uint8 *DispBuf,uint8 *hex,int len);
void shift_time_data(uint8 *DispBuf,uint32 time);
uint8_t DataXorCheck(uint8_t *buf,uint8_t len);
uint8 Tick_Out(uint32 *tick,uint32 timeout);
uint8 asc2hex(uint8 * Dst,uint8 * src,int len);
U8 Get_Chip_ID(uint8 *id);
void reversalByte(uint8 *dst,uint8 *src,int len);
int check_date(int year,int mon ,int day);
#ifdef __cplusplus
}
#endif

#endif

