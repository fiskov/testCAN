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
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usbd_cdc_if.h"
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
int ledSendCounter, ledRcvCounter, ledUSBCounter;
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
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */
  //скорость передачи по CAN-шине. Либо 250 кбит/с, либо 115 кбит/с
  #define CAN_SPEED 250
  if (CAN_SPEED == 115) MX_CAN1_Init_115(); else	MX_CAN1_Init();
    
	CAN_InitFilter(&hcan1, 0, 0); 	
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    debugPinCounter(3, &ledSendCounter);
    debugPinCounter(4, &ledRcvCounter);
    debugPinCounter(5, &ledUSBCounter);
    
		HAL_Delay(1);
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */


static uint8_t tx_USBbfr[11] = {0};
//на прерывание от первого CAN-приёмника отправляем по USB
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *p_hcan) {
	HAL_CAN_GetRxMessage(p_hcan, CAN_RX_FIFO0, &RxHeader, RxData);
  
  
  tx_USBbfr[0] = (RxHeader.StdId >> 8) & 0xFF;
  tx_USBbfr[1] = RxHeader.StdId & 0xFF;
  tx_USBbfr[2] = 8;
  
  for (int i = 0; i<8; i++)	tx_USBbfr[i+3] = RxData[i];
  
  ledRcvCounter=50;
  //отправка по USB
	CDC_Transmit_FS(tx_USBbfr, 11);
}


HAL_StatusTypeDef CANsendTest(CAN_HandleTypeDef *p_hcan, uint16_t addr, uint8_t * bfr, uint8_t size) {
	TxHeader.StdId = addr;
	TxHeader.RTR = CAN_RTR_DATA;
	TxHeader.IDE = CAN_ID_STD;
	TxHeader.DLC = size;
	TxHeader.TransmitGlobalTime = DISABLE;
	//for (int i=0; i<size; i++) TxData[i] = bfr[i];
  
	ledSendCounter=50;
	return HAL_CAN_AddTxMessage(p_hcan, &TxHeader, bfr, &TxMailbox);
}


//на прием от USB посылаем в CAN
static uint8_t bfrSend[8] = {0};
void CDC_RecieveCallBack(uint8_t *bfrUSB, uint32_t len)
{
  uint16_t addr = bfrUSB[0] * 256 + bfrUSB[1];

  int size = bfrUSB[2];
  if (size > 8) size = 8;
  memset(bfrSend, 0, 8);
  for (int i=0; i<size; i++) bfrSend[i] = bfrUSB[i+3];
  
  ledUSBCounter=50;
  
  CANsendTest(&hcan1, addr, bfrSend, size);
}

void debugPinPulse(int pinLedNo, int delayTime)
{
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

void debugPinCounter(int pinLedNo, int *counter)
{
	GPIO_TypeDef *port = GPIOD;
	uint16_t pin;
  int cnt = *counter;
	switch (pinLedNo) {
		case 4: pin = DBG_LED4_Pin; break;
		case 5: pin = DBG_LED5_Pin; break;
		case 6: pin = DBG_LED6_Pin; break;
		default: pin = DBG_LED3_Pin;			
	}
	
  if (cnt > 0) {
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
    cnt--;
    if (cnt==0) HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
  }
	*counter = cnt;	
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
