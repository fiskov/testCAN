/**
  ******************************************************************************
  * File Name          : CAN.c
  * Description        : This file provides code for the configuration
  *                      of the CAN instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "can.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

CAN_HandleTypeDef hcan1;

/* CAN1 init function */
void MX_CAN1_Init(void)
{

  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 21;
  hcan1.Init.Mode = CAN_MODE_LOOPBACK;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_6TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_1TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = ENABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }

}

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**CAN1 GPIO Configuration
    PB8     ------> CAN1_RX
    PB9     ------> CAN1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* CAN1 interrupt Init */
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
    HAL_NVIC_SetPriority(CAN1_SCE_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN1_SCE_IRQn);
  /* USER CODE BEGIN CAN1_MspInit 1 */

  /* USER CODE END CAN1_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN1_CLK_DISABLE();

    /**CAN1 GPIO Configuration
    PB8     ------> CAN1_RX
    PB9     ------> CAN1_TX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8|GPIO_PIN_9);

    /* CAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_SCE_IRQn);
  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void CAN_InitFilter0(CAN_HandleTypeDef *p_hcan, int filterAddr, int filterGroup){	
	//фильтр на сообщения
  CAN_FilterTypeDef canFilterConfig;
	
  canFilterConfig.FilterBank = 0;
	canFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	canFilterConfig.FilterScale = CAN_FILTERSCALE_16BIT;
	if (filterGroup == 0) filterGroup = 0x7F; //если "0" - группа не определена, срабатывает только на broadcast
	canFilterConfig.FilterIdHigh = (0x0700 + filterGroup) << 5;
	canFilterConfig.FilterIdLow = (0x077F) << 5; // broadcast
	canFilterConfig.FilterMaskIdHigh = 0x077F << 5; 
	canFilterConfig.FilterMaskIdLow = 0x077F << 5;
	
  canFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  canFilterConfig.FilterActivation = ENABLE;
	if ( HAL_CAN_ConfigFilter(p_hcan, &canFilterConfig) != HAL_OK) Error_Handler();	
	
	//старт модуля передачи
  if ( HAL_CAN_Start(p_hcan) != HAL_OK) Error_Handler();
  if ( HAL_CAN_ActivateNotification(p_hcan, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) Error_Handler();
}
void CAN_InitFilter(CAN_HandleTypeDef *p_hcan, int filterAddr, int filterGroup){
	
	//фильтр на сообщения
  CAN_FilterTypeDef canFilterConfig;
	//CAN1 будет принимать в FIFO0, CAN2 в FIFO1
	canFilterConfig.FilterFIFOAssignment = ( p_hcan->Instance==CAN1 ? CAN_RX_FIFO0 : CAN_RX_FIFO1);
	canFilterConfig.SlaveStartFilterBank = 14;
	
	//на stm32f407vg CAN2 в адресном пространство CAN1, поэтому пишем в фильтр #14
  canFilterConfig.FilterBank = 0 + (p_hcan->Instance==CAN1 ? 0 : 14);
	
	//так как используется 16 битный фильтр, то можно определить 2 фильтра по маске (в High и в Low соответственно)
	//сдвиг на 5 - так устроен bxCAN в STM32	
	canFilterConfig.FilterMode = CAN_FILTERMODE_IDLIST;
	canFilterConfig.FilterScale = CAN_FILTERSCALE_16BIT;
	
	//так как используется 16 битный фильтр, то можно определить 4 идентификатора
	//сдвиг на 5 - так устроен bxCAN в STM32
	//фильтр по группе
  canFilterConfig.FilterIdHigh = (0x0200 + filterGroup) << 5; 
  canFilterConfig.FilterIdLow = (0x0500 + filterGroup) << 5;
  canFilterConfig.FilterMaskIdHigh = (0x0600 + filterGroup) << 5;
  canFilterConfig.FilterMaskIdLow = (0x0700 + filterGroup) << 5;
  
  //если группа=0, значит она "не определан", фильтр не включаем
	canFilterConfig.FilterActivation = (filterGroup > 0 ? ENABLE : DISABLE);
  HAL_CAN_ConfigFilter(p_hcan, &canFilterConfig);
	
	//фильтр по группе - broadcast
	canFilterConfig.FilterBank = 1 + (p_hcan->Instance==CAN1 ? 0 : 14); 
  canFilterConfig.FilterIdHigh = (0x0200 + 0x7F) << 5; 
  canFilterConfig.FilterIdLow = (0x0500 + 0x7F) << 5;
  canFilterConfig.FilterMaskIdHigh = (0x0600 + 0x7F) << 5;
  canFilterConfig.FilterMaskIdLow = (0x0700 + 0x7F) << 5;
  
	canFilterConfig.FilterActivation = ENABLE;
  HAL_CAN_ConfigFilter(p_hcan, &canFilterConfig);	

	//еще один фильтр - по адресу, команда 0x480. Для CAN2 фильтр #15	
  canFilterConfig.FilterBank = 2 + (p_hcan->Instance==CAN1 ? 0 : 14); 	
	//так как используется 16 битный фильтр, то надо определить 4 идентификатора
  canFilterConfig.FilterIdHigh = (0x0480 + filterAddr) << 5;	
  canFilterConfig.FilterIdLow = (0x0480 + filterAddr) << 5;
  canFilterConfig.FilterMaskIdHigh = (0x0480 + filterAddr) << 5;
  canFilterConfig.FilterMaskIdLow = (0x0480 + filterAddr) << 5;	
	
  HAL_CAN_ConfigFilter(p_hcan, &canFilterConfig);	
  
	//старт модуля передачи
  HAL_CAN_Start(p_hcan);
  HAL_CAN_ActivateNotification(p_hcan, 
		( p_hcan->Instance==CAN1 ? CAN_IT_RX_FIFO0_MSG_PENDING : CAN_IT_RX_FIFO1_MSG_PENDING) 
	);
}
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
