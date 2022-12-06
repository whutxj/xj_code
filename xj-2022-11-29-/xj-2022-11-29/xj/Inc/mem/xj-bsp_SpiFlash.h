/******************************************************************************
*  
*  File name:     bsp_SpiFlash.h
*  
*  Version:       V1.00
*  Created on:    2016-12-28
*  Description:   
*  History:
*      Date        Author        Modification
*    1.2014-12-15  
*    2. ...
******************************************************************************/
#ifndef __BSP_SPIFLASH_H
#define __BSP_SPIFLASH_H

/*****************************************************************************/
#ifdef __cplusplus
 extern "C" {
#endif
	 
#ifndef __BSP_SPIFLASH_C
#define BSP_SPIFLASH_EXTERN  extern
#else
#define BSP_SPIFLASH_EXTERN
#endif

/******************************************************************************
* Includes
******************************************************************************/
#include "stm32f4xx_hal.h"	 

/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/
/*Define----------------------------------------------------------------------*/
#define countof(a)                	(sizeof(a) / sizeof(*(a)))
#define SPI_FLASH_PageSize        	256
#define SPI_FLASH_SectorSize		0x1000
#define Dummy_Byte                	0xA5
/* Definition for SPIx clock resources */
//#define SPIx                             SPI2
//#define SPIx_CLK_ENABLE()                __SPI2_CLK_ENABLE()
//#define DMAx_CLK_ENABLE()                __DMA2_CLK_ENABLE()
//#define SPIx_SCK_GPIO_CLK_ENABLE()       __GPIOB_CLK_ENABLE()
//#define SPIx_MISO_GPIO_CLK_ENABLE()      __GPIOB_CLK_ENABLE() 
//#define SPIx_MOSI_GPIO_CLK_ENABLE()      __GPIOB_CLK_ENABLE() 

//#define SPIx_FORCE_RESET()               __SPI2_FORCE_RESET()
//#define SPIx_RELEASE_RESET()             __SPI2_RELEASE_RESET()

///* Definition for SPIx Pins */
//#define SPIx_SCK_PIN                     GPIO_PIN_13
//#define SPIx_SCK_GPIO_PORT               GPIOB
//#define SPIx_SCK_AF                      GPIO_AF5_SPI2
//#define SPIx_MISO_PIN                    GPIO_PIN_14
//#define SPIx_MISO_GPIO_PORT              GPIOB
//#define SPIx_MISO_AF                     GPIO_AF5_SPI2
//#define SPIx_MOSI_PIN                    GPIO_PIN_15
//#define SPIx_MOSI_GPIO_PORT              GPIOB
//#define SPIx_MOSI_AF                     GPIO_AF5_SPI2
//#define SPIx_NSS_PIN                    GPIO_PIN_12
//#define SPIx_NSS_GPIO_PORT              GPIOB
//#define SPIx_NSS_AF                     GPIO_AF5_SPI2

#define SPIx                             SPI1
#define SPIx_CLK_ENABLE()                __SPI1_CLK_ENABLE()
#define DMAx_CLK_ENABLE()                __DMA2_CLK_ENABLE()
#define SPIx_SCK_GPIO_CLK_ENABLE()       __GPIOA_CLK_ENABLE()
#define SPIx_MISO_GPIO_CLK_ENABLE()      __GPIOA_CLK_ENABLE() 
#define SPIx_MOSI_GPIO_CLK_ENABLE()      __GPIOA_CLK_ENABLE() 

#define SPIx_FORCE_RESET()               __SPI1_FORCE_RESET()
#define SPIx_RELEASE_RESET()             __SPI1_RELEASE_RESET()

/* Definition for SPIx Pins */
#define SPIx_SCK_PIN                        GPIO_PIN_5
#define SPIx_SCK_GPIO_PORT                  GPIOA
#define SPIx_SCK_AF                         GPIO_AF5_SPI1
#define SPIx_MISO_PIN                       GPIO_PIN_6
#define SPIx_MISO_GPIO_PORT                 GPIOA
#define SPIx_MISO_AF                        GPIO_AF5_SPI1
#define SPIx_MOSI_PIN                       GPIO_PIN_7
#define SPIx_MOSI_GPIO_PORT                 GPIOA
#define SPIx_MOSI_AF                        GPIO_AF5_SPI1
#define SPIx_NSS_PIN                        GPIO_PIN_4 
#define SPIx_NSS_GPIO_PORT                  GPIOA
#define SPIx_NSS_AF                         GPIO_AF5_SPI1

#define BSP_SPIFLASH_CS_LOW()       \
                                    HAL_GPIO_WritePin(SPIx_NSS_GPIO_PORT, GPIO_PIN_4, GPIO_PIN_RESET);\

                                    
#define BSP_SPIFLASH_CS_HIGH()       HAL_GPIO_WritePin(SPIx_NSS_GPIO_PORT, GPIO_PIN_4, GPIO_PIN_SET);\


#define WriteEnable               0x06       //дʹ�ܣ�����״̬�Ĵ���
#define WriteDisable              0x04       //д��ֹ
#define ReadStatusRegister        0x05       //��״̬�Ĵ���
#define WriteStatusRegister       0x01       //д״̬�Ĵ���
#define Read_Data                 0x03       //��ȡ�洢������
#define FastReadData              0x0B       //���ٶ�ȡ�洢������
#define FastReadDualOutput        0x3B       //����˫�˿������ʽ��ȡ�洢������
#define Page_Program              0x02       //ҳ����--д����

#define BlockErace                0xD8       //�����
#define SectorErace               0x20       //��������
#define ChipErace                 0xC7       //Ƭ����
#define Power_Down                0xB9       //����ģʽ

#define ReleacePowerDown          0xAB       //�˳�����ģʽ
#define ReadDeviceID              0xAB       //��ȡ�豸ID��Ϣ

#define ReadDeviceID              0xAB       //�˳�����ģʽ���豸ID��Ϣ
#define ReadManuIDDeviceID        0x90       //��ȡ���쳧��ID��Ϣ���豸ID��Ϣ
#define ReadJedec_ID              0x9F       //JEDEC��ID��Ϣ

/******************************************************************************
* Data Type Definitions
******************************************************************************/


/******************************************************************************
* Global Variable Definitions
******************************************************************************/
BSP_SPIFLASH_EXTERN SPI_HandleTypeDef SpiHandle;

/******************************************************************************
* Function Prototype Definitions
******************************************************************************/
BSP_SPIFLASH_EXTERN void BSP_SPIFLASH_Init(void);
BSP_SPIFLASH_EXTERN uint8_t BSP_SPIFLASH_ByteRead(uint32_t ReadAddr);
BSP_SPIFLASH_EXTERN uint8_t BSP_SPIFLASH_FastRead(uint32_t ReadAddr);
BSP_SPIFLASH_EXTERN void BSP_SPIFLASH_BufferRead(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);
BSP_SPIFLASH_EXTERN void BSP_SPIFLASH_BufferWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
BSP_SPIFLASH_EXTERN void BSP_SPIFLASH_SectorErase(uint32_t SectorAddr);
BSP_SPIFLASH_EXTERN void BSP_SPIFLASH_BulkErase(uint32_t BlockAddr);
BSP_SPIFLASH_EXTERN void BSP_SPIFLASH_ChipErase(void);

BSP_SPIFLASH_EXTERN uint8_t BSP_SPIFLASH_ReadStatusRegister(void);
BSP_SPIFLASH_EXTERN void BSP_SPIFLASH_WriteStatusRegister(uint8_t Byte);
BSP_SPIFLASH_EXTERN void BSP_SPIFLASH_PowerDown(void);
BSP_SPIFLASH_EXTERN void BSP_SPIFLASH_ReleasePowerDown(void);
BSP_SPIFLASH_EXTERN uint8_t BSP_SPIFLASH_ReadDeviceID(void);
BSP_SPIFLASH_EXTERN uint16_t BSP_SPIFLASH_ReadManuID_DeviceID(uint32_t ReadManu_DeviceID_Addr);
BSP_SPIFLASH_EXTERN uint32_t BSP_SPIFLASH_ReadJedecID(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_SPIFLASH_H */

/******************************************************************************
*                            end of file
******************************************************************************/
