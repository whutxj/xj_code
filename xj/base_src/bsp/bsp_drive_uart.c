

#ifndef _BSP_DRIVER_UART_C_
#define _BSP_DRIVER_UART_C_

/******************************************************************************
* Include Files
******************************************************************************/
#include "main.h"
#include "bsp_driver_uart.h"
//#include "mbport.h"
//#include "portserial.h"
#include "Global.h"
#include "Sys_Config.h"

typedef struct
{
	USART_TypeDef *Instance;
	QueueType Txqueue;
	QueueType Rxqueue;

	char rs485;
	void (*rx_enable)(void);
	void (*tx_enable)(void);
    char rcv_flag;
}usart_info;

#define MODBUS_COM  UART5

static usart_info usart1;
 usart_info usart2;
static usart_info usart3;
static usart_info usart4;
static usart_info usart5;


/******************************************************************************
* Static Variable Definitions
******************************************************************************/

//static uint8_t RxData1,RxData2,RxData3,RxData4,RxData5;
extern void HAL_TIM4_Base_MspInit(void);
extern void HAL_TIM4_PeriodElapsedCallback(void);
/******************************************************************************
* Static Function Prototype Definitions
******************************************************************************/
//void HAL_TIM6_Base_MspInit(void);
//void HAL_TIM5_Base_MspInit(void);
//void HAL_UART_MspInit(UART_HandleTypeDef* huart)
//{
//    static DMA_HandleTypeDef hdma_tx1;

//    static DMA_HandleTypeDef hdma_tx2;
//    static DMA_HandleTypeDef hdma_tx3;
//    static DMA_HandleTypeDef hdma_tx4;
//    static DMA_HandleTypeDef hdma_tx5;

//    GPIO_InitTypeDef  GPIO_InitStruct;

//    if(huart->Instance == USART1)
//    {
//        /*##-1- Enable peripherals and GPIO Clocks #################################*/
//        /* Enable GPIO TX/RX clock */
//        USART1_TX_GPIO_CLK_ENABLE();
//        USART1_RX_GPIO_CLK_ENABLE();

//        /* Enable USARTx clock */
//        USART1_CLK_ENABLE();

//        /*##-2- Configure peripheral GPIO ##########################################*/
//        /* UART TX GPIO pin configuration  */
//        GPIO_InitStruct.Pin       = USART1_TX_PIN;
//        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
//        GPIO_InitStruct.Pull      = GPIO_PULLUP;
//        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;

//        HAL_GPIO_Init(USART1_TX_GPIO_PORT, &GPIO_InitStruct);

//        /* UART RX GPIO pin configuration  */
//        GPIO_InitStruct.Pin = USART1_RX_PIN;
//        GPIO_InitStruct.Mode      = GPIO_MODE_INPUT;

//        HAL_GPIO_Init(USART1_RX_GPIO_PORT, &GPIO_InitStruct);
//        #ifdef USR_DMA
//        /* Enable DMA clock */
//        USART1_DMA_CLK_ENABLE();

//        /*##-3- Configure the DMA ##################################################*/
//        /* Configure the DMA handler for Transmission process */
//        hdma_tx1.Instance                 = USART1_TX_DMA_CHANNEL;
//        hdma_tx1.Init.Direction           = DMA_MEMORY_TO_PERIPH;
//        hdma_tx1.Init.PeriphInc           = DMA_PINC_DISABLE;
//        hdma_tx1.Init.MemInc              = DMA_MINC_ENABLE;
//        hdma_tx1.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
//        hdma_tx1.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
//        hdma_tx1.Init.Mode                = DMA_NORMAL;
//        hdma_tx1.Init.Priority            = DMA_PRIORITY_LOW;

//        HAL_DMA_Init(&hdma_tx1);

//        /* Associate the initialized DMA handle to the UART handle */
//        __HAL_LINKDMA(huart, hdmatx, hdma_tx1);

//        /*##-4- Configure the NVIC for DMA #########################################*/
//        /* NVIC configuration for DMA transfer complete interrupt (USART2_TX) */
//        HAL_NVIC_SetPriority(USART1_DMA_TX_IRQn, 0, 1);
//        HAL_NVIC_EnableIRQ(USART1_DMA_TX_IRQn);
//        #endif

//        /* NVIC for USART, to catch the TX complete */
//        HAL_NVIC_SetPriority(USART1_IRQn, 8, 1);
//        HAL_NVIC_EnableIRQ(USART1_IRQn);

//    }
//    else if(huart->Instance == USART2)
//    {
//        /*##-1- Enable peripherals and GPIO Clocks #################################*/
//        /* Enable GPIO TX/RX clock */
//        USART2_TX_GPIO_CLK_ENABLE();
//        USART2_RX_GPIO_CLK_ENABLE();

//        __HAL_AFIO_REMAP_USART2_ENABLE();
//        AFIO->MAPR |= AFIO_MAPR_USART2_REMAP;

//        /* Enable USARTx clock */
//        USART2_CLK_ENABLE();

//        /*##-2- Configure peripheral GPIO ##########################################*/
//        /* UART TX GPIO pin configuration  */
//        GPIO_InitStruct.Pin       = USART2_TX_PIN;
//        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
//        GPIO_InitStruct.Pull      = GPIO_SPEED_FREQ_HIGH;
//        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;

//        HAL_GPIO_Init(USART2_TX_GPIO_PORT, &GPIO_InitStruct);

//        /* UART RX GPIO pin configuration  */
//        GPIO_InitStruct.Pin = USART2_RX_PIN;
//        GPIO_InitStruct.Mode      = GPIO_MODE_INPUT;

//        HAL_GPIO_Init(USART2_RX_GPIO_PORT, &GPIO_InitStruct);

//        #ifdef USR_DMA
//        //        /* Enable DMA clock */
//        USART2_DMA_CLK_ENABLE();

//        /*##-3- Configure the DMA ##################################################*/
//        /* Configure the DMA handler for Transmission process */
//        hdma_tx2.Instance                 = USART2_TX_DMA_CHANNEL;
//        hdma_tx2.Init.Direction           = DMA_MEMORY_TO_PERIPH;
//        hdma_tx2.Init.PeriphInc           = DMA_PINC_DISABLE;
//        hdma_tx2.Init.MemInc              = DMA_MINC_ENABLE;
//        hdma_tx2.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
//        hdma_tx2.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
//        hdma_tx2.Init.Mode                = DMA_NORMAL;
//        hdma_tx2.Init.Priority            = DMA_PRIORITY_LOW;

//        HAL_DMA_Init(&hdma_tx2);

//        /* Associate the initialized DMA handle to the UART handle */
//        __HAL_LINKDMA(huart, hdmatx, hdma_tx2);

//        /*##-4- Configure the NVIC for DMA #########################################*/
//        /* NVIC configuration for DMA transfer complete interrupt (USART2_TX) */
//        HAL_NVIC_SetPriority(USART2_DMA_TX_IRQn, 8, 1);
//        HAL_NVIC_EnableIRQ(USART2_DMA_TX_IRQn);
//        #endif

//        /* set UART2_TX high for RS485 in RX mode */
//        GPIOD->ODR |= (0x01UL << 5);
//        //UART2_Receive_ENABLE();

//        /* NVIC for USART, to catch the TX complete */
//        HAL_NVIC_SetPriority(USART2_IRQn, 8, 1);
//        HAL_NVIC_EnableIRQ(USART2_IRQn);
//    }
//    if(huart->Instance == USART3)
//    {
//        /*##-1- Enable peripherals and GPIO Clocks #################################*/
//        /* Enable GPIO TX/RX clock */
//        __HAL_RCC_AFIO_CLK_ENABLE();


//        USART3_TX_GPIO_CLK_ENABLE();
//        USART3_RX_GPIO_CLK_ENABLE();


//        __HAL_AFIO_REMAP_USART3_ENABLE();
//        /* Enable USARTx clock */
//        USART3_CLK_ENABLE();

//        /*##-2- Configure peripheral GPIO ##########################################*/
//        /* UART TX GPIO pin configuration  */
//        GPIO_InitStruct.Pin       = USART3_TX_PIN;
//        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
//        GPIO_InitStruct.Pull      = GPIO_PULLUP;
//        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;

//        HAL_GPIO_Init(USART3_TX_GPIO_PORT, &GPIO_InitStruct);

//        /* UART RX GPIO pin configuration  */
//        GPIO_InitStruct.Pin = USART3_RX_PIN;
//        GPIO_InitStruct.Mode      = GPIO_MODE_INPUT;

//        HAL_GPIO_Init(USART3_RX_GPIO_PORT, &GPIO_InitStruct);

//        #ifdef USR_DMA
//        /*##-3- Configure the DMA ##################################################*/
//        /* Configure the DMA handler for Transmission process */
//        hdma_tx3.Instance                 = USART3_TX_DMA_CHANNEL;
//        hdma_tx3.Init.Direction           = DMA_MEMORY_TO_PERIPH;
//        hdma_tx3.Init.PeriphInc           = DMA_PINC_DISABLE;
//        hdma_tx3.Init.MemInc              = DMA_MINC_ENABLE;
//        hdma_tx3.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
//        hdma_tx3.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
//        hdma_tx3.Init.Mode                = DMA_NORMAL;
//        hdma_tx3.Init.Priority            = DMA_PRIORITY_LOW;

//        HAL_DMA_Init(&hdma_tx3);

//        /* Associate the initialized DMA handle to the UART handle */
//        __HAL_LINKDMA(huart, hdmatx, hdma_tx3);

//        /*##-4- Configure the NVIC for DMA #########################################*/
//        /* NVIC configuration for DMA transfer complete interrupt (USART2_TX) */
//        HAL_NVIC_SetPriority(USART3_DMA_TX_IRQn, 0, 1);
//        HAL_NVIC_EnableIRQ(USART3_DMA_TX_IRQn);

//        #endif

//        /* NVIC for USART, to catch the TX complete */
//        HAL_NVIC_SetPriority(USART3_IRQn, 8, 1);
//        HAL_NVIC_EnableIRQ(USART3_IRQn);
//    }
//    if(huart->Instance == UART4)
//    {
//        /*##-1- Enable peripherals and GPIO Clocks #################################*/
//        /* Enable GPIO TX/RX clock */
//        UART4_TX_GPIO_CLK_ENABLE();
//        UART4_RX_GPIO_CLK_ENABLE();

////        __HAL_RCC_AFIO_CLK_ENABLE();
////        AFIO->MAPR |= AFIO_MAPR_USART2_REMAP;

//        /* Enable USARTx clock */
//        UART4_CLK_ENABLE();



//        /*##-2- Configure peripheral GPIO ##########################################*/
//        /* UART TX GPIO pin configuration  */
//        GPIO_InitStruct.Pin       = UART4_TX_PIN;
//        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
//        GPIO_InitStruct.Pull      = GPIO_PULLUP;
//        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;

//        HAL_GPIO_Init(UART4_TX_GPIO_PORT, &GPIO_InitStruct);

//        /* UART RX GPIO pin configuration  */
//        GPIO_InitStruct.Pin = UART4_RX_PIN;
//        GPIO_InitStruct.Mode      = GPIO_MODE_INPUT;

//        HAL_GPIO_Init(UART4_RX_GPIO_PORT, &GPIO_InitStruct);

//        #ifdef USR_DMA
//        /* Enable DMA clock */
//        UART4_DMA_CLK_ENABLE();

//        /*##-3- Configure the DMA ##################################################*/
//        /* Configure the DMA handler for Transmission process */
//        hdma_tx4.Instance                 = UART4_TX_DMA_CHANNEL;
//        hdma_tx4.Init.Direction           = DMA_MEMORY_TO_PERIPH;
//        hdma_tx4.Init.PeriphInc           = DMA_PINC_DISABLE;
//        hdma_tx4.Init.MemInc              = DMA_MINC_ENABLE;
//        hdma_tx4.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
//        hdma_tx4.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
//        hdma_tx4.Init.Mode                = DMA_NORMAL;
//        hdma_tx4.Init.Priority            = DMA_PRIORITY_LOW;

//        HAL_DMA_Init(&hdma_tx4);

//        /* Associate the initialized DMA handle to the UART handle */
//        __HAL_LINKDMA(huart, hdmatx, hdma_tx4);

//        /*##-4- Configure the NVIC for DMA #########################################*/
//        /* NVIC configuration for DMA transfer complete interrupt (USART2_TX) */
//        HAL_NVIC_SetPriority(UART4_DMA_TX_IRQn, 0, 1);
//        HAL_NVIC_EnableIRQ(UART4_DMA_TX_IRQn);

//        /* NVIC configuration for DMA transfer complete interrupt (USART2_RX) */
//        HAL_NVIC_SetPriority(UART4_DMA_RX_IRQn, 8, 0);
//        HAL_NVIC_EnableIRQ(UART4_DMA_RX_IRQn);
//        #endif

//        /* NVIC for USART, to catch the TX complete */
//        HAL_NVIC_SetPriority(UART4_IRQn, 8, 1);
//        HAL_NVIC_EnableIRQ(UART4_IRQn);
//    }
//    if(huart->Instance == UART5)
//    {
//        /*##-1- Enable peripherals and GPIO Clocks #################################*/
//        /* Enable GPIO TX/RX clock */
//        UART5_TX_GPIO_CLK_ENABLE();
//        UART5_RX_GPIO_CLK_ENABLE();

////        __HAL_RCC_AFIO_CLK_ENABLE();
////        AFIO->MAPR |= AFIO_MAPR_UART5_REMAP;

//        /* Enable USARTx clock */
//        UART5_CLK_ENABLE();



//        /*##-2- Configure peripheral GPIO ##########################################*/
//        /* UART TX GPIO pin configuration  */
//        GPIO_InitStruct.Pin       = UART5_TX_PIN;
//        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
//        GPIO_InitStruct.Pull      = GPIO_PULLUP;
//        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;

//        HAL_GPIO_Init(UART5_TX_GPIO_PORT, &GPIO_InitStruct);

//        /* UART RX GPIO pin configuration  */
//        GPIO_InitStruct.Pin = UART5_RX_PIN;
//        GPIO_InitStruct.Mode      = GPIO_MODE_INPUT;

//        HAL_GPIO_Init(UART5_RX_GPIO_PORT, &GPIO_InitStruct);

//        #ifdef USR_DMA

//        /* Enable DMA clock */
//        UART5_DMA_CLK_ENABLE();

//        /*##-3- Configure the DMA ##################################################*/
//        /* Configure the DMA handler for Transmission process */
//        hdma_tx5.Instance                 = UART5_TX_DMA_CHANNEL;
//        hdma_tx5.Init.Direction           = DMA_MEMORY_TO_PERIPH;
//        hdma_tx5.Init.PeriphInc           = DMA_PINC_DISABLE;
//        hdma_tx5.Init.MemInc              = DMA_MINC_ENABLE;
//        hdma_tx5.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
//        hdma_tx5.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
//        hdma_tx5.Init.Mode                = DMA_NORMAL;
//        hdma_tx5.Init.Priority            = DMA_PRIORITY_LOW;

//        HAL_DMA_Init(&hdma_tx5);

//        /* Associate the initialized DMA handle to the UART handle */
//        __HAL_LINKDMA(huart, hdmatx, hdma_tx5);

//        /*##-4- Configure the NVIC for DMA #########################################*/
//        /* NVIC configuration for DMA transfer complete interrupt (USART2_TX) */
//        HAL_NVIC_SetPriority(UART5_DMA_TX_IRQn, 0, 1);
//        HAL_NVIC_EnableIRQ(UART5_DMA_TX_IRQn);

//        /* NVIC configuration for DMA transfer complete interrupt (USART2_RX) */
//        HAL_NVIC_SetPriority(UART5_DMA_RX_IRQn, 0, 0);
//        HAL_NVIC_EnableIRQ(UART5_DMA_RX_IRQn);
//        #endif

//        /* NVIC for USART, to catch the TX complete */
//        HAL_NVIC_SetPriority(UART5_IRQn, 8, 1);
//        HAL_NVIC_EnableIRQ(UART5_IRQn);
//    }
//}


void usart2_TxEable()
{
  UART2_Transmit_ENABLE() ;
    //buf[0] = Usart1_Tx_Bsp_buf;
	usart2.Instance->CR1 &= ~(1<<2);
}

void usart2_RxEable()
{
  UART2_Receive_ENABLE() ;
	usart2.Instance->CR1 |= (1<<2);
}

void usart3_TxEable()
{
	UART3_Transmit_ENABLE() ;
}
void usart3_RxEable()
{
	UART3_Receive_ENABLE() ;
}

void usart5_TxEable()
{
	//UART5_Transmit_ENABLE() ;
	usart5.Instance->CR1 &= ~(1<<2);
}
void usart5_RxEable()
{
	//UART5_Receive_ENABLE() ;
	usart5.Instance->CR1 |= (1<<2);
}


void usart1_bsp_data_init()
{
    // usart 1 232

    InitQueue(&Uart1TxQueue,Usart1_Tx_Bsp_buf,sizeof(Usart1_Tx_Bsp_buf));
	InitQueue(&Uart1RxQueue,Usart1_Rx_Bsp_buf,sizeof(Usart1_Rx_Bsp_buf));

	usart1.Instance = USART1;
	usart1.Rxqueue = Uart1RxQueue;
	usart1.Txqueue = Uart1TxQueue;
	usart1.rs485 = 0;
}
void usart2_bsp_data_init()
{
    // usart 2 232 //test
	InitQueue(&Uart2TxQueue,Usart2_Tx_Bsp_buf,sizeof(Usart2_Tx_Bsp_buf));
	InitQueue(&Uart2RxQueue,Usart2_Rx_Bsp_buf,sizeof(Usart2_Rx_Bsp_buf));

	usart2.Instance = USART2;
	usart2.Rxqueue = Uart2RxQueue;
	usart2.Txqueue = Uart2TxQueue;
 	usart2.rx_enable = usart2_RxEable;
	usart2.tx_enable = usart2_TxEable;
	usart2.rs485 = 1;
}

void usart3_bsp_data_init()
{
    // usart 3 585

    InitQueue(&Uart3TxQueue,Usart3_Tx_Bsp_buf,sizeof(Usart3_Tx_Bsp_buf));
	InitQueue(&Uart3RxQueue,Usart3_Rx_Bsp_buf,sizeof(Usart3_Rx_Bsp_buf));

	usart3.Instance = USART3;
	usart3.Rxqueue = Uart3RxQueue;
	usart3.Txqueue = Uart3TxQueue;
 	usart3.rx_enable = usart3_RxEable;
	usart3.tx_enable = usart3_TxEable;
	usart3.rs485 = 1;
}
void usart4_bsp_data_init()
{
    // usart 4 232

    InitQueue(&Uart4TxQueue,Usart4_Tx_Bsp_buf,sizeof(Usart4_Tx_Bsp_buf));
    InitQueue(&Uart4RxQueue,Usart4_Rx_Bsp_buf,sizeof(Usart4_Rx_Bsp_buf));

    usart4.Instance = UART4;
    usart4.Rxqueue = Uart4RxQueue;
    usart4.Txqueue = Uart4TxQueue;
    usart4.rs485 = 0;
}
void usart5_bsp_data_init()
{
    // usart 5 485

    InitQueue(&Uart5TxQueue,Usart5_Tx_Bsp_buf,sizeof(Usart5_Tx_Bsp_buf));
	InitQueue(&Uart5RxQueue,Usart5_Rx_Bsp_buf,sizeof(Usart5_Rx_Bsp_buf));

	usart5.Instance = UART5;
	usart5.Rxqueue = Uart5RxQueue;
	usart5.Txqueue = Uart5TxQueue;
 	//usart5.rx_enable = usart5_RxEable;
	//usart5.tx_enable = usart5_TxEable;       
	usart5.rs485 = 0; 


}



void usart_irq_proc(usart_info *info)
{
	uint16 RegData = 0;
	uint16 data = 0;

	RegData = info->Instance->SR;
	/*
	 It is cleared by a software sequence (an read to the
		USART_SR register followed by a read to the USART_DR register

	*/
	if((RegData & (0x0f)) != 0)// over err ;parity err,
	{
		data = info->Instance->DR & 0xffff;
        return;
	}
  
	if((RegData & (1<<4)) != 0)// over err ;parity err,
	{
		data = info->Instance->DR & 0xffff;
		//EnQueue(&info->Rxqueue,data&0xff);
        info->rcv_flag = 1;
	}

	if(RegData & (1<<5))//rxne
	{
		data = (info->Instance->DR & 0xffff);

		EnQueue(&info->Rxqueue,data&0xff);

        // add
        #if 0
        if(info->Instance == MODBUS_COM)
        {
            MBRxCharBuf = data;
            if(Work_Mod == MODBUS_SLAVER)
            {
              prvvMBPortRxISR();
            }
            else
            {
              prvvUARTRxISR();
            }
        }
				#endif
	}
	if(RegData & (1<<6))//tc
	{
        info->Instance->SR &=	~(1<<6);//清发送完成中断

 		if(DeQueue(&info->Txqueue,(uint8*)&data))
		{//send data
			info->Instance->DR = data;
		}
        else
        {
            info->Instance->CR1 &=	~(1<<6);//清发送完成中断
            if(info->rs485)
            {
                if(info->rx_enable)
                {// rx_enable
                    info->rx_enable();
										info->Instance->CR1 &=	~(1<<3);//
                }
            }
            #if 0
            if(info->Instance == MODBUS_COM)
            {
                if(Work_Mod == MODBUS_SLAVER)
                {
                    prvvMBPortTxISR();
                }
                else
                {
                    prvvUARTTxReadyISR();
                }
            }
						#endif

        }
	}

	if(RegData & (1<< 3))
	{
		RegData = info->Instance->SR;
		RegData = info->Instance->CR1;
		RegData = info->Instance->CR2;
	}

}

void USART1_IRQHandler(void)
{
    NVIC_ClearPendingIRQ(USART1_IRQn);
	usart_irq_proc(&usart1);
}

void UART4_IRQHandler(void)
{
  NVIC_ClearPendingIRQ(UART4_IRQn);
  usart_irq_proc(&usart4);
}
void USART2_IRQHandler(void)
{
	NVIC_ClearPendingIRQ(USART2_IRQn);
	usart_irq_proc(&usart2);
}

void UART5_IRQHandler(void)
{
	NVIC_ClearPendingIRQ(UART5_IRQn);
	usart_irq_proc(&usart5);
}

void USART3_IRQHandler(void)
{
	NVIC_ClearPendingIRQ(USART3_IRQn);
	usart_irq_proc(&usart3);
}


static uint16 usart_send(usart_info *info,uint8 *buf,uint16 len,uint32 timeout)
{
	int i = 0;
	uint32  tick = 0;
    tick = xTaskGetTickCount();
	if(len == 0)
        return 0;
	if(QueueEmpty(&info->Txqueue))
	{//空
        int ret = 0;
		if(info->rs485)
		{//假如是485
			if(info->tx_enable)
			{
				info->tx_enable();
				info->Instance->CR1 |=	(1<<3);//
			}
		}
		__disable_irq();
		for(i = 1;i<len;i++)
		{
			if(EnQueue(&info->Txqueue,buf[i]) == FALSE)
            {
                break;
            }
		}

		//enable send
        ret = info->Instance->SR;
		info->Instance->CR1 |=  (1<<6);
		//info->Instance->CR1 |=  (1<<7);
        info->Instance->DR = buf[0];
		__enable_irq();
		for(;i<len;)
		{
			if(EnQueue(&info->Txqueue,buf[i]) == FALSE)
            {
                vTaskDelay(5);
            }
            else
            {
                i++;
            }
            if(xTaskGetTickCount() - tick > timeout)
            {
                return i;
            }
		}
        ret = ret;
		return i;
	}
	else
	{

		while((xTaskGetTickCount() - tick < timeout)&&(i < len))
		{
			if(QueueFull(&info->Txqueue) == 0)
			{
				EnQueue(&info->Txqueue,buf[i++]);
			}
			else
			{
				vTaskDelay(5);
			}
		}

		return i;
	}
}

#if 0
static int usart_rcv(usart_info *info,uint8 *buf,uint16 len,uint32 timeout)
{
	int i = 0;
    int waite = 0;
	uint32  tick = 0;

	if(QueueLength(&info->Rxqueue) >= len)
	{
		for(i = 0; i < len;i++)
		{
			DeQueue(&info->Rxqueue,buf + i);
		}
        
        if(QueueLength(&info->Rxqueue) == 0)
        {
            info->rcv_flag = 0;
        }

		return len;
	}
	else
	{
        tick = xTaskGetTickCount();

		//do
		while(1)
		{
            if( info->rcv_flag)
            {
                if(DeQueue(&info->Rxqueue,buf + i) == 1)
                {
                  i++;
                }
                else
                {
                  info->rcv_flag = 0;
                  break;
                }
                if(i >= len)
                {
                   break;
                }
            }
            else if(xTaskGetTickCount() - tick < timeout)
            {
              vTaskDelay(1);
            }
            else
            {
                break;
            }
            
		}//while((xTaskGetTickCount() - tick < timeout)&&(i < len));
	}

    if(QueueLength(&info->Rxqueue) == 0)
    {
        info->rcv_flag = 0;
    }

	return i;

}

#endif
static int usart_rcv(usart_info *info,uint8 *buf,uint16 len,uint32 timeout)
{
	int i = 0;
    int waite = 0;
	uint32  tick = 0;
  
    tick = xTaskGetTickCount();
		//do
		
		while(1)
		{
          if( info->rcv_flag)
          {
              if(waite == 0)
              {
                //dbg_printf(1,"com %d",QueueLength(&info->Rxqueue));
                waite = 1;
              }
          }
          if(DeQueue(&info->Rxqueue,buf + i) == 1)
          {
              i++;
              if(i >= len)
              {
                 break;
              }            
          }          
          else if(xTaskGetTickCount() - tick < timeout)
          {
             vTaskDelay(1); 
          }
          else
          {
              break;
          }
            
		}
	
    if(QueueLength(&info->Rxqueue) == 0)
    {
        info->rcv_flag = 0;
    }

	return i;

}

/*------------------------------------------------------------
操作目的：   发送数据
初始条件：   无
操作结果：
函数参数：
        port 串口号
        buf 发送的数据
        len 发送的长度
        timeout 超时时间 tick

返回值：
        实际发送的长度
------------------------------------------------------------*/
int Usart_Write(char port,uint8 *buf,uint16 len,uint32 timeout)
{
	int ret = 0;

	switch(port)
	{
		case COM1:
		{
			ret  = usart_send(&usart1,buf,len,timeout);

			break;
		}
		case COM2:
		{
			ret  = usart_send(&usart2,buf,len,timeout);

			break;
		}
		case COM3:
		{
			ret  = usart_send(&usart3,buf,len,timeout);

			break;
		}
		case COM4:
		{
			ret  = usart_send(&usart4,buf,len,timeout);

			break;
		}
		case COM5:
		{
			ret  = usart_send(&usart5,buf,len,timeout);

			break;
		}

		default:
			break;
	}
    if(ret > 0)
        ret = 1;
	return ret;

}
/*------------------------------------------------------------
操作目的：   读串口数据
初始条件：   无
操作结果：
函数参数：
        port 串口号
        buf 读出的数据
        len 想要读的长度
        timeout 超时时间 tick

返回值：
        实际返回的长度
------------------------------------------------------------*/
int Usart_Read(char port,uint8 *buf,uint16 len,uint32 timeout)
{
	int ret = 0;

	switch(port)
	{
		case COM1:
		{
			ret  = usart_rcv(&usart1,buf,len,timeout);
			break;
		}
		case COM2:
		{
			ret  = usart_rcv(&usart2,buf,len,timeout);
			break;
		}
		case COM3:
		{
			ret  = usart_rcv(&usart3,buf,len,timeout);
			break;
		}
		case COM4:
		{
			ret  = usart_rcv(&usart4,buf,len,timeout);
			break;
		}
		case COM5:
		{
			ret  = usart_rcv(&usart5,buf,len,timeout);
			break;
		}
		default:
			break;
	}

	return ret;
}

///******************************************************************************
//* Function: void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
//* Parameter:
//* Return:   Void
//* Description:
//*   TIM2 GPIO Configuration.
//******************************************************************************/
//void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
//{
////    if(htim_base->Instance==TIM1)
////    {
////        HAL_TIM1_Base_MspInit();
////    }
////    else if(htim_base->Instance==TIM3)//串口接收超时定时器
////    {
////        HAL_TIM3_Base_MspInit();
////    }
//    if(htim_base->Instance==TIM4)
//    {
//        HAL_TIM4_Base_MspInit();
//    }
////    if(htim_base->Instance==TIM5)//Modbus接收超时定时器
////    {
////        HAL_TIM5_Base_MspInit();
////    }
//    else if(htim_base->Instance==TIM6)//Modbus-wifi接收超时定时器
//    {
//        HAL_TIM6_Base_MspInit();
//    }

//}



///************************ Timer5 ****************************/
//void HAL_TIM5_PeriodElapsedCallback(void)
//{
////    if(Work_Mod == MODBUS_SLAVER)
////    {
////        vPortTimerISR();
////    }
////    else
////    {
////        prvvTIMERExpiredISR();
////    }
//}

//void UART_SetBaudRate(int Ch,int BoardRate)
//{

//}

//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
//{
//	if(htim->Instance==TIM4)
//	{
//		HAL_TIM4_PeriodElapsedCallback();
//	}
//	else if(htim->Instance==TIM5)
//	{
//		HAL_TIM5_PeriodElapsedCallback();
//	}
//	else if(htim->Instance==TIM6)
//	{
//		//HAL_TIM6_PeriodElapsedCallback();
//		HAL_TIM6_PeriodElapsedCallback();
//	}
//}




int USART_Init(COM_NAME Port,           //com name
               STOP_BIT_NAME StopBits,  // stopbit
               uint8_t Parity,          //Parity
               uint32_t BaudRate)       //BaudRate
{
    UART_HandleTypeDef *UartHandle;
    GPIO_InitTypeDef  GPIO_InitStruct;

    if(Port == COM1)
    {
        UartHandle = &huart1;
        UartHandle->Instance        = USART1;
    }
    else if(Port == COM2)
    {
        UartHandle = &Uart2Handle;
        UartHandle->Instance        = USART2;
    }
    else if(Port == COM3)
    {
        UartHandle = &Uart3Handle;
        UartHandle->Instance        = USART3;
    }
    else if(Port == COM4)
    {
        UartHandle = &Uart4Handle;
        UartHandle->Instance        = UART4;
    }
    else if(Port == COM5)
    {
        UartHandle = &Uart5Handle;
        UartHandle->Instance        = UART5;
    }
    else
    {
        return 0;
    }

    /*2. Parity select*/
    if(P_NONE == Parity)
    {
        UartHandle->Init.WordLength = UART_WORDLENGTH_8B;
        UartHandle->Init.Parity     = UART_PARITY_NONE;
    }
    else if(P_ODD == Parity)
    {
        UartHandle->Init.WordLength = UART_WORDLENGTH_9B;
        UartHandle->Init.Parity     = UART_PARITY_ODD;
        //UartHandle->Init.WordLength = UART_WORDLENGTH_8B;
    }
    else if(P_EVEN == Parity)
    {
        UartHandle->Init.WordLength = UART_WORDLENGTH_9B;
        UartHandle->Init.Parity     = UART_PARITY_EVEN;

    }
    else
    {
        return 0;
    }


    UartHandle->Init.BaudRate   = BaudRate;

    UartHandle->Init.StopBits   = UART_STOPBITS_1;
    UartHandle->Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    UartHandle->Init.Mode       = UART_MODE_TX_RX;
    UartHandle->Init.OverSampling = UART_OVERSAMPLING_16;
    if(HAL_UART_DeInit(UartHandle) != HAL_OK)
    {
        return 0;
    }
    if(HAL_UART_Init(UartHandle) != HAL_OK)
    {
        return 0;
    }

//    //aa
//    huart2.Instance = UART5;
//    huart2.Init.BaudRate = BaudRate;    //波特率
//    huart2.Init.WordLength = UART_WORDLENGTH_9B;//UART_WORDLENGTH_8B;
//    huart2.Init.StopBits = UART_STOPBITS_1;
//    huart2.Init.Parity = UART_PARITY_EVEN;//UART_PARITY_NONE;//UART_PARITY_EVEN;
//    huart2.Init.Mode = UART_MODE_TX_RX;
//    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
//    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
//    HAL_UART_Init(&huart2);


    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;

    if(Port == COM1)
    {
//        USART1_CLK_ENABLE();
//        GPIO_InitStruct.Pin = USART1_TX_PIN;
//        HAL_GPIO_Init(USART1_TX_GPIO_PORT, &GPIO_InitStruct);
//        USART1_TX_GPIO_CLK_ENABLE();
//
//        GPIO_InitStruct.Pin = USART1_RX_PIN;
//        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//        HAL_GPIO_Init(USART1_RX_GPIO_PORT, &GPIO_InitStruct);
//        USART1_RX_GPIO_CLK_ENABLE();
//        
//        InitQueue(&Uart1RxQueue,Usart1_Rx_Bsp_buf,sizeof(Usart1_Rx_Bsp_buf));
        __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
        __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);

        usart1_bsp_data_init();
    }
    if(Port == COM2)
    {

//        UART2_EN_CLK_ENABLE();
//
//        GPIO_InitStruct.Pin = UART2_EN_PIN;
//        HAL_GPIO_Init(UART2_EN_GPIO_PORT, &GPIO_InitStruct);
				usart2_bsp_data_init();

//        InitQueue(&Uart2RxQueue,Usart2_Rx_Bsp_buf,sizeof(Usart2_Rx_Bsp_buf));
//        // test
//        UART2_Receive_ENABLE();

        __HAL_UART_ENABLE_IT(&Uart2Handle, UART_IT_RXNE);
        __HAL_UART_ENABLE_IT(&Uart2Handle, UART_IT_IDLE);

//        __GPIOB_CLK_ENABLE();
//        GPIO_InitStruct.Pin = GPIO_PIN_5;
//        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
				usart2.Instance->CR1 &=	~(1<<3);//

    }
    else if(Port == COM3)
    {

        //UART3_EN_CLK_ENABLE();
        //GPIO_InitStruct.Pin = UART3_EN_PIN;
        //HAL_GPIO_Init(UART3_EN_GPIO_PORT, &GPIO_InitStruct);

        InitQueue(&Uart3RxQueue,Usart3_Rx_Bsp_buf,sizeof(Usart3_Rx_Bsp_buf));
        __HAL_UART_ENABLE_IT(&Uart3Handle, UART_IT_RXNE);
        __HAL_UART_ENABLE_IT(&Uart3Handle, UART_IT_IDLE);
        usart3_bsp_data_init();
    }
    else if(Port == COM4)
    {

//        UART4_CLK_ENABLE();
//        GPIO_InitStruct.Pin = UART4_EN_PIN;
//
//        HAL_GPIO_Init(UART4_EN_GPIO_PORT, &GPIO_InitStruct);
//
        InitQueue(&Uart4RxQueue,Usart4_Rx_Bsp_buf,sizeof(Usart4_Rx_Bsp_buf));
        __HAL_UART_ENABLE_IT(&Uart4Handle, UART_IT_RXNE);
        __HAL_UART_ENABLE_IT(&Uart4Handle, UART_IT_IDLE);

        usart4_bsp_data_init();
    }
    else if(Port == COM5)
    {

        //UART5_EN_CLK_ENABLE();
        //GPIO_InitStruct.Pin = UART5_EN_PIN;
        //HAL_GPIO_Init(UART5_EN_GPIO_PORT, &GPIO_InitStruct);
        InitQueue(&Uart5RxQueue,Usart5_Rx_Bsp_buf,sizeof(Usart5_Rx_Bsp_buf));
        __HAL_UART_ENABLE_IT(&Uart5Handle, UART_IT_RXNE);
        __HAL_UART_ENABLE_IT(&Uart5Handle, UART_IT_IDLE);

        usart5_bsp_data_init();
    }
    else
    {
        return 0;
    }
    //add




    return 1;

}

#endif /* BSP_DRIVER_UART_C */

/******************************************************************************
*                            end of file
******************************************************************************/
