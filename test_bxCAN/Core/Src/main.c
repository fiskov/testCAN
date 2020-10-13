/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "can.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
HAL_StatusTypeDef CANsendTest(CAN_HandleTypeDef *p_hcan, uint16_t addr);
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
CAN_FilterTypeDef sFilterConfig;
CAN_TxHeaderTypeDef TxHeader;
CAN_RxHeaderTypeDef RxHeader;
uint8_t TxData[8];
uint8_t RxData[8];
uint32_t TxMailbox;
int rcvOk, rcvOk1;
const uint8_t CANaddr = 1, CANgroup = 2;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
	HAL_Delay(50);
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_CAN1_Init();
  /* USER CODE BEGIN 2 */

	CAN_InitFilter(&hcan1, CANaddr, CANgroup); 	
	
	//проверка посылка 0х400 на правильный адрес 
	//проверка посылка 0х400 на не правильный адрес 
	//проверка посылка 0х700 на правильную группу
	//проверка посылка 0х700 на не правильную группу
	//проверка посылка 0х700 широковещательная	
  //0x100, 0х300, 0х500, 0х600, 0х700 - команды для группы устройств, 0x400 - команда для одного устройства по адресу
	uint16_t addrArray[] = {0x480 + CANaddr, 0x481 + CANaddr, 0x700 + CANgroup, 0x701 + CANgroup, 0x77F, 	
					0x100 + CANgroup, 0x200 + CANgroup, 0x300 + CANgroup, 0x400 + CANaddr, 0x500 + CANgroup, 0x600 + CANgroup, 0x700 + CANgroup};	
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		rcvOk = 0;
		
		for (int i = 0; i < 7; i++) {
			rcvOk1 = 0;
			CANsendTest(&hcan1, addrArray[i]); 
			debugPinPulsePIN3(70);	
			if (rcvOk1) debugPinPulsePIN4(50);	
			HAL_Delay(350);
		} 
		
		for (int i = 0; i < rcvOk; i++) {debugPinPulsePIN5(250);HAL_Delay(250); }
		
		HAL_Delay(1500);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */


//на прерывание от любого CAN-приёмника обрабатываем принятую информацию
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *p_hcan) {
	HAL_CAN_GetRxMessage(p_hcan, CAN_RX_FIFO0, &RxHeader, RxData);
	
	int res = 1;
	for (int i = 0; i<8; i++)	
		if (RxData[i] != i) res = 0;
	
	//отделяем проверку по группе от  адреса
	if (res) {
		int msgId = RxHeader.StdId;
		switch (msgId & 0x0F00) {
			case 0x200: rcvOk++; rcvOk1=1; break; //от прибора управления
		
			case 0x400: rcvOk++; rcvOk1=1; break; //от программатора
			case 0x500: rcvOk++; rcvOk1=1; break;
			case 0x600: rcvOk++; rcvOk1=1; break;
			case 0x700: rcvOk++; rcvOk1=1; break;	
		}	
	}
}


HAL_StatusTypeDef CANsendTest(CAN_HandleTypeDef *p_hcan, uint16_t addr) {
	TxHeader.StdId = addr;
	TxHeader.RTR = CAN_RTR_DATA;
	TxHeader.IDE = CAN_ID_STD;
	TxHeader.DLC = 8;
	TxHeader.TransmitGlobalTime = DISABLE;
	for (int i=0; i<8; i++) TxData[i] = i;
	
	return HAL_CAN_AddTxMessage(p_hcan, &TxHeader, TxData, &TxMailbox);
}

void debugPinPulse(int pinLedNo, int delayTime){
	GPIO_TypeDef *port = GPIOD;
	uint16_t pin;
	switch (pinLedNo) {
		case 4: pin = DBG_LED4_Pin; break;
		case 5: pin = DBG_LED5_Pin; break;
		case 6: pin = DBG_LED6_Pin; break;
		default: pin = DBG_LED3_Pin;			
	}
	
	HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
	HAL_Delay(delayTime);
	HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
	while (1) {
		debugPinPulsePIN3(50);
		debugPinPulsePIN4(50);
		debugPinPulsePIN5(50);
		debugPinPulsePIN6(50);
	}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
