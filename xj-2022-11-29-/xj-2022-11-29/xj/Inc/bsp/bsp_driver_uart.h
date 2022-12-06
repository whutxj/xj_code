/******************************************************************************
*
*  File name:      bsp_driver_uart.h

*  Description:   This is file used for Uart commucation.
*  History:
*      Date        Author        Modification
*    1.
*    2. ...
******************************************************************************/
#ifndef _BSP_DRIVER_UART_H
#define _BSP_DRIVER_UART_H

/*****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#ifndef _BSP_DRIVER_UART_C_
#define BSP_DRIVER_UART_EXTERN  extern
#else
#define BSP_DRIVER_UART_EXTERN
#endif

/******************************************************************************
* Includes
******************************************************************************/
#include "main.h"
#include "cmsis_os.h"
//#include "stm32f1xx_hal_tim.h"
#include "LibQueue.h"
#include "Global.h"
/******************************************************************************
* Macros Definitions (constant/Macros)
******************************************************************************/
#define SLAVER

#define UART1_RX_SIZE      32	//屏幕
#define UART1_TX_SIZE      128
#define UART2_RX_SIZE      200	//200//以太网
#define UART2_TX_SIZE      350  //350
#define UART3_RX_SIZE      64	//485	
#define UART3_TX_SIZE      64	
#define UART4_RX_SIZE      512//1024 //(1024+256) 4g
#define UART4_TX_SIZE      512	
#define UART5_RX_SIZE      32	//读卡器
#define UART5_TX_SIZE      32


typedef enum
{
   COM1 = 1,
   COM2,
   COM3,
   COM4,
   COM5,
}COM_NAME;

typedef enum
{
    STOP_1 = 1,
    STOP_2,
}STOP_BIT_NAME;

#define P_NONE 0
#define P_ODD  1
#define P_EVEN 2

/******************USART1 UART******************************************/
#define USART1_CLK_ENABLE()              __HAL_RCC_USART1_CLK_ENABLE();
#define USART1_DMA_CLK_ENABLE()          __HAL_RCC_DMA1_CLK_ENABLE()
#define USART1_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define USART1_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()

#define USART1_FORCE_RESET()             __HAL_RCC_USART1_FORCE_RESET()
#define USART1_RELEASE_RESET()           __HAL_RCC_USART1_RELEASE_RESET()


/* Definition for USARTx Pins */
#define USART1_TX_PIN                    GPIO_PIN_9
#define USART1_TX_GPIO_PORT              GPIOA
#define USART1_RX_PIN                    GPIO_PIN_10
#define USART1_RX_GPIO_PORT              GPIOA

/* Definition for USART1's DMA */
#define USART1_TX_DMA_CHANNEL             DMA1_Channel4


/* Definition for USART1's NVIC */
#define USART1_DMA_TX_IRQn                DMA1_Channel4_IRQn

#define USART1_DMA_TX_IRQHandler          DMA1_Channel4_IRQHandler

/* Definition for USART1's NVIC */
#define USART1_IRQn                      USART1_IRQn
#define USART1_IRQHandler                USART1_IRQHandler

/* Definition for USART2 Pins */
#define USART2_CLK_ENABLE()              __HAL_RCC_USART2_CLK_ENABLE();
#define USART2_DMA_CLK_ENABLE()          __HAL_RCC_DMA1_CLK_ENABLE()
#define USART2_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOD_CLK_ENABLE()
#define USART2_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOD_CLK_ENABLE()

#define USART2_FORCE_RESET()             __HAL_RCC_USART2_FORCE_RESET()
#define USART2_RELEASE_RESET()           __HAL_RCC_USART2_RELEASE_RESET()


/* Definition for USARTx Pins */
#define USART2_TX_PIN                    GPIO_PIN_5
#define USART2_TX_GPIO_PORT              GPIOD
#define USART2_RX_PIN                    GPIO_PIN_6
#define USART2_RX_GPIO_PORT              GPIOD

/* Definition for USART1's DMA */
#define USART2_TX_DMA_CHANNEL             DMA1_Channel7


/* Definition for USART1's NVIC */
#define USART2_DMA_TX_IRQn                DMA1_Channel7_IRQn
#define USART2_DMA_TX_IRQHandler          DMA1_Channel7_IRQHandler

/* Definition for USART1's NVIC */
#define USART2_IRQn                      USART2_IRQn
#define USART2_IRQHandler                USART2_IRQHandler

/* Definition for USART3 Pins */
#define USART3_CLK_ENABLE()              __HAL_RCC_USART3_CLK_ENABLE();
#define USART3_DMA_CLK_ENABLE()          __HAL_RCC_DMA1_CLK_ENABLE()
#define USART3_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOD_CLK_ENABLE()
#define USART3_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOD_CLK_ENABLE()

#define USART3_FORCE_RESET()             __HAL_RCC_USART3_FORCE_RESET()
#define USART3_RELEASE_RESET()           __HAL_RCC_USART3_RELEASE_RESET()


/* Definition for USARTx Pins */
#define USART3_TX_PIN                    GPIO_PIN_8
#define USART3_TX_GPIO_PORT              GPIOD
#define USART3_RX_PIN                    GPIO_PIN_9
#define USART3_RX_GPIO_PORT              GPIOD

/* Definition for USART1's DMA */
#define USART3_TX_DMA_CHANNEL             DMA1_Channel2


/* Definition for USART1's NVIC */
#define USART3_DMA_TX_IRQn                DMA1_Channel2_IRQn
//#define USART3_DMA_RX_IRQn                DMA1_Channel6_IRQn
#define USART3_DMA_TX_IRQHandler          DMA1_Channel2_IRQHandler
//#define USART3_DMA_RX_IRQHandler          DMA1_Channel6_IRQHandler

/* Definition for USART1's NVIC */
#define USART3_IRQn                      USART3_IRQn
#define USART3_IRQHandler                USART3_IRQHandler
/* Definition for USART4 Pins */

#define UART4_CLK_ENABLE()              __HAL_RCC_UART4_CLK_ENABLE();
#define UART4_DMA_CLK_ENABLE()          __HAL_RCC_DMA1_CLK_ENABLE()
#define UART4_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()
#define UART4_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()

#define UART4_FORCE_RESET()             __HAL_RCC_UART4_FORCE_RESET()
#define UART4_RELEASE_RESET()           __HAL_RCC_UART4_RELEASE_RESET()


/* Definition for USART4 Pins */
#define UART4_TX_PIN                    GPIO_PIN_10
#define UART4_TX_GPIO_PORT              GPIOC
#define UART4_RX_PIN                    GPIO_PIN_11
#define UART4_RX_GPIO_PORT              GPIOC

/* Definition for USART1's DMA */
#define UART4_TX_DMA_CHANNEL             DMA2_Channel5
#define UART4_RX_DMA_CHANNEL             DMA1_Channel6


/* Definition for USART1's NVIC */
#define UART4_DMA_TX_IRQn                DMA1_Channel5_IRQn
#define UART4_DMA_RX_IRQn                DMA1_Channel6_IRQn
#define UART4_DMA_TX_IRQHandler          DMA1_Channel5_IRQHandler
#define UART4_DMA_RX_IRQHandler          DMA1_Channel6_IRQHandler

/* Definition for USART1's NVIC */
#define UART4_IRQn                      UART4_IRQn
#define UART4_IRQHandler                UART4_IRQHandler


/* Definition for USART5 Pins */
#define UART5_CLK_ENABLE()              __HAL_RCC_UART5_CLK_ENABLE();
#define UART5_DMA_CLK_ENABLE()          __HAL_RCC_DMA1_CLK_ENABLE()
#define UART5_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOD_CLK_ENABLE()
#define UART5_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()

#define UART5_TX_PIN                    GPIO_PIN_12
#define UART5_TX_GPIO_PORT              GPIOC
#define UART5_RX_PIN                    GPIO_PIN_2
#define UART5_RX_GPIO_PORT              GPIOD

/* Definition for USART1's DMA */
#define UART5_TX_DMA_CHANNEL             DMA1_Channel7
#define UART5_RX_DMA_CHANNEL             DMA1_Channel6


/* Definition for USART1's NVIC */
#define UART5_DMA_TX_IRQn                DMA1_Channel7_IRQn
#define UART5_DMA_RX_IRQn                DMA1_Channel6_IRQn
#define UART5_DMA_TX_IRQHandler          DMA1_Channel7_IRQHandler
#define UART5_DMA_RX_IRQHandler          DMA1_Channel6_IRQHandler

/* Definition for USART1's NVIC */
#define UART5_IRQn                      UART5_IRQn
#define UART5_IRQHandler                UART5_IRQHandler

/*485·¢ËÍÊ¹ÄÜÅäÖÃ*/
#define UART2_EN_PIN                     	GPIO_PIN_5 //GPIO_PIN_0
#define UART2_EN_GPIO_PORT               	GPIOD//GPIOE
#define UART2_EN_CLK_ENABLE()            	__GPIOE_CLK_ENABLE()
#define UART2_Transmit_ENABLE()          	HAL_GPIO_WritePin(UART2_EN_GPIO_PORT,UART2_EN_PIN,GPIO_PIN_RESET)
#define UART2_Receive_ENABLE()           	HAL_GPIO_WritePin(UART2_EN_GPIO_PORT,UART2_EN_PIN,GPIO_PIN_SET)
//#define UART2_Transmit_ENABLE()           GPIOD->CRL = (GPIOD->CRL & ~(0x03UL << 22)) | (0x02UL << 22)
//#define UART2_Receive_ENABLE()            GPIOD->CRL &= ~(0x03UL << 22)

/*485·¢ËÍÊ¹ÄÜÅäÖÃ*/
#define UART3_EN_PIN                     	GPIO_PIN_10
#define UART3_EN_GPIO_PORT               	GPIOD
#define UART3_EN_CLK_ENABLE()            	__GPIOD_CLK_ENABLE()
#define UART3_Transmit_ENABLE()          	HAL_GPIO_WritePin(UART3_EN_GPIO_PORT,UART3_EN_PIN,GPIO_PIN_SET)
#define UART3_Receive_ENABLE()           	HAL_GPIO_WritePin(UART3_EN_GPIO_PORT,UART3_EN_PIN,GPIO_PIN_RESET)

/*485·¢ËÍÊ¹ÄÜÅäÖÃ*/
#define UART4_EN_PIN                     	GPIO_PIN_8
#define UART4_EN_GPIO_PORT               	GPIOF
#define UART4_EN_CLK_ENABLE()            	__GPIOF_CLK_ENABLE()
#define UART4_Transmit_ENABLE()          	HAL_GPIO_WritePin(UART4_EN_GPIO_PORT,UART4_EN_PIN,GPIO_PIN_SET)
#define UART4_Receive_ENABLE()           	HAL_GPIO_WritePin(UART4_EN_GPIO_PORT,UART4_EN_PIN,GPIO_PIN_RESET)

/*485·¢ËÍÊ¹ÄÜÅäÖÃ*/
#define UART5_EN_PIN                     	GPIO_PIN_3
#define UART5_EN_GPIO_PORT               	GPIOD
#define UART5_EN_CLK_ENABLE()            	__GPIOD_CLK_ENABLE()
#define UART5_Transmit_ENABLE()          	HAL_GPIO_WritePin(UART5_EN_GPIO_PORT,UART5_EN_PIN,GPIO_PIN_SET)
#define UART5_Receive_ENABLE()           	HAL_GPIO_WritePin(UART5_EN_GPIO_PORT,UART5_EN_PIN,GPIO_PIN_RESET)


#define RxTimeOutTIM3                          	TIM3
#define RxTimeOutTIM3_CLK_ENABLE               	__TIM3_CLK_ENABLE

#define MBRxTimeOutTIM                         	TIM5
#define MBRxTimeOutTIM_CLK_ENABLE             	__TIM5_CLK_ENABLE

#define SnmpRxTimeOutTIM                     	TIM6
#define SnmpRxTimeOutTIM_CLK_ENABLE          	__TIM6_CLK_ENABLE

/* Definition for TIMx's NVIC */
#define RxTimeOutTIM3_IRQn                     	TIM3_IRQn
#define RxTimeOutTIM3_IRQHandler               	TIM3_IRQHandler

#define MBRxTimeOutTIM_IRQn                    	TIM5_IRQn
#define MBRxTimeOutTIM_IRQHandler              	TIM5_IRQHandler

#define SnmpRxTimeOutTIM_IRQn                	TIM6_DAC_IRQn
#define SnmpRxTimeOutTIM_IRQHandler          	TIM6_DAC_IRQHandler

#define HAL_TIM_CLEAR_IT(__HANDLE__, __INTERRUPT__)            __HAL_TIM_CLEAR_IT(__HANDLE__, __INTERRUPT__)
#define HAL_TIM_SetCounter(__HANDLE__, __COUNTER__)            __HAL_TIM_SetCounter(__HANDLE__, __COUNTER__)
/******************************************************************************
* Data Type Definitions
******************************************************************************/

/******************************************************************************
* Global Variable Definitions
******************************************************************************/
BSP_DRIVER_UART_EXTERN TIM_HandleTypeDef  MBRxTimeOutTimHandle;

BSP_DRIVER_UART_EXTERN UART_HandleTypeDef Uart1Handle;
BSP_DRIVER_UART_EXTERN UART_HandleTypeDef Uart2Handle;
BSP_DRIVER_UART_EXTERN UART_HandleTypeDef Uart4Handle;
BSP_DRIVER_UART_EXTERN UART_HandleTypeDef Uart3Handle;
BSP_DRIVER_UART_EXTERN UART_HandleTypeDef Uart5Handle;

BSP_DRIVER_UART_EXTERN QueueType Uart1RxQueue;
BSP_DRIVER_UART_EXTERN QueueType Uart2RxQueue;
BSP_DRIVER_UART_EXTERN QueueType Uart3RxQueue;
BSP_DRIVER_UART_EXTERN QueueType Uart4RxQueue;
BSP_DRIVER_UART_EXTERN QueueType Uart5RxQueue;

BSP_DRIVER_UART_EXTERN QueueType Uart1TxQueue;
BSP_DRIVER_UART_EXTERN QueueType Uart2TxQueue;
BSP_DRIVER_UART_EXTERN QueueType Uart3TxQueue;
BSP_DRIVER_UART_EXTERN QueueType Uart4TxQueue;
BSP_DRIVER_UART_EXTERN QueueType Uart5TxQueue;


BSP_DRIVER_UART_EXTERN uint8_t Usart1_Rx_Bsp_buf[UART1_RX_SIZE];
BSP_DRIVER_UART_EXTERN uint8_t Usart2_Rx_Bsp_buf[UART2_RX_SIZE];
BSP_DRIVER_UART_EXTERN uint8_t Usart3_Rx_Bsp_buf[UART3_RX_SIZE];
BSP_DRIVER_UART_EXTERN uint8_t Usart4_Rx_Bsp_buf[UART4_RX_SIZE];
BSP_DRIVER_UART_EXTERN uint8_t Usart5_Rx_Bsp_buf[UART5_RX_SIZE];

BSP_DRIVER_UART_EXTERN uint8_t Usart1_Tx_Bsp_buf[UART1_TX_SIZE];
BSP_DRIVER_UART_EXTERN uint8_t Usart2_Tx_Bsp_buf[UART2_TX_SIZE];
BSP_DRIVER_UART_EXTERN uint8_t Usart3_Tx_Bsp_buf[UART3_TX_SIZE];
BSP_DRIVER_UART_EXTERN uint8_t Usart4_Tx_Bsp_buf[UART4_TX_SIZE];
BSP_DRIVER_UART_EXTERN uint8_t Usart5_Tx_Bsp_buf[UART5_TX_SIZE];
BSP_DRIVER_UART_EXTERN uint8_t Work_Mod;


int USART_Init(COM_NAME Port,           //com name
               STOP_BIT_NAME StopBits,  // stopbit
               uint8_t Parity,          //Parity
               uint32_t BaudRate);       //BaudRate

int Usart_Write(char port,uint8 *buf,uint16 len,uint32 timeout);

int Usart_Read(char port,uint8 *buf,uint16 len,uint32 timeout);
#ifdef __cplusplus
}
#endif /* extern "C" */

/*****************************************************************************/
#endif /* __BSP_DRIVER_UART_H */

/******************************************************************************
*                            end of file
******************************************************************************/

