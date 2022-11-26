/******************************************************************************
*  
*  File name:     mbFunction.h
*  
*  Version:       V1.00
*  Created on:    2015-2-5
*  Description:   
*  History:
*      Date        Author        Modification
*    1.2015-2-5 
*    2. ...
******************************************************************************/
#ifndef __MBFUNCTION_H
#define __MBFUNCTION_H

#ifdef __cplusplus
 extern "C" {
#endif

#ifndef __MBFUNCTION_C
#define MBFUNCTION_EXTERN  extern
#else
#define MBFUNCTION_EXTERN
#endif

/******************************************************************************
* Include Files
******************************************************************************/
#include "mb.h"
#include "mbdatatype.h"

/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/
#define MB_POLL_CYCLETIME       100  

#define REG_INPUT_START         	5000
#define REG_INPUT_NREGS         	400
#define REG_HOLDING_START       	5400
#define REG_HOLDING_NREGS       	108
#define REG_COIL_START          	5900
#define REG_COIL_NREGS          	16
#define REG_DISC_START_STANDARD     5000
#define REG_DISC_NREGS_STANDARD     48
#define REG_DISC_START_EXPAND     	5100
#define REG_DISC_NREGS_EXPAND     	96
#define REG_DISC_START_INNER     	5400
#define REG_DISC_NREGS_INNER     	32

/******************************************************************************
* Global Variable Definitions
******************************************************************************/
MBFUNCTION_EXTERN USHORT usRegInputStart;//= REG_INPUT_START;
MBFUNCTION_EXTERN USHORT *usRegInputBuf[REG_INPUT_NREGS];//={7,7,7,7};
MBFUNCTION_EXTERN USHORT usRegHoldingStart;// = REG_HOLDING_START;
MBFUNCTION_EXTERN USHORT *usRegHoldingBuf[REG_HOLDING_NREGS];//={5,5,5,5,5};
MBFUNCTION_EXTERN UCHAR *usRegCoilBuf[REG_COIL_NREGS/8];//=0x6600;       //0b1010101000000000
MBFUNCTION_EXTERN UCHAR *usRegDiscBufStandard[REG_DISC_NREGS_STANDARD/8];//=0x6660;       //0b1010101010100000
MBFUNCTION_EXTERN UCHAR *usRegDiscBufExpand[REG_DISC_NREGS_EXPAND/8];
MBFUNCTION_EXTERN UCHAR *usRegDiscBufInner[REG_DISC_NREGS_INNER/8];
/******************************************************************************
* Function Prototype Definitions
******************************************************************************/

MBFUNCTION_EXTERN eMBErrorCode    eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress,USHORT usNRegs );

MBFUNCTION_EXTERN eMBErrorCode    eMBRegHoldingCB(UCHAR * pucRegBuffer,USHORT usAddress,USHORT usNRegs,eMBRegisterMode eMode);

MBFUNCTION_EXTERN eMBErrorCode    eMBRegCoilsCB(UCHAR * pucRegBuffer,USHORT usAddress,USHORT usNCoils,eMBRegisterMode eMode);

MBFUNCTION_EXTERN eMBErrorCode    eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress,USHORT usNDiscrete );

MBFUNCTION_EXTERN UCHAR eMBGetCoilAddrSector(USHORT usAddress);

#ifdef __cplusplus
}
#endif

#endif /*__MBFUNCTION_H */
/******************************************************************************
*                            end of file
******************************************************************************/
