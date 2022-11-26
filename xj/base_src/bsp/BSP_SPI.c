
#ifndef __BSP_SPI_C
#define __BSP_SPI_C

/******************************************************************************
* Include Files
******************************************************************************/
#include "main.h"
#include "BSP_SPI.h"



/******************************************************************************
* Static Variable Definitions
******************************************************************************/
uint32_t SpixTimeout = EVAL_SPI1_TIMEOUT_MAX;        /*<! Value of Timeout when SPI communication fails */
static SPI_HandleTypeDef heval_Spi1;
static SPI_HandleTypeDef heval_Spi2;
//static SPI_HandleTypeDef hspi3;//hspi3

/******************************************************************************
* Static Function Prototype Definitions
******************************************************************************/
//使用默认初始化
static void SPIx_MspInit(SPI_HandleTypeDef *hspi)
{
//  GPIO_InitTypeDef  gpioinitstruct = {0};
//  
//  /*** Configure the GPIOs ***/  
//  /* Enable GPIO clock */
//  
//  if(hspi == &heval_Spi1)
//  {
//      EVAL_SPI1_SCK_GPIO_CLK_ENABLE();
//      EVAL_SPI1_MISO_MOSI_GPIO_CLK_ENABLE();
//      
//      /* configure SPI SCK */
//      gpioinitstruct.Pin        = EVAL_SPI1_SCK_PIN;
//      gpioinitstruct.Mode       = GPIO_MODE_AF_PP;
//      gpioinitstruct.Pull       = GPIO_NOPULL;
//      gpioinitstruct.Speed      = GPIO_SPEED_FREQ_HIGH;
//      HAL_GPIO_Init(EVAL_SPI1_SCK_GPIO_PORT, &gpioinitstruct);

//      /* configure SPI MISO and MOSI */
//      gpioinitstruct.Pin        = (EVAL_SPI1_MISO_PIN | EVAL_SPI1_MOSI_PIN);
//      gpioinitstruct.Mode       = GPIO_MODE_AF_PP;
//      gpioinitstruct.Pull       = GPIO_NOPULL;
//      gpioinitstruct.Speed      = GPIO_SPEED_FREQ_HIGH;
//      HAL_GPIO_Init(EVAL_SPI1_MISO_MOSI_GPIO_PORT, &gpioinitstruct); 

//      /*** Configure the SPI peripheral ***/ 
//      /* Enable SPI clock */
//      EVAL_SPI1_CLK_ENABLE();      
//  }
//  else if(hspi == &heval_Spi2)
//  {
//      EVAL_SPI2_SCK_GPIO_CLK_ENABLE();
//      EVAL_SPI2_MISO_MOSI_GPIO_CLK_ENABLE();
//      
//      /* configure SPI SCK */
//      gpioinitstruct.Pin        = EVAL_SPI2_SCK_PIN;
//      gpioinitstruct.Mode       = GPIO_MODE_AF_PP;
//      gpioinitstruct.Pull       = GPIO_NOPULL;
//      gpioinitstruct.Speed      = GPIO_SPEED_FREQ_HIGH;
//      HAL_GPIO_Init(EVAL_SPI2_SCK_GPIO_PORT, &gpioinitstruct);

//      /* configure SPI MISO and MOSI */
//      gpioinitstruct.Pin        = (EVAL_SPI2_MISO_PIN | EVAL_SPI2_MOSI_PIN);
//      gpioinitstruct.Mode       = GPIO_MODE_AF_PP;
//      gpioinitstruct.Pull       = GPIO_NOPULL;
//      gpioinitstruct.Speed      = GPIO_SPEED_FREQ_HIGH;
//      HAL_GPIO_Init(EVAL_SPI2_MISO_MOSI_GPIO_PORT, &gpioinitstruct); 

//      /*** Configure the SPI peripheral ***/ 
//      /* Enable SPI clock */
//      EVAL_SPI2_CLK_ENABLE();        
//  }
//  else if(hspi == &hspi3)
//  {
//      EVAL_SPI3_SCK_GPIO_CLK_ENABLE();
//      EVAL_SPI3_MISO_MOSI_GPIO_CLK_ENABLE();
//      
//      /* configure SPI SCK */
//      gpioinitstruct.Pin        = EVAL_SPI3_SCK_PIN;
//      gpioinitstruct.Mode       = GPIO_MODE_AF_PP;
//      gpioinitstruct.Pull       = GPIO_NOPULL;
//      gpioinitstruct.Speed      = GPIO_SPEED_FREQ_HIGH;
//      HAL_GPIO_Init(EVAL_SPI3_SCK_GPIO_PORT, &gpioinitstruct);

//      /* configure SPI MISO and MOSI */
//      gpioinitstruct.Pin        = (EVAL_SPI3_MISO_PIN | EVAL_SPI3_MOSI_PIN);
//      gpioinitstruct.Mode       = GPIO_MODE_AF_PP;
//      gpioinitstruct.Pull       = GPIO_NOPULL;
//      gpioinitstruct.Speed      = GPIO_SPEED_FREQ_HIGH;
//      HAL_GPIO_Init(EVAL_SPI3_MISO_MOSI_GPIO_PORT, &gpioinitstruct); 

//      /*** Configure the SPI peripheral ***/ 
//      /* Enable SPI clock */
//      EVAL_SPI3_CLK_ENABLE();        
//  }

}

/**
  * @brief  Initializes SPI HAL.
  * @retval None
  */
//这里屏蔽掉，采用HAL中的默认初始化
HAL_StatusTypeDef BSP_SPIx_Init(uint8_t Port)
{
//    SPI_HandleTypeDef heval_Spi = {0};
//    
//    if(Port == SPI_PORT1)
//    {
//        /* DeInitializes the SPI peripheral */
//        heval_Spi1.Instance = EVAL_SPI1;
//        HAL_SPI_DeInit(&heval_Spi1);

//        /* SPI Config */
//        /* SPI baudrate is set to 36 MHz (PCLK2/SPI_BaudRatePrescaler = 72/2 = 36 MHz) */
//        heval_Spi1.Init.BaudRatePrescaler  = SPI_BAUDRATEPRESCALER_2;
//        heval_Spi1.Init.Direction          = SPI_DIRECTION_2LINES;
//        heval_Spi1.Init.CLKPhase           = SPI_PHASE_1EDGE;
//        heval_Spi1.Init.CLKPolarity        = SPI_POLARITY_LOW;
//        heval_Spi1.Init.CRCCalculation     = SPI_CRCCALCULATION_DISABLE;
//        heval_Spi1.Init.CRCPolynomial      = 7;
//        heval_Spi1.Init.DataSize           = SPI_DATASIZE_8BIT;
//        heval_Spi1.Init.FirstBit           = SPI_FIRSTBIT_MSB;
//        heval_Spi1.Init.NSS                = SPI_NSS_SOFT;
//        heval_Spi1.Init.TIMode             = SPI_TIMODE_DISABLE;
//        heval_Spi1.Init.Mode               = SPI_MODE_MASTER;

//        SPIx_MspInit(&heval_Spi1);

//        return (HAL_SPI_Init(&heval_Spi1));        
//    }
//    else if(Port == SPI_PORT2)
//    {
//        heval_Spi2.Instance = EVAL_SPI2;
//        HAL_SPI_DeInit(&heval_Spi2);

//        /* SPI Config */
//        /* SPI baudrate is set to 36 MHz (PCLK2/SPI_BaudRatePrescaler = 72/2 = 36 MHz) */
//        heval_Spi2.Init.BaudRatePrescaler  = SPI_BAUDRATEPRESCALER_64;
//        heval_Spi2.Init.Direction          = SPI_DIRECTION_2LINES;
//        heval_Spi2.Init.CLKPhase           = SPI_PHASE_2EDGE;
//        heval_Spi2.Init.CLKPolarity        = SPI_POLARITY_LOW;
//        heval_Spi2.Init.CRCCalculation     = SPI_CRCCALCULATION_DISABLE;
//        heval_Spi2.Init.CRCPolynomial      = 7;
//        heval_Spi2.Init.DataSize           = SPI_DATASIZE_8BIT;
//        heval_Spi2.Init.FirstBit           = SPI_FIRSTBIT_MSB;
//        heval_Spi2.Init.NSS                = SPI_NSS_SOFT;
//        heval_Spi2.Init.TIMode             = SPI_TIMODE_DISABLE;
//        heval_Spi2.Init.Mode               = SPI_MODE_MASTER;

//        SPIx_MspInit(&heval_Spi2);

//        return (HAL_SPI_Init(&heval_Spi2));             
//    }
//    else if(Port == SPI_PORT3)
//    {
//        hspi3.Instance = EVAL_SPI3;
//        HAL_SPI_DeInit(&hspi3);

//        /* SPI Config */
//        /* SPI baudrate is set to 36 MHz (PCLK2/SPI_BaudRatePrescaler = 72/2 = 36 MHz) */
//        hspi3.Init.BaudRatePrescaler  = SPI_BAUDRATEPRESCALER_8;
//        hspi3.Init.Direction          = SPI_DIRECTION_2LINES;
//        hspi3.Init.CLKPhase           = SPI_PHASE_2EDGE;
//        hspi3.Init.CLKPolarity        = SPI_POLARITY_LOW;
//        hspi3.Init.CRCCalculation     = SPI_CRCCALCULATION_DISABLE;
//        hspi3.Init.CRCPolynomial      = 7;
//        hspi3.Init.DataSize           = SPI_DATASIZE_8BIT;
//        hspi3.Init.FirstBit           = SPI_FIRSTBIT_MSB;
//        hspi3.Init.NSS                = SPI_NSS_SOFT;
//        hspi3.Init.TIMode             = SPI_TIMODE_DISABLE;
//        hspi3.Init.Mode               = SPI_MODE_MASTER;

//        SPIx_MspInit(&hspi3);

//        return (HAL_SPI_Init(&hspi3));     
//        
//    }
//    
//    return HAL_ERROR;
	return HAL_OK;
}

/**
  * @brief SPI error treatment function
  * @retval None
  */
void BSP_SPIx_Error (uint8_t Port)
{
    /* De-initialize the SPI communication BUS */  
//    if(Port == SPI_PORT1)
//    {
//            
//        HAL_SPI_DeInit(&heval_Spi1);        
//    }
//    else if(Port == SPI_PORT2)
//    {
//        HAL_SPI_DeInit(&heval_Spi2); 
//    }
//    else
//    {
//        HAL_SPI_DeInit(&hspi3);
//    }
    /* Re- Initiaize the SPI communication BUS */
    BSP_SPIx_Init(Port);
}
/**
  * @brief  SPI Write a byte to device
  * @param  WriteValue to be written
  * @retval The value of the received byte.
  */
uint8_t BSP_SPIx_Write(uint8_t Port,uint8_t WriteValue)
{
    HAL_StatusTypeDef status = HAL_OK;
    uint8_t ReadValue = 0;
    SPI_HandleTypeDef heval_Spi = {0};

    if(Port == SPI_PORT1)
    {
        heval_Spi = heval_Spi1;
    }
    else if(Port == SPI_PORT2)
    {
        heval_Spi = heval_Spi2;
    }
    else if(Port == SPI_PORT3)
    {
        heval_Spi = hspi3;
    }
           
    status = HAL_SPI_TransmitReceive(&heval_Spi, (uint8_t*) &WriteValue, (uint8_t*) &ReadValue, 1, SpixTimeout);

    /* Check the communication status */
    if(status != HAL_OK)
    {
    /* Execute user timeout callback */
        BSP_SPIx_Error(Port);
    }

    return ReadValue;
}


/**
  * @brief SPI Read 1 byte from device
  * @retval Read data
*/
uint8_t BSP_SPIx_Read(uint8_t Port)
{
  return (BSP_SPIx_Write(Port,SPI_DUMMY_BYTE));
}


#endif
