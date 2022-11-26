/*
 * CRC16.h
 *
 *  Created on:
 *
 */

#ifndef CRC16_H_
#define CRC16_H_

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __CRC16_C__
#define CRC16_EXTERN  extern
#else
#define CRC16_EXTERN
#endif
CRC16_EXTERN uint16_t CRC16LUT(const unsigned char *puchMsg, uint16_t usDataLen);
CRC16_EXTERN uint16_t CRC16(uint16_t *pBuf, uint16_t uLen);
CRC16_EXTERN uint16_t CRC16ShortToChar(uint16_t *pBuf, uint16_t uLen);
CRC16_EXTERN uint16_t crc16_xmodem(uint8_t *from, uint8_t *to);

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif /* CRC16_H_ */
