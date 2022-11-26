/******************************************************************************
*  
*  File name:     BSP_RTC.c
*  
*  Version:       V1.00
*  Created on:    2014-12-15
*  Description:   
*  History:
*      Date        Author        Modification
*    1.2014-12-15  
*    2. ...
******************************************************************************/
#ifndef __BSP_CAN_C
#define __BSP_CAN_C

/******************************************************************************
* Include Files
******************************************************************************/
#include "BSP_Can.h"
#include "main.h"
#define SIZE_CNA_RX  10
CAN_HandleTypeDef    CanHandle;

static CanTxMsgTypeDef        TxMessage;
static CanRxMsgTypeDef        RxMessage;

static CanRxMsgTypeDef        RxMessagebuf[SIZE_CNA_RX];
typedef struct
{
    short int front;			// 
    short int  rear;			// 
	char  flag;					//
    int size;
	CanRxMsgTypeDef *elment;//元素
}CanQueueType;

static  CanQueueType can_Queue;
/******************************************************************************
* Static Variable Definitions
******************************************************************************/
static void Error_Handler(void)
{
        
}


/******************************************************************************
* Static Variable Definitions
******************************************************************************/
int InitCanQueue(CanQueueType *Q,CanRxMsgTypeDef *buf,int len)
{
	if(buf == NULL)
	{
		return 0;
	}

	Q->front = 0;
	Q->rear = 0;
	Q->elment = buf;
	Q->size = len;
	Q->flag = 0;
	return 1;
}



int Can_EnCanQueue(CanQueueType *Q, CanRxMsgTypeDef e)
{
	if(Q->size == 0)
	{
		return 0;
	}

	if((Q->front != Q->rear) || (Q->flag ==0))
	{
		Q->elment[Q->rear++] = e;
		if(Q->rear == Q->size)
		{
			Q->rear = 0;
		}
		if(Q->front == Q->rear)
		{
			Q->flag = 1;
		}
		return 1;
	}
	else
	{
		return 0;
	}

}

int Can_DeQueue(CanQueueType *Q, CanRxMsgTypeDef *e)
{
	if(Q->size == 0)
	{
		return 0;
	}

	if((Q->front != Q->rear) || (Q->flag ==1))
	{
		*e = Q->elment[Q->front++];

		if(Q->front == Q->size)
		{
			Q->front = 0;
		}
		if(Q->front != Q->rear)
		{
			Q->flag = 0;
		}

		return 1;
	}

	return 0;
}

/**
  * @brief CAN MSP Initialization
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration
  *           - NVIC configuration for DMA interrupt request enable
  * @param hcan: CAN handle pointer
  * @retval None
  */
void HAL_CAN_MspInit(CAN_HandleTypeDef *hcan)
{
    GPIO_InitTypeDef   GPIO_InitStruct;

    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* CAN1 Periph clock enable */
    CANx_CLK_ENABLE();
    /* Enable GPIO clock ****************************************/
    CANx_GPIO_CLK_ENABLE();
    /* Enable AFIO clock and Remap CAN PINs to PA11 and PA12*******/
    //CANx_AFIO_REMAP_CLK_ENABLE();
    //CANx_AFIO_REMAP_RX_TX_PIN();
    __HAL_RCC_AFIO_CLK_ENABLE();
    /*##-2- Configure peripheral GPIO ##########################################*/
    /* CAN1 TX GPIO pin configuration */
    GPIO_InitStruct.Pin = CANx_TX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Pull = GPIO_PULLUP;

    HAL_GPIO_Init(CANx_TX_GPIO_PORT, &GPIO_InitStruct);

    /* CAN1 RX GPIO pin configuration */
    GPIO_InitStruct.Pin = CANx_RX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_INPUT;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Pull = GPIO_PULLUP;

    HAL_GPIO_Init(CANx_RX_GPIO_PORT, &GPIO_InitStruct);

    /*##-3- Configure the NVIC #################################################*/
    /* NVIC configuration for CAN1 Reception complete interrupt */
    HAL_NVIC_SetPriority(CANx_RX_IRQn, 8, 0);
    HAL_NVIC_EnableIRQ(CANx_RX_IRQn);

    HAL_NVIC_SetPriority(CAN1_TX_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);
}

/**
  * @brief CAN MSP De-Initialization
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO to their default state
  * @param hcan: CAN handle pointer
  * @retval None
  */
void HAL_CAN_MspDeInit(CAN_HandleTypeDef *hcan)
{
    /*##-1- Reset peripherals ##################################################*/
    CANx_FORCE_RESET();
    CANx_RELEASE_RESET();

    /*##-2- Disable peripherals and GPIO Clocks ################################*/
    /* De-initialize the CAN1 TX GPIO pin */
    HAL_GPIO_DeInit(CANx_TX_GPIO_PORT, CANx_TX_PIN);
    /* De-initialize the CAN1 RX GPIO pin */
    HAL_GPIO_DeInit(CANx_RX_GPIO_PORT, CANx_RX_PIN);

    /*##-4- Disable the NVIC for CAN reception #################################*/
    HAL_NVIC_DisableIRQ(CANx_RX_IRQn);
}

void CANx_RX_IRQHandler(void)
{
  HAL_NVIC_ClearPendingIRQ(CAN1_RX0_IRQn);
  HAL_CAN_IRQHandler(&CanHandle);
  
}
void CAN1_TX_IRQHandler(void)
{
  HAL_NVIC_ClearPendingIRQ(CAN1_TX_IRQn);
  HAL_CAN_IRQHandler(&CanHandle);
}
void BSP_Can_Init(void)
{
    CAN_FilterConfTypeDef  sFilterConfig;


    /*##-1- Configure the CAN peripheral #######################################*/
    CanHandle.Instance = CANx;
    CanHandle.pTxMsg = &TxMessage;
    CanHandle.pRxMsg = &RxMessage;

    CanHandle.Init.TTCM = DISABLE;
    CanHandle.Init.ABOM = ENABLE;
    CanHandle.Init.AWUM = DISABLE;
    CanHandle.Init.NART = DISABLE;
    CanHandle.Init.RFLM = DISABLE;
    CanHandle.Init.TXFP = DISABLE;
    CanHandle.Init.Mode = CAN_MODE_NORMAL;
    CanHandle.Init.SJW = CAN_SJW_1TQ;
    CanHandle.Init.BS1 = CAN_BS1_8TQ;
    CanHandle.Init.BS2 = CAN_BS2_7TQ;
    CanHandle.Init.Prescaler = 18;
    //CAN 波特率 =APB1/BRP分频/(1+tBS1+tBS2)
    if (HAL_CAN_Init(&CanHandle) != HAL_OK)
    {
        /* Initiliazation Error */
        Error_Handler();
    }

    /*##-2- Configure the CAN Filter ###########################################*/
    sFilterConfig.FilterNumber = 0;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdHigh = 0x0000;
    sFilterConfig.FilterIdLow = 0x0000;
    sFilterConfig.FilterMaskIdHigh = 0x0000;
    sFilterConfig.FilterMaskIdLow = 0x0000;
    sFilterConfig.FilterFIFOAssignment = 0;
    sFilterConfig.FilterActivation = ENABLE;
    sFilterConfig.BankNumber = 14;

    InitCanQueue(&can_Queue,RxMessagebuf,SIZE_CNA_RX);
    
    if (HAL_CAN_ConfigFilter(&CanHandle, &sFilterConfig) != HAL_OK)
    {
        /* Filter configuration Error */
        Error_Handler();
    }
    if (HAL_CAN_Receive_IT(&CanHandle, CAN_FIFO0) != HAL_OK)
    {
         Error_Handler();
    }
    if (HAL_CAN_Transmit_IT(&CanHandle) != HAL_OK)
    {
        
    }    
    

}

/*********************************************************************************************************************
函数名称: 	Can_write          
功能描述: 	BMS信息处理
输　入:   	无

输　出:   	无                 
*********************************************************************************************************************/
uint8_t BSP_Can_write(CanTxMsgTypeDef TxMsg )
{	
	uint8_t ret=1;

	//hcan1.pTxMsg->StdId = 0x1806e5f4;
	CanHandle.pTxMsg->ExtId = TxMsg.ExtId;
	CanHandle.pTxMsg->RTR = TxMsg.RTR;
	CanHandle.pTxMsg->IDE = TxMsg.IDE;
	CanHandle.pTxMsg->DLC = TxMsg.DLC;

	CanHandle.pTxMsg->Data[0] = TxMsg.Data[0];
	CanHandle.pTxMsg->Data[1] = TxMsg.Data[1];
	CanHandle.pTxMsg->Data[2] = TxMsg.Data[2];
	CanHandle.pTxMsg->Data[3] = TxMsg.Data[3];
	CanHandle.pTxMsg->Data[4] = TxMsg.Data[4];
	CanHandle.pTxMsg->Data[5] = TxMsg.Data[5];
	CanHandle.pTxMsg->Data[6] = TxMsg.Data[6];
	CanHandle.pTxMsg->Data[7] = TxMsg.Data[7];

	if(HAL_CAN_Transmit_IT(&CanHandle)!= HAL_OK)
	{
        ret=0;
	}
	
	return ret;
}


void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef* hcan)
{
    if(hcan->Instance == CAN1)
    {   
        Can_EnCanQueue(&can_Queue,*(hcan->pRxMsg));
        
        //HAL_CAN_Receive_IT(hcan, CAN_FIFO0);
        if( HAL_BUSY == HAL_CAN_Receive_IT(hcan, CAN_FIFO0))//
        {  
            /* Enable FIFO 0 overrun and message pending Interrupt */  
            __HAL_CAN_ENABLE_IT(hcan, CAN_IT_FOV0 | CAN_IT_FMP0);  
        }         
    }
}

uint8_t BSP_Can_Read(CanRxMsgTypeDef *RxMsg )
{	
    return Can_DeQueue(&can_Queue,RxMsg);
}
uint32_t pclk2;// = HAL_RCC_GetPCLK2Freq();
void can_test_fun(void)
{
    #include "string.h"
    #include "bsp_Led_Function.h"
    pclk2 = HAL_RCC_GetPCLK2Freq();
    BSP_Can_Init();
    CanTxMsgTypeDef TxMsg ;
    CanRxMsgTypeDef RxMsg;
    
    while(1)
    {
        if(BSP_Can_Read(&RxMsg))
        {
            TxMsg.DLC = RxMsg.DLC;
            TxMsg.ExtId = RxMsg.ExtId;
            TxMsg.IDE = CAN_ID_EXT;
            TxMsg.RTR = CAN_RTR_DATA;
            memcpy(TxMsg.Data,RxMsg.Data,8);
            BSP_Can_write(TxMsg); 
            //dbg_printf(1,"can TEST OK");            
        }
        else
        {
//            TxMsg.DLC = 8;
//            TxMsg.ExtId = 0x00000000;
//            TxMsg.IDE = CAN_ID_EXT;
//            TxMsg.RTR = CAN_RTR_DATA;
//            memset(TxMsg.Data,9,8);
            //BSP_Can_write(TxMsg);
            
            //osDelay(500);//
            osDelay(5);//
            //LED_Function(BOARD_LED,LED_Toggle);   
        }
        
        
    }
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */


#endif

