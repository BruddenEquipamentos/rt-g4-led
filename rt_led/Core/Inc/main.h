/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "stm32g0xx_hal.h"

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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);
void MX_I2C2_Init(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define INC_MINUS_Pin GPIO_PIN_0
#define INC_MINUS_GPIO_Port GPIOA
#define SP_MINUS_Pin GPIO_PIN_1
#define SP_MINUS_GPIO_Port GPIOA
#define SP_PLUS_Pin GPIO_PIN_2
#define SP_PLUS_GPIO_Port GPIOA
#define INC_PLUS_Pin GPIO_PIN_3
#define INC_PLUS_GPIO_Port GPIOA
#define BK_RODA_Pin GPIO_PIN_4
#define BK_RODA_GPIO_Port GPIOA
#define HRM_ADC_Pin GPIO_PIN_5
#define HRM_ADC_GPIO_Port GPIOA
#define HRM_DIG_Pin GPIO_PIN_7
#define HRM_DIG_GPIO_Port GPIOA
#define CMD_Pin GPIO_PIN_0
#define CMD_GPIO_Port GPIOB
#define I_FREQ_Pin GPIO_PIN_1
#define I_FREQ_GPIO_Port GPIOB
#define W_HW_Pin GPIO_PIN_2
#define W_HW_GPIO_Port GPIOB
#define LO_PLUS_Pin GPIO_PIN_12
#define LO_PLUS_GPIO_Port GPIOB
#define LO_MINUS_Pin GPIO_PIN_13
#define LO_MINUS_GPIO_Port GPIOB
#define TS03_Pin GPIO_PIN_14
#define TS03_GPIO_Port GPIOB
#define TS04_Pin GPIO_PIN_15
#define TS04_GPIO_Port GPIOB
#define TS05_Pin GPIO_PIN_8
#define TS05_GPIO_Port GPIOA
#define TS09_Pin GPIO_PIN_9
#define TS09_GPIO_Port GPIOA
#define TS10_Pin GPIO_PIN_6
#define TS10_GPIO_Port GPIOC
#define TS11_Pin GPIO_PIN_7
#define TS11_GPIO_Port GPIOC
#define GPO_Pin GPIO_PIN_10
#define GPO_GPIO_Port GPIOA
#define TS00_Pin GPIO_PIN_15
#define TS00_GPIO_Port GPIOA
#define TS01_Pin GPIO_PIN_0
#define TS01_GPIO_Port GPIOD
#define TS02_Pin GPIO_PIN_1
#define TS02_GPIO_Port GPIOD
#define TS06_Pin GPIO_PIN_2
#define TS06_GPIO_Port GPIOD
#define TS07_Pin GPIO_PIN_3
#define TS07_GPIO_Port GPIOD
#define TS08_Pin GPIO_PIN_3
#define TS08_GPIO_Port GPIOB
#define CONTR_Pin GPIO_PIN_4
#define CONTR_GPIO_Port GPIOB
#define EMG_Pin GPIO_PIN_5
#define EMG_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
