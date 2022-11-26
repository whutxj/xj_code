

/******************************************************************************
* Includes
******************************************************************************/
#include "Drv_Usart.h"
#include "LibQueue.h"
#include "bsp_Driver_Uart.h"
#include <string.h>

/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/

/******************************************************************************
* Data Type Definitions
******************************************************************************/


/******************************************************************************
* Global Variable Definitions
******************************************************************************/

int Drv_Uart_Write(uint8_t Port,const unsigned char *pBuf, uint16_t usDataLen,int timeout)
{
    return Usart_Write(Port,(uint8 *)pBuf,usDataLen,timeout); 
}

int Drv_Uart_Read(uint8_t Port,unsigned char *pBuf, uint16_t uLen,int timeout)
{   
    return Usart_Read(Port,pBuf,uLen,timeout);
}




