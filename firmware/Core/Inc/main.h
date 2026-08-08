/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

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
#define KEY1_S2_Pin GPIO_PIN_0
#define KEY1_S2_GPIO_Port GPIOC
#define KEY2_S3_Pin GPIO_PIN_1
#define KEY2_S3_GPIO_Port GPIOC
#define KEY3_S4_Pin GPIO_PIN_2
#define KEY3_S4_GPIO_Port GPIOC
#define KEY4_S5_Pin GPIO_PIN_3
#define KEY4_S5_GPIO_Port GPIOC
#define BEEP_Pin GPIO_PIN_5
#define BEEP_GPIO_Port GPIOC
#define LED_ZT_Pin GPIO_PIN_10
#define LED_ZT_GPIO_Port GPIOC
#define SCREEN_PWR_Pin GPIO_PIN_11
#define SCREEN_PWR_GPIO_Port GPIOC
#define SCREEN_CS_Pin GPIO_PIN_4
#define SCREEN_CS_GPIO_Port GPIOB
#define SCREEN_RD_Pin GPIO_PIN_5
#define SCREEN_RD_GPIO_Port GPIOB
#define SCREEN_WR_Pin GPIO_PIN_6
#define SCREEN_WR_GPIO_Port GPIOB
#define SCREEN_DATA_Pin GPIO_PIN_7
#define SCREEN_DATA_GPIO_Port GPIOB
#define KEY0_S1_Pin GPIO_PIN_9
#define KEY0_S1_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
