/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define DBG_LED4_Pin GPIO_PIN_12
#define DBG_LED4_GPIO_Port GPIOD
#define DBG_LED3_Pin GPIO_PIN_13
#define DBG_LED3_GPIO_Port GPIOD
#define DBG_LED5_Pin GPIO_PIN_14
#define DBG_LED5_GPIO_Port GPIOD
#define DBG_LED6_Pin GPIO_PIN_15
#define DBG_LED6_GPIO_Port GPIOD
/* USER CODE BEGIN Private defines */
void debugPinPulse(int pinLedNo, int delayTime);
#define debugPinPulsePIN3(x) do{ debugPinPulse(3,x); } while(0)
#define debugPinPulsePIN4(x) do{ debugPinPulse(4,x); } while(0)
#define debugPinPulsePIN5(x) do{ debugPinPulse(5,x); } while(0)
#define debugPinPulsePIN6(x) do{ debugPinPulse(6,x); } while(0)
#define debugPinSetPIN6(x) do{ HAL_GPIO_WritePin(DBG_LED6_GPIO_Port, DBG_LED6_Pin, GPIO_PIN_SET);  } while(0)
#define debugPinResetPIN6(x) do{ HAL_GPIO_WritePin(DBG_LED6_GPIO_Port, DBG_LED6_Pin, GPIO_PIN_RESET);  } while(0)

#define debugPinTogglePIN3(x) do{ HAL_GPIO_TogglePin(DBG_LED3_GPIO_Port, DBG_LED3_Pin);  } while(0)
#define debugPinTogglePIN4(x) do{ HAL_GPIO_TogglePin(DBG_LED4_GPIO_Port, DBG_LED4_Pin);  } while(0)
#define debugPinTogglePIN5(x) do{ HAL_GPIO_TogglePin(DBG_LED5_GPIO_Port, DBG_LED5_Pin);  } while(0)
#define debugPinTogglePIN6(x) do{ HAL_GPIO_TogglePin(DBG_LED6_GPIO_Port, DBG_LED6_Pin);  } while(0)

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
