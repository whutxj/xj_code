
#ifndef __BSP_I2C_C
#define __BSP_I2C_C
/******************************************************************************
* Include Files
******************************************************************************/
#include "BSP_I2C.h"

/******************************************************************************
* Static Variable Definitions
******************************************************************************/
static void Error_Handler(void);
static uint16_t Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength);
/* Buffer used for transmission */
//static uint8_t aTxBuffer[] = " ****I2C_TwoBoards communication based on Polling****  ****I2C_TwoBoards communication based on Polling****  ****I2C_TwoBoards communication based on Polling**** ";

/* Buffer used for reception */
//static uint8_t aRxBuffer[RXBUFFERSIZE];
/* I2C handler declaration */
static I2C_HandleTypeDef I2cHandle;
/******************************************************************************
* Static Function Prototype Definitions
******************************************************************************/
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
  GPIO_InitTypeDef  GPIO_InitStruct;
  
  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO TX/RX clock */
  I2Cx_SCL_GPIO_CLK_ENABLE();
  I2Cx_SDA_GPIO_CLK_ENABLE();
  /* Enable I2Cx clock */
  I2Cx_CLK_ENABLE(); 

  /*##-2- Configure peripheral GPIO ##########################################*/  
  /* I2C TX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = I2Cx_SCL_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(I2Cx_SCL_GPIO_PORT, &GPIO_InitStruct);
    
  /* I2C RX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = I2Cx_SDA_PIN;
  HAL_GPIO_Init(I2Cx_SDA_GPIO_PORT, &GPIO_InitStruct);
}

/**
  * @brief I2C MSP De-Initialization 
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO, DMA and NVIC configuration to their default state
  * @param hi2c: I2C handle pointer
  * @retval None
  */
void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{
  
  /*##-1- Reset peripherals ##################################################*/
  I2Cx_FORCE_RESET();
  I2Cx_RELEASE_RESET();

  /*##-2- Disable peripherals and GPIO Clocks #################################*/
  /* Configure I2C Tx as alternate function  */
  HAL_GPIO_DeInit(I2Cx_SCL_GPIO_PORT, I2Cx_SCL_PIN);
  /* Configure I2C Rx as alternate function  */
  HAL_GPIO_DeInit(I2Cx_SDA_GPIO_PORT, I2Cx_SDA_PIN);
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *I2cHandle)
{
  Error_Handler();
}

static void Error_Handler(void)
{
  /* Error if LED2 is slowly blinking (1 sec. period) */
//  while(1)
//  {    
////    HAL_Delay(100);
//  } 
}

#if 0
static void Config_I2C(I2C_HandleTypeDef I2cHandle,uint32_t I2C_ADDRESS)
{
    I2cHandle.Instance             = I2Cx;
    I2cHandle.Init.ClockSpeed      = I2C_SPEEDCLOCK / 10;
    I2cHandle.Init.DutyCycle       = I2C_DUTYCYCLE;
    I2cHandle.Init.OwnAddress1     = I2C_ADDRESS;
    I2cHandle.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    I2cHandle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    I2cHandle.Init.OwnAddress2     = 0xFF;
    I2cHandle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    I2cHandle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;  
}
#endif

void I2C_Init(uint32_t I2C_ADDRESS)
{
//    Config_I2C(I2cHandle,I2C_ADDRESS);
    I2cHandle.Instance             = I2Cx;
    I2cHandle.Init.ClockSpeed      = I2C_SPEEDCLOCK ;
    I2cHandle.Init.DutyCycle       = I2C_DUTYCYCLE/ 100;
    I2cHandle.Init.OwnAddress1     = I2C_ADDRESS;
    I2cHandle.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    I2cHandle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    I2cHandle.Init.OwnAddress2     = 0xFF;
    I2cHandle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    I2cHandle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE; 
    if(HAL_I2C_Init(&I2cHandle) != HAL_OK)
    {
        /* Initialization Error */
        Error_Handler();
    }
}

/**
  * @brief  Compares two buffers.
  * @param  pBuffer1, pBuffer2: buffers to be compared.
  * @param  BufferLength: buffer's length
  * @retval 0  : pBuffer1 identical to pBuffer2
  *         >0 : pBuffer1 differs from pBuffer2
  */
static uint16_t Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength)
{
  while (BufferLength--)
  {
    if ((*pBuffer1) != *pBuffer2)
    {
      return BufferLength;
    }
    pBuffer1++;
    pBuffer2++;
  }

  return 0;
}

uint8_t MMA_Write_I2C(uint32_t I2C_ADDRESS,uint8_t *aTxBuffer,uint8_t bufLen)
{
//    Config_I2C(I2cHandle,I2C_ADDRESS);
    if(HAL_I2C_Master_Transmit(&I2cHandle, (uint16_t)I2C_ADDRESS,\
                           (uint8_t*)aTxBuffer, bufLen, 100) != HAL_OK)
    {
        if (HAL_I2C_GetError(&I2cHandle) != HAL_I2C_ERROR_AF)
        {
          Error_Handler();
        }
        return HAL_ERROR;
    }
    return HAL_OK;
}

uint8_t MMA_I2C_WriteByte(uint32_t I2C_ADDRESS,uint8_t regAddr,uint8_t val)
{
    uint8_t buf[2] = {0};
    buf[0] = regAddr;
    buf[1] = val;
    if(MMA_Write_I2C(I2C_ADDRESS,buf,2) != HAL_OK)
    {
        if (HAL_I2C_GetError(&I2cHandle) != HAL_I2C_ERROR_AF)
        {
            Error_Handler();
        }
        return HAL_ERROR;
    }
    return HAL_OK;
}


uint8_t MMA_Read_I2C(uint32_t I2C_ADDRESS,uint8_t regAddr,uint8_t *aRxBuffer,uint8_t bufLen)
{

    while(HAL_I2C_Mem_Read(&I2cHandle, (uint16_t)I2C_ADDRESS, 
                  regAddr,1 , aRxBuffer, bufLen, 100) != HAL_OK)
    {
        if (HAL_I2C_GetError(&I2cHandle) != HAL_I2C_ERROR_AF)
        {
          Error_Handler();
          return HAL_ERROR;
        }
    }
       return HAL_OK;
}

#endif
