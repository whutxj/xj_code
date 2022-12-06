/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
//设置具备DNS的功能
#ifndef LWIP_DNS
#define LWIP_DNS 1
#endif


/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
//typedef unsigned   char    u8_t;
//typedef signed     char    s8_t;
//typedef unsigned   short   u16_t;
//typedef signed     short   s16_t;
//typedef unsigned   long    u32_t;
//typedef signed     long    s32_t;
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define RL1_ON_Pin GPIO_PIN_2
#define RL1_ON_GPIO_Port GPIOE
#define RL2_OFF_Pin GPIO_PIN_3
#define RL2_OFF_GPIO_Port GPIOE
#define Standby_state_led_Pin GPIO_PIN_4
#define Standby_state_led_GPIO_Port GPIOE
#define work_led_Pin GPIO_PIN_5
#define work_led_GPIO_Port GPIOE
#define error_led_Pin GPIO_PIN_6
#define error_led_GPIO_Port GPIOE
#define run_led_Pin GPIO_PIN_13
#define run_led_GPIO_Port GPIOC
#define Leakage_test_Pin GPIO_PIN_0
#define Leakage_test_GPIO_Port GPIOC
#define SPI2_MISO_Flash_Pin GPIO_PIN_2
#define SPI2_MISO_Flash_GPIO_Port GPIOC
#define SPI2_MOSI_Flash_Pin GPIO_PIN_3
#define SPI2_MOSI_Flash_GPIO_Port GPIOC
#define relay_connection_test1_Pin GPIO_PIN_0
#define relay_connection_test1_GPIO_Port GPIOA
#define relay_connection_test2_Pin GPIO_PIN_3
#define relay_connection_test2_GPIO_Port GPIOA
#define ji_dian_qi_T_Pin GPIO_PIN_4
#define ji_dian_qi_T_GPIO_Port GPIOA
#define huan_jing_T_Pin GPIO_PIN_5
#define huan_jing_T_GPIO_Port GPIOA
#define qiang_tou_T_Pin GPIO_PIN_6
#define qiang_tou_T_GPIO_Port GPIOA
#define AC_ZERO_RN8209D_Pin GPIO_PIN_0
#define AC_ZERO_RN8209D_GPIO_Port GPIOB
#define CP_VS_Pin GPIO_PIN_1
#define CP_VS_GPIO_Port GPIOB
#define BOOT1_Pin GPIO_PIN_2
#define BOOT1_GPIO_Port GPIOB
#define CP_OUT_Pin GPIO_PIN_8
#define CP_OUT_GPIO_Port GPIOE
#define CP_PWM_Pin GPIO_PIN_11
#define CP_PWM_GPIO_Port GPIOE
#define DI2_Pin GPIO_PIN_15
#define DI2_GPIO_Port GPIOE
#define SPI2_SCK_Flash_Pin GPIO_PIN_10
#define SPI2_SCK_Flash_GPIO_Port GPIOB
#define ETH_RESET_Pin GPIO_PIN_14
#define ETH_RESET_GPIO_Port GPIOB
#define RS485_TX_Pin GPIO_PIN_8
#define RS485_TX_GPIO_Port GPIOD
#define RS485_RX_Pin GPIO_PIN_9
#define RS485_RX_GPIO_Port GPIOD
#define RS485_CK_Pin GPIO_PIN_10
#define RS485_CK_GPIO_Port GPIOD
#define RESET_4G_Pin GPIO_PIN_14
#define RESET_4G_GPIO_Port GPIOD
#define POWER_4G_Pin GPIO_PIN_15
#define POWER_4G_GPIO_Port GPIOD
#define TX_4G_Pin GPIO_PIN_6
#define TX_4G_GPIO_Port GPIOC
#define RX_4G_Pin GPIO_PIN_7
#define RX_4G_GPIO_Port GPIOC
#define BT1_IO_Pin GPIO_PIN_9
#define BT1_IO_GPIO_Port GPIOC
#define BT1_RST_Pin GPIO_PIN_8
#define BT1_RST_GPIO_Port GPIOA
#define BT1_TX_Pin GPIO_PIN_9
#define BT1_TX_GPIO_Port GPIOA
#define BT1_RX_Pin GPIO_PIN_10
#define BT1_RX_GPIO_Port GPIOA
#define SPI3_NSS_RN8209D_Pin GPIO_PIN_15
#define SPI3_NSS_RN8209D_GPIO_Port GPIOA
#define card_reader_TX_Pin GPIO_PIN_10
#define card_reader_TX_GPIO_Port GPIOC
#define card_reader_RX_Pin GPIO_PIN_11
#define card_reader_RX_GPIO_Port GPIOC
#define Screen_TX_Pin GPIO_PIN_12
#define Screen_TX_GPIO_Port GPIOC
#define Screen_RX_Pin GPIO_PIN_2
#define Screen_RX_GPIO_Port GPIOD
#define SPI3_SCK_RN8209D_Pin GPIO_PIN_3
#define SPI3_SCK_RN8209D_GPIO_Port GPIOB
#define SPI3_MISO_RN8209D_Pin GPIO_PIN_4
#define SPI3_MISO_RN8209D_GPIO_Port GPIOB
#define SPI3_MOSI_RN8209D_Pin GPIO_PIN_5
#define SPI3_MOSI_RN8209D_GPIO_Port GPIOB
#define SPI2_NSS_Flash_Pin GPIO_PIN_9
#define SPI2_NSS_Flash_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
#define FILT_NUM   10 //AD的深度
#define CH_NUM     8   //AD的通道数

extern volatile unsigned short FiltAdcBuf[FILT_NUM][CH_NUM];
//extern volatile unsigned short FiltAdcLeakCurrBuf[FILT_NUM][CH_NUM];
extern IWDG_HandleTypeDef hiwdg;
extern RTC_HandleTypeDef hrtc;
extern SPI_HandleTypeDef hspi2;//flash的SPI
extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

extern CAN_HandleTypeDef hcan1;

extern IWDG_HandleTypeDef hiwdg;

extern RTC_HandleTypeDef hrtc;


extern SPI_HandleTypeDef hspi3;//这里是电量芯片的接口  SPI3
extern DMA_HandleTypeDef hdma_spi3_tx;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim4;

extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart1;
extern USART_HandleTypeDef husart3;
extern UART_HandleTypeDef huart6;
extern DMA_HandleTypeDef hdma_uart4_tx;
extern DMA_HandleTypeDef hdma_uart4_rx;
extern DMA_HandleTypeDef hdma_usart3_rx;
extern DMA_HandleTypeDef hdma_usart3_tx;
extern DMA_HandleTypeDef hdma_usart6_rx;
extern DMA_HandleTypeDef hdma_usart6_tx;

extern WWDG_HandleTypeDef hwwdg;

//extern osThreadId defaultTaskHandle;

//#define CH_CP  0
//#define CH_TEMP 1
//#define CH_GUN  2
//0 = 继电器粘连测试端口1  
//1 = 继电器粘连测试端口2   
//2 = 继电器温度  
//3=环境温度  
//4=枪头温度   
//5 = 计量芯片过零检测  
//6= CP-VS 
//7= 漏电检测端口


#define SWITCH_CONNNECT_NEG	0//0 = 继电器粘连测试端口1  
#define SWITCH_CONNNECT_POS	1//1 = 继电器粘连测试端口2  
#define SWITCH_TEMP	2//2 = 继电器温度  
#define CH_TEMP 3//=环境温度  
#define CH_GUN  4//4=枪头温度 
#define MEASURE_ZERO  5//5 = 计量芯片过零检测 
#define CH_CP  6  // CP的电压
#define LEAK_CURRENCE_PORT	7//7= 漏电检测端口




/* Private function prototypes -----------------------------------------------*/
void usr_IWDG_Init();
void usr_GPIO_Init();
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
