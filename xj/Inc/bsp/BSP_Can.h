
#ifndef __BSP_CAN_H
#define __BSP_CAN_H

/*****************************************************************************/
#ifdef __cplusplus
 extern "C" {
#endif

/******************************************************************************
* Includes
******************************************************************************/
#include "main.h"
     
#ifndef __BSP_CAN_C
#define BSP_CAN_EXTERN  extern
#else
#define BSP_CAN_EXTERN
#endif
// can 1
#define CANx                            CAN1
#define CANx_CLK_ENABLE()               __HAL_RCC_CAN1_CLK_ENABLE()
#define CANx_GPIO_CLK_ENABLE()          __HAL_RCC_GPIOA_CLK_ENABLE()

#define CANx_FORCE_RESET()              __HAL_RCC_CAN1_FORCE_RESET()
#define CANx_RELEASE_RESET()            __HAL_RCC_CAN1_RELEASE_RESET()

/* Definition for USARTx Pins */
#define CANx_TX_PIN                    GPIO_PIN_12
#define CANx_TX_GPIO_PORT              GPIOA
#define CANx_RX_PIN                    GPIO_PIN_11
#define CANx_RX_GPIO_PORT              GPIOA

/* Definition for AFIO Remap */
#define CANx_AFIO_REMAP_CLK_ENABLE()   __HAL_RCC_AFIO_CLK_ENABLE()
#define CANx_AFIO_REMAP_RX_TX_PIN()    __HAL_AFIO_REMAP_CAN1_1()

/* Definition for USARTx's NVIC */
#define CANx_RX_IRQn                   USB_LP_CAN1_RX0_IRQn
#define CANx_RX_IRQHandler             USB_LP_CAN1_RX0_IRQHandler


void BSP_Can_Init(void);

uint8_t BSP_Can_Read(CanRxMsgTypeDef *RxMsg);

uint8_t BSP_Can_write(CanTxMsgTypeDef TxMsg);

#ifdef __cplusplus
	}
#endif
	
#endif

