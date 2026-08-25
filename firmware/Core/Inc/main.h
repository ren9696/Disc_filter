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
extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;
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
#define KEY1_S2_EXTI_IRQn EXTI0_IRQn
#define KEY2_S3_Pin GPIO_PIN_1
#define KEY2_S3_GPIO_Port GPIOC
#define KEY2_S3_EXTI_IRQn EXTI1_IRQn
#define KEY3_S4_Pin GPIO_PIN_2
#define KEY3_S4_GPIO_Port GPIOC
#define KEY3_S4_EXTI_IRQn EXTI2_IRQn
#define KEY4_S5_Pin GPIO_PIN_3
#define KEY4_S5_GPIO_Port GPIOC
#define KEY4_S5_EXTI_IRQn EXTI3_IRQn
#define M6_FI_Pin GPIO_PIN_4
#define M6_FI_GPIO_Port GPIOA
#define M6_BI_Pin GPIO_PIN_5
#define M6_BI_GPIO_Port GPIOA
#define PRESSURE_ADC_Pin GPIO_PIN_6
#define PRESSURE_ADC_GPIO_Port GPIOA
#define BEEP_Pin GPIO_PIN_5
#define BEEP_GPIO_Port GPIOC
#define DC_ADC_Pin GPIO_PIN_0
#define DC_ADC_GPIO_Port GPIOB
#define M1_FI_Pin GPIO_PIN_12
#define M1_FI_GPIO_Port GPIOB
#define M1_BI_Pin GPIO_PIN_13
#define M1_BI_GPIO_Port GPIOB
#define M2_FI_Pin GPIO_PIN_14
#define M2_FI_GPIO_Port GPIOB
#define M2_BI_Pin GPIO_PIN_15
#define M2_BI_GPIO_Port GPIOB
#define M3_FI_Pin GPIO_PIN_6
#define M3_FI_GPIO_Port GPIOC
#define M3_BI_Pin GPIO_PIN_7
#define M3_BI_GPIO_Port GPIOC
#define M4_FI_Pin GPIO_PIN_8
#define M4_FI_GPIO_Port GPIOC
#define M4_BI_Pin GPIO_PIN_9
#define M4_BI_GPIO_Port GPIOC
#define M5_FI_Pin GPIO_PIN_11
#define M5_FI_GPIO_Port GPIOA
#define M5_BI_Pin GPIO_PIN_12
#define M5_BI_GPIO_Port GPIOA
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
#define M_PWR_EN_Pin GPIO_PIN_8
#define M_PWR_EN_GPIO_Port GPIOB
#define KEY0_S1_Pin GPIO_PIN_9
#define KEY0_S1_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
