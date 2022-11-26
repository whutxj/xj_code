#ifndef __BSP_I2C_H
#define __BSP_I2C_H

/******************************************************************************
* Include Files
******************************************************************************/
#include "main.h"
//#include "stm32f1xx_nucleo.h"

/* Definition for I2Cx clock resources */
#define I2Cx                            I2C1
#define I2Cx_CLK_ENABLE()               __HAL_RCC_I2C1_CLK_ENABLE()
#define I2Cx_SDA_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
#define I2Cx_SCL_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE() 

#define I2Cx_FORCE_RESET()              __HAL_RCC_I2C1_FORCE_RESET()
#define I2Cx_RELEASE_RESET()            __HAL_RCC_I2C1_RELEASE_RESET()

/* Definition for I2Cx Pins */
#define I2Cx_SCL_PIN                    GPIO_PIN_6
#define I2Cx_SCL_GPIO_PORT              GPIOB
#define I2Cx_SDA_PIN                    GPIO_PIN_7 
#define I2Cx_SDA_GPIO_PORT              GPIOB


/* I2C SPEEDCLOCK define to max value: 400 KHz on STM32F1xx*/
#define I2C_SPEEDCLOCK   400000
#define I2C_DUTYCYCLE    I2C_DUTYCYCLE_2

#define I2C_OK 1
#define I2C_ERROR 0

/* Size of Transmission buffer */
#define TXBUFFERSIZE                      (COUNTOF(aTxBuffer) - 1)
/* Size of Reception buffer */
#define RXBUFFERSIZE                      TXBUFFERSIZE

/* Exported macro ------------------------------------------------------------*/
#define COUNTOF(__BUFFER__)   (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))





extern void I2C_Init(uint32_t I2C_ADDRESS);
extern uint8_t MMA_Write_I2C(uint32_t I2C_ADDRESS,uint8_t *aTxBuffer,uint8_t bufLen);
extern uint8_t MMA_Read_I2C(uint32_t I2C_ADDRESS,uint8_t regAddr,uint8_t *aTxBuffer,uint8_t bufLen);
extern uint8_t MMA_I2C_WriteByte(uint32_t I2C_ADDRESS,uint8_t regAddr,uint8_t val);




#endif
