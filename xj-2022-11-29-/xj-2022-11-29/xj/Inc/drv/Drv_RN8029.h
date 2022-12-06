#ifndef _DRV_RN8290_H
#define _DRV_RN8290_H

#ifdef __cplusplus
 extern "C" {
#endif

/******************************************************************************
* Includes
******************************************************************************/
  #include "main.h"

/*****************************************************************************
*结构体声明     
*****************************************************************************/ 

#define			ADSYSCON        0x00 
#define        	ADEMUCON        0x01
#define        	ADHFConst     	0x02 
#define        	ADPStart      	0x03 
#define        	ADDStart      	0x04 
#define			ADGPQA        	0x05 
#define        	ADGPQB        	0x06 
#define        	ADPhsA        	0x07 
#define        	ADPhsB        	0x08
#define			ADQPHSCAL				0x09    
#define			ADAPOSA 				0x0a
#define        	ADAPOSB 				0x0b
#define        	ADRPOSA 				0x0c
#define        	ADRPOSB 				0x0d
#define        	ADIARMSOS     	        0x0e
#define        	ADIBRMSOS     	        0x0f
#define        	ADIBGain      	        0x10
#define			ADD2FPL       	        0x11
#define        	ADD2FPH       	        0x12
#define        	ADDCIAH       	        0x13
#define        	ADDCIBH       	        0x14
#define         ADDCUH		            0x15   
#define         ADDCL   	            0x16 
#define         ADEMUCON2	            0x17
#define			ADPFCnt    	            0x20
#define        	ADDFcnt    	            0x21
#define        	ADIARMS       	0x22
#define        	ADIBRMS       	0x23
#define        	ADURMS        	0x24
#define			ADUFreq       	0x25
#define        	ADPowerPA     	0x26
#define        	ADPowerPB     	0x27
#define         ADEnergyP  	    0x29
#define         ADEnergyP2 	    0x2a
#define         ADEnergyD  	    0x2b
#define         ADEnergyD2    	0x2c
#define         ADEMUStatus   	0x2d
#define         ADSPL_IA      	0x30
#define         ADSPL_IB      	0x31
#define         ADSPL_U       	0x32
#define         ADIE  		    0x40
#define         ADIF  		    0x41
#define         ADRIF    	    0x42
#define         ADSysStatus  	0x43
#define         ADRData      	0x44
#define         ADWData      	0x45
#define         ADDeviceID   	0x7f
#define         WriteEn   	    0xea
     

typedef struct
{
    double    K_I1;
    double    K_I2;
    double      K_U;
    double    K_P2;
    uint32_t    Effective_I1;
    uint32_t    Effective_I2;
    uint32_t    Effective_U;
    uint32_t    Effective_P1;
    uint32_t    Effective_P2;
    uint32_t    PowerS;
    uint32_t    Power_Factor;  //功率因数
}EMU_DATA;

typedef struct
{
    double saveK_I2;
    double saveK_U;
    double saveK_P2;
    uint32_t saveGP;
    uint32_t savePhase;
    uint32_t saveOffset;
    uint32_t rev;
}SAVEPARA;


typedef struct
{
    uint32_t U;
    uint32_t I;
    uint32_t P;
}UIPPARA;
#define   A7053_OK         0x00
#define   A7053_ERROR      0x01


#define A7053_SPI_CS_PIN                           GPIO_PIN_15        /* Pa.15*/
#define A7053_SPI_CS_GPIO_PORT                     GPIOA
#define A7053_SPI_CS_GPIO_CLK_ENABLE()             __HAL_RCC_GPIOA_CLK_ENABLE()
#define A7053_SPI_CS_GPIO_CLK_DISABLE()            __HAL_RCC_GPIOA_CLK_DISABLE()

#define A7053_SPI_CS_LOW()       HAL_GPIO_WritePin(A7053_SPI_CS_GPIO_PORT, A7053_SPI_CS_PIN, GPIO_PIN_RESET)
#define A7053_SPI_CS_HIGH()      HAL_GPIO_WritePin(A7053_SPI_CS_GPIO_PORT, A7053_SPI_CS_PIN, GPIO_PIN_SET)

// SPI 计量芯片 的spi口配置

#define SPI_A7053_PORT  SPI_PORT3  //用的是SPI3

#define  SPI2_Read()   BSP_SPIx_Read(SPI_A7053_PORT) 
#define  SPI2_Write(a) BSP_SPIx_Write(SPI_A7053_PORT,a)


uint8_t Drv_RN8209_SPI_Init(void);
void RN8209_Init_Para(void);

uint8_t RN8209_Cal_UIP_K(uint8_t *buf);
uint8_t RN8209_Measure_Adj(uint8_t *buf);
uint32_t RN8209_Read_ID(void);
float RN8209_Read_Val(uint32_t *buf);

#endif
