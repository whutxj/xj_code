/******************************************************************************
*  
*  File name:     main.h
*  Author:        
*  Version:       V1.00
*  Created on:    2016-12-15
*  Description:   
*  History:
*      Date        Author        Modification
*    1.2016-12-15  
*    2. ...
******************************************************************************/


#ifndef __BSP_SPI_H
#define __BSP_SPI_H

#ifdef __cplusplus
 extern "C" {
#endif

#ifndef __MBSP_SPI_C
#define SPI_EXTERN  extern
#else
#define SPI_EXTERN
#endif

/******************************************************************************
* Includes
******************************************************************************/
#include "main.h"

/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/
/*##################### SPI1 ###################################*/
#define EVAL_SPI1                               SPI1
#define EVAL_SPI1_CLK_ENABLE()                  __HAL_RCC_SPI1_CLK_ENABLE()

#define EVAL_SPI1_SCK_GPIO_PORT                 GPIOA            
#define EVAL_SPI1_SCK_PIN                       GPIO_PIN_5
#define EVAL_SPI1_SCK_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOA_CLK_ENABLE()
#define EVAL_SPI1_SCK_GPIO_CLK_DISABLE()        __HAL_RCC_GPIOA_CLK_DISABLE()

#define EVAL_SPI1_MISO_MOSI_GPIO_PORT           GPIOA
#define EVAL_SPI1_MISO_MOSI_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOA_CLK_ENABLE()
#define EVAL_SPI1_MISO_MOSI_GPIO_CLK_DISABLE()  __HAL_RCC_GPIOA_CLK_DISABLE()
#define EVAL_SPI1_MISO_PIN                      GPIO_PIN_6       
#define EVAL_SPI1_MOSI_PIN                      GPIO_PIN_7      
/* Maximum Timeout values for flags waiting loops. These timeouts are not based
   on accurate values, they just guarantee that the application will not remain
   stuck if the SPI communication is corrupted.
   You may modify these timeout values depending on CPU frequency and application
   conditions (interrupts routines ...). */   
#define EVAL_SPI1_TIMEOUT_MAX                   1000


/*##################### SPI2 ###################################*/
#define EVAL_SPI2                               SPI2
#define EVAL_SPI2_CLK_ENABLE()                  __HAL_RCC_SPI2_CLK_ENABLE()

#define EVAL_SPI2_SCK_GPIO_PORT                 GPIOB/* PB.13*/
#define EVAL_SPI2_SCK_PIN                       GPIO_PIN_13
#define EVAL_SPI2_SCK_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOB_CLK_ENABLE()
#define EVAL_SPI2_SCK_GPIO_CLK_DISABLE()        __HAL_RCC_GPIOB_CLK_DISABLE()

#define EVAL_SPI2_MISO_MOSI_GPIO_PORT           GPIOB
#define EVAL_SPI2_MISO_MOSI_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOB_CLK_ENABLE()
#define EVAL_SPI2_MISO_MOSI_GPIO_CLK_DISABLE()  __HAL_RCC_GPIOB_CLK_DISABLE()
#define EVAL_SPI2_MISO_PIN                      GPIO_PIN_14      /* PA.06*/
#define EVAL_SPI2_MOSI_PIN                      GPIO_PIN_15       /* PA.07*/
/* Maximum Timeout values for flags waiting loops. These timeouts are not based
   on accurate values, they just guarantee that the application will not remain
   stuck if the SPI communication is corrupted.
   You may modify these timeout values depending on CPU frequency and application
   conditions (interrupts routines ...). */   
#define EVAL_SPI2_TIMEOUT_MAX                   1000


/*##################### SPI3 ###################################*/
#define EVAL_SPI3                               hspi3
#define EVAL_SPI3_CLK_ENABLE()                  __HAL_RCC_SPI3_CLK_ENABLE()

#define EVAL_SPI3_SCK_GPIO_PORT                 GPIOB             /* PA.05*/
#define EVAL_SPI3_SCK_PIN                       GPIO_PIN_3
#define EVAL_SPI3_SCK_GPIO_CLK_ENABLE()         __HAL_RCC_GPIOB_CLK_ENABLE()
#define EVAL_SPI3_SCK_GPIO_CLK_DISABLE()        __HAL_RCC_GPIOB_CLK_DISABLE()

#define EVAL_SPI3_MISO_MOSI_GPIO_PORT           GPIOB
#define EVAL_SPI3_MISO_MOSI_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOB_CLK_ENABLE()
#define EVAL_SPI3_MISO_MOSI_GPIO_CLK_DISABLE()  __HAL_RCC_GPIOB_CLK_DISABLE()
#define EVAL_SPI3_MISO_PIN                      GPIO_PIN_4       /* PA.06*/
#define EVAL_SPI3_MOSI_PIN                      GPIO_PIN_5       /* PA.07*/
/* Maximum Timeout values for flags waiting loops. These timeouts are not based
   on accurate values, they just guarantee that the application will not remain
   stuck if the SPI communication is corrupted.
   You may modify these timeout values depending on CPU frequency and application
   conditions (interrupts routines ...). */   
#define EVAL_SPI3_TIMEOUT_MAX                   1000

#define SPI_DUMMY_BYTE  0xa5

/******************************************************************************
* Data Type Definitions
******************************************************************************/
enum
{
    SPI_PORT1,//spi1
    SPI_PORT2,//spi2
    SPI_PORT3,//spi3
};

/******************************************************************************
* Global Variable Definitions
******************************************************************************/


/******************************************************************************
* Function Prototype Definitions
******************************************************************************/
HAL_StatusTypeDef BSP_SPIx_Init(uint8_t Port);
/**
  * @brief SPI error treatment function
  * @retval None
  */
void BSP_SPIx_Error (uint8_t Port);
/**
  * @brief  SPI Write a byte to device
  * @param  WriteValue to be written
  * @retval The value of the received byte.
  */
uint8_t BSP_SPIx_Write(uint8_t Port,uint8_t WriteValue);
uint8_t BSP_SPIx_Read(uint8_t Port);

#ifdef __cplusplus
}
#endif

#endif 

/************************  *****END OF FILE****/
